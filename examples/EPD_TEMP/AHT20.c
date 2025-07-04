#include "AHT20.h"

//初始化输入
void SoftI2CInit()
{
	R32_PA_PU |= SDA_PIN;//SDA开启上拉电阻，变为上拉输入模式
	R32_PA_PU &= ~(VCC_PIN);// disable VCC pullup
	R32_PA_PD_DRV |= VCC_PIN;
	SCL_HIGH;
	R32_PA_SET |= VCC_PIN;//VCC输出
	R32_PA_DIR |= SCL_PIN|VCC_PIN;//SCL和VCC输出




//	I2CStart();
//    I2C_Write( (0x38<<1)|0 );//地址加写命令
//     //设置命令
//    I2CStop();
    //Delay_Ms(300); //校准需要300ms

}

void I2CStart()
{
	SCL_HIGH;
	SDA_HIGH;
	I2CDelayUs(1);
	SDA_LOW;
	SCL_LOW;
	
}

void I2CStop()
{
	SDA_LOW;
	I2CDelayUs(1);
	SCL_HIGH;
	I2CDelayUs(1);
	SDA_HIGH;
	
}


uint8_t I2C_Write(uint8_t dat)
{
	uint8_t data = dat;
	for (uint8_t i = 0; i<8;i++)
	{
		
		if(data&0x80){SDA_HIGH;}
		else{SDA_LOW;}
		SCL_HIGH;
		I2CDelayUs(1);
		SCL_LOW;
		data = (data << 1);
	}
	SDA_HIGH;//输入读取
	I2CDelayUs(1);
	SCL_HIGH;
	I2CDelayUs(1);
	if(READ_SDA()==0)
	{
		SCL_LOW;
		return ACK;
	} 
	else 
	{	
		SCL_LOW;
		return NACK;
	}
	
}

uint8_t I2C_Read(uint8_t ack)
{
	uint8_t data = 0;
	SDA_HIGH;
	for (uint8_t i = 0; i < 8 ; i++ )
	{
		data = (data << 1);
		SCL_HIGH;
		if(READ_SDA() == 0){;}
		else{data |= 0x01;}
		SCL_LOW;
		I2CDelayUs(1);
		
	}

	if(ack)
	{
		SDA_LOW;//ACK
	} 
	else 
	{
		SDA_HIGH;//NACK
	}
	SCL_HIGH;
	I2CDelayUs(1);
	SCL_LOW;
	SDA_HIGH;
	return data;

}


void AHT20_beginMeasure(void)
{
	
	I2CStart();
    I2C_Write( (0x38<<1)|0 );//地址加写命令
    I2C_Write( 0xAC ); //触发测量
    I2C_Write( 0x33 );
    I2C_Write( 0x00 );
    I2CStop();
    return;
}

void AHT20_getDat(uint8_t *data)
{
	I2CStart();
    I2C_Write(0x38<<1|1);//读取命令
	data[0] = I2C_Read(1);
	data[1] = I2C_Read(1);
	data[2] = I2C_Read(1);
	data[3] = I2C_Read(1);
	data[4] = I2C_Read(1);
	data[5] = I2C_Read(0);
    I2CStop();
}
