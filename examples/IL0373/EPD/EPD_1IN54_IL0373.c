#include "EPD_1IN54_IL0373.h"

/* 控制刷新电压波形用到的LUT */
//注意和默认模式不同，这里黑白反色了，这样就能跳过paint clear white。

//LUT需要分开
const unsigned char lut_20_vcomDC[] =
{
  //6 bytes per waveform. byte 1: levels(00=G 01=VDH 10=VDL 11=Floating)
  //level select //f0 //f1//f2//f3 //repeat
  0x00, 0x08, 0x00, 0x00, 0x00, 0x02,//接地
  0x60, 0x08, 0x08, 0x00, 0x00, 0x04,//VCOM还会翻转 VDH VDL变换，0x60=0b01100000
  0x00, 0x0A, 0x00, 0x00, 0x00, 0x01,//接地
  0x00, 0x12, 0x12, 0x00, 0x00, 0x01,//接地
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, //allgateon //vcomhighvoltage 这两个不要动
};

const unsigned char lut_21_ww[] =
{
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,//VDH持续一段时间
  0x90, 0x08, 0x08, 0x00, 0x00, 0x04, //VDL VDH变换
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,//VDH持续
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01, //VDL持续
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_22_bw[] =
{
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02, //和ww相同，可以理解
  0x90, 0x08, 0x08, 0x00, 0x00, 0x04,
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_23_wb[] =
{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x08, 0x08, 0x00, 0x00, 0x04, //就是反过来了
  0x80, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_24_bb[] =
{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x08, 0x08, 0x00, 0x00, 0x04,
  0x80, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/******
 *初始化EPD用的GPIO引脚和SPI接口。
 *
 *
 */
void EPD_Hal_Init(void)
{    
    //初始化输入和输出。BUSY空闲时为高，所以用IPU。
    //浮空应该也是可以的。
	GPIOA_ModeCfg( EPD_SCK_PIN |
	               EPD_SDA_PIN |
                   EPD_RES_PIN |
                   EPD_DC_PIN |
                   EPD_CS_PIN, GPIO_ModeOut_PP_5mA );
   GPIOA_ModeCfg(EPD_BUSY_PIN, GPIO_ModeIN_Floating);

   SPI_MasterDefInit();//默认的SPI初始化，三线全双工。后续需要改
   SPI_CLKCfg(5); //2分频
   CS_HIGH;
   RES_HIGH;
   DC_HIGH;
}

//写EPD寄存器地址
static void EPD_Cmd(uint8_t cmd)
{
	DC_LOW;
	CS_LOW;
	SPI_MasterSendByte(cmd);
	CS_HIGH;
}

//写EPD寄存器数据。EPD的操作就是先写地址，再写数据，可以写多次。
static void EPD_Dat(uint8_t dat)
{
	DC_HIGH;
	CS_LOW;
	SPI_MasterSendByte(dat);
	CS_HIGH;
}

static void EPD_DATAPGM(const unsigned char* data, size_t len)
{

	for(uint16_t i=0; i<len; i++)
	{
		EPD_Dat( data[i] );
	}

}

//RES引脚发送复位命令
static void EPD_HardReset(void)
{
	//RES_HIGH; //既然已经拉高了，那就没有必要再拉低一次。
	//devDelay(10);
	RES_LOW;
	devDelay(100);
	RES_HIGH;
	//devDelay(10);
}

//写入波形控制的LUT，通过改变LUT数组可以控制波形
//不应该直接调用，它是给初始化函数用的。
static void EPD_LUT(const uint8_t *lutPtr)
{
	EPD_Cmd(0x32);
	
	for(uint16_t i=0; i<153; i++)
	{
		EPD_Dat( lutPtr[i] );
	}
	
	WAIT_BUSY;
	EPD_Cmd( 0x3F );
	EPD_Dat( lutPtr[153] );
	EPD_Cmd( 0x03 );
	EPD_Dat( lutPtr[154] );
	EPD_Cmd( 0x04 );
	EPD_Dat( lutPtr[155] );
	EPD_Dat( lutPtr[156] );
	EPD_Dat( lutPtr[157] );
	EPD_Cmd( 0x2c );
	EPD_Dat( lutPtr[158] );
	
}

//设置显示窗口
static void EPD_SetWindows( uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend )
{
    EPD_Cmd( 0x44 );
    EPD_Dat( (Xstart>>3) & 0xFF );
    EPD_Dat( (Xend>>3) & 0xFF );

    EPD_Cmd( 0x45 );
    EPD_Dat( Ystart & 0xFF );
    EPD_Dat( (Ystart>>8) & 0xFF );
    EPD_Dat( Yend & 0xFF );
    EPD_Dat( (Yend>>8) & 0xFF );
    
}

//设置光标位置
static void EPD_SetCursor( uint16_t X, uint16_t Y )
{
    EPD_Cmd( 0x4E );
    EPD_Dat( X>>3 & 0xFF );

    EPD_Cmd( 0x4F );
    EPD_Dat( Y & 0xFF );
    EPD_Dat( (Y>>8) & 0xFF );

}

void EPD_Init(void)
{
	//硬重置
	EPD_HardReset();
	devDelay( 100 );
	WAIT_BUSY;

	EPD_Cmd( 0x06 );
	EPD_Dat( 0x17 ); //A
	EPD_Dat( 0x17 ); //B
	EPD_Dat( 0x17 ); //C

	EPD_Cmd( 0x00 ); //panel setting
	EPD_Dat( 0x3F ); //1F用OTP，3F用内存LUT

	EPD_Cmd(0x20);
	EPD_DATAPGM(lut_20_vcomDC, sizeof(lut_20_vcomDC));
	EPD_Cmd(0x23);
	EPD_DATAPGM(lut_21_ww, sizeof(lut_21_ww));
	EPD_Cmd(0x24);
	EPD_DATAPGM(lut_22_bw, sizeof(lut_22_bw));
	EPD_Cmd(0x21);
	EPD_DATAPGM(lut_23_wb, sizeof(lut_23_wb));
	EPD_Cmd(0x22);
	EPD_DATAPGM(lut_24_bb, sizeof(lut_24_bb));

	EPD_Cmd( 0x04 );//start boot
	WAIT_BUSY;

	EPD_Cmd( 0x61 ); //resolution
	EPD_Dat( EPD_WIDTH & 0xFF );
	EPD_Dat( EPD_HEIGHT >> 8 );
	EPD_Dat( EPD_HEIGHT & 0xFF );

	EPD_Cmd(0X50);			//VCOM AND DATA INTERVAL SETTING 边框颜色			
	EPD_Dat(0x57);		//WBmode:VBDF 17|D7 VBDW 97 VBDB 57		WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

}

//告诉墨水屏把RAM里的数据刷新到屏幕上
void EPD_Update(void)
{
    EPD_Cmd( 0x12 );
    devDelay( 100 );
    WAIT_BUSY;	
    
}


//告诉墨水屏把RAM里的数据刷新到屏幕上,配合局刷初始化使用
void EPD_PartialUpdate(void)
{
    EPD_Cmd( 0x12 );
    WAIT_BUSY;	
    
}


//刷白屏。
void EPD_Clear(void)
{

	EPD_Cmd(0x10);
	for(uint32_t i = 0; i < 2888; i++)
	{
		EPD_Dat(0xFF);
	}

	EPD_Cmd(0x13);
	for(uint32_t i = 0; i < 2888; i++)
	{
		EPD_Dat((uint8_t)i);
	}


	EPD_Update();
	
}


void EPD_SendDisplay(uint8_t *image)
{
#if(defined(REVERSED)) && (REVERSED == 1)

	EPD_Cmd(0x13);
	
    R8_SPI_CTRL_CFG |= RB_SPI_BIT_ORDER;
	for (int i=2887;i>=0;i--){
	EPD_Dat(image[i]);
	}
	R8_SPI_CTRL_CFG &= ~(RB_SPI_BIT_ORDER);
	
	EPD_Cmd(0x10);
	
	R8_SPI_CTRL_CFG |= RB_SPI_BIT_ORDER;
	for (int i=2887;i>=0;i--){
	EPD_Dat(image[i]);
	}
	R8_SPI_CTRL_CFG &= ~(RB_SPI_BIT_ORDER);

	
#else
	EPD_Cmd(0x13);
	
    DC_HIGH;
	CS_LOW;
	SPI_MasterDMATrans(image,2888);
	CS_HIGH;

	EPD_Cmd(0x10);
	
    DC_HIGH;
	CS_LOW;
	SPI_MasterDMATrans(image,2888);
	CS_HIGH;

#endif
	EPD_Update();
}

void EPD_PartialDisplay(uint8_t *image)
{
	//硬重置
	EPD_HardReset();
	devDelay( 100 );

	//EPD_LUT(MyPartialLUT);
	//可以不用指定WS，因为用的是内存里的LUT
	/*
	EPD_Cmd( 0x37 );
	EPD_Dat( 0x00 );
	EPD_Dat( 0x00 );
	EPD_Dat( 0x00 );
	EPD_Dat( 0x00 );
	EPD_Dat( 0x00 );
	EPD_Dat( 0x40|1<<6 );
	EPD_Dat( 0x00 );
	EPD_Dat( 0x00 );
	EPD_Dat( 0x00 );
	EPD_Dat( 0x00 );
*/
    //边框颜色,这里边框接地
	EPD_Cmd( 0x3C );
	EPD_Dat( 0x80 );

	//可以不用先开启analog，等刷新指令的时候再开也不迟
/*
	EPD_Cmd( 0x22 );
	EPD_Dat( 0xC0 );
	EPD_Cmd( 0x20 );
	WAIT_BUSY;
*/	
	EPD_SetWindows(0, 0, EPD_WIDTH-1, EPD_HEIGHT-1);
	EPD_SetCursor(0, 0);

//传送显示数据，不需要传送0x26
	EPD_Cmd(0x24);
#if(defined(REVERSED)) && (REVERSED == 1)
    R8_SPI_CTRL_CFG |= RB_SPI_BIT_ORDER;
	for (int i=2887;i>=0;i--){
	EPD_Dat(image[i]);
	}
	R8_SPI_CTRL_CFG &= ~(RB_SPI_BIT_ORDER);
#else
    DC_HIGH;
	CS_LOW;
	SPI_MasterDMATrans(image,2888);
	CS_HIGH;
#endif
	EPD_PartialUpdate();
}

//让屏幕睡眠
void EPD_Sleep(void)
{
    EPD_Cmd(0x50);
    EPD_Dat(0xF7);

    EPD_Cmd(0x02);
    WAIT_BUSY;
    EPD_Cmd(0x07);
    EPD_Dat(0xA5);
}
