#include "EPD_1IN54_IL0373.h"

/* 控制刷新电压波形用到的LUT */
//注意和默认模式不同，这里黑白反色了，这样就能跳过paint clear white。

//LUT需要分开
const unsigned char lut_20_vcomDC[] =
{
  //6 bytes per waveform. byte 1: levels(00=G 01=VDH 10=VDL 11=Floating)
  //level select //f0 //f1//f2//f3 //repeat
  0x00, 0x14, 0x00, 0x00, 0x00, 0x02,//接地
  0x60, 0x01, 0x01, 0x00, 0x00, 0x02,//VCOM还会翻转 VDH VDL变换，0x60=0b01100000
  0x00, 0x0C, 0x00, 0x00, 0x00, 0x01,//接地
  0x00, 0x1A, 0x00, 0x00, 0x00, 0x02,//接地
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, //allgateon //vcomhighvoltage 这两个不要动
};

const unsigned char lut_21_ww[] =
{
  0x40, 0x14, 0x00, 0x00, 0x00, 0x02,//VDH持续一段时间
  0x90, 0x01, 0x01, 0x00, 0x00, 0x02, //VDL VDH变换
  0x40, 0x0C, 0x00, 0x00, 0x00, 0x01,//VDH持续
  0x80, 0x1A, 0x01, 0x00, 0x00, 0x02, //VDL持续
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_22_bw[] =
{
  0x40, 0x14, 0x00, 0x00, 0x00, 0x02, //和ww相同，可以理解
  0x90, 0x01, 0x01, 0x00, 0x00, 0x02,
  0x40, 0x0C, 0x00, 0x00, 0x00, 0x01,
  0x80, 0x1A, 0x01, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_23_wb[] =
{
  0x80, 0x14, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x01, 0x01, 0x00, 0x00, 0x02, //就是反过来了
  0x80, 0x0C, 0x00, 0x00, 0x00, 0x01,
  0x40, 0x1A, 0x01, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_24_bb[] =
{
  0x80, 0x14, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x01, 0x01, 0x00, 0x00, 0x02,
  0x80, 0x0C, 0x00, 0x00, 0x00, 0x01,
  0x40, 0x1A, 0x01, 0x00, 0x00, 0x02,
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
	devDelay(3);
	RES_HIGH;
	//devDelay(10);
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
	WAIT_BUSY;

	EPD_Cmd( 0x06 );
	EPD_Dat( 0x17 ); //A
	EPD_Dat( 0x17 ); //B
	EPD_Dat( 0x17 ); //C

	EPD_Cmd( 0x00 ); //panel setting
	EPD_Dat( 0x3F ); //1F用OTP，3F用内存LUT

	EPD_Cmd( 0x01 );
	EPD_Dat( 0x03 );
	EPD_Dat( 0x00 );
	EPD_Dat( 0x2B );
	EPD_Dat( 0x2B );
	EPD_Dat( 0x09 );

	EPD_Cmd(0x20);
	EPD_DATAPGM(lut_20_vcomDC, sizeof(lut_20_vcomDC));
	EPD_Cmd(0x21);
	EPD_DATAPGM(lut_21_ww, sizeof(lut_21_ww));
	EPD_Cmd(0x22);
	EPD_DATAPGM(lut_22_bw, sizeof(lut_22_bw));
	EPD_Cmd(0x23);
	EPD_DATAPGM(lut_23_wb, sizeof(lut_23_wb));
	EPD_Cmd(0x24);
	EPD_DATAPGM(lut_24_bb, sizeof(lut_24_bb));

	EPD_Cmd( 0x04 );//start boot
	WAIT_BUSY;

	EPD_Cmd( 0x61 ); //resolution
	EPD_Dat( EPD_WIDTH & 0xFF );
	EPD_Dat( EPD_HEIGHT >> 8 );
	EPD_Dat( EPD_HEIGHT & 0xFF );

	EPD_Cmd(0X50);			//VCOM AND DATA INTERVAL SETTING 边框颜色			
	EPD_Dat(0x47);		//反色所以用47而不是57，07的话是浮空。

}

//告诉墨水屏把RAM里的数据刷新到屏幕上
void EPD_Update(void)
{
    EPD_Cmd( 0x12 );
    
}


//告诉墨水屏把RAM里的数据刷新到屏幕上,配合局刷初始化使用
void EPD_PartialUpdate(void)
{
    EPD_Cmd( 0x12 );
    	
    
}


//刷白屏。
void EPD_Clear(void)
{

	EPD_Cmd(0x10);
	for(uint32_t i = 0; i < 2888; i++)
	{
		EPD_Dat(0x00);
	}

	EPD_Cmd(0x13);
	for(uint32_t i = 0; i < 2888; i++)
	{
		EPD_Dat(0x00);
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
	//启动
	EPD_Cmd( 0x04 );
	WAIT_BUSY;

    //上LUT
	//进入partial模式
	//设置partial window
    
//传送显示数据
	EPD_Cmd(0x13);
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

	//出partial模式
}

//让屏幕睡眠
void EPD_Sleep(void)
{
    EPD_Cmd(0x50);
    EPD_Dat(0x17);

    EPD_Cmd(0x82);
    EPD_Dat(0x00);

    EPD_Cmd(0x02);
   // WAIT_BUSY;
   // EPD_Cmd(0x07);
   // EPD_Dat(0xA5);
}

void EPD_DeepSleep(void)
{
	EPD_Cmd(0x07);
	EPD_Dat(0xA5);
}
