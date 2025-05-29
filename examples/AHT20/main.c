#include "CH57x_common.h"
#include "AHT20.h"

void main()
{
	GPIOA_ModeCfg(GPIO_Pin_9,GPIO_ModeOut_PP_5mA);
	GPIOA_ResetBits(GPIO_Pin_9);
	sys_safe_access_enable();
    R8_CLK_SYS_CFG = CLK_SOURCE_HSE_PLL_100MHz;
    sys_safe_access_disable();

	uint8_t ret = 0;
	uint8_t data[5] = {0};
		
    SoftI2CInit();
    DelayMs(100);
    //读取状态字
	I2CStart();
    I2C_Write( (0x38<<1)|1 );
    ret = I2C_Read(0);
    I2CStop();

    //发送测量命令
    DelayMs(10);
    I2CStart();
    I2C_Write( (0x38<<1)|0 );//地址加写命令
    I2C_Write( 0xAC ); //触发测量
    I2C_Write( 0x33 );
    I2C_Write( 0x00 );
    I2CStop();
    DelayMs(100);

	//读取测量数值
    I2CStart();
    I2C_Write(0x38<<1|1);//读取命令
	ret = I2C_Read(1);
	data[0] = I2C_Read(1);
	data[1] = I2C_Read(1);
	data[2] = I2C_Read(1);
	data[3] = I2C_Read(1);
	data[4] = I2C_Read(0);
    I2CStop();
  
    while(1);
	
}

