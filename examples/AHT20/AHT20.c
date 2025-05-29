#include "AHT20.h"

//初始化输入
void SoftI2CInit()
{
	R32_PA_PU |= SDA_PIN;//SDA开启上拉电阻，变为上拉输入模式
	R32_PA_DIR |= SCL_PIN;//SCL输出
	SCL_HIGH;	

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
	for (uint8_t i = 0; i<8;i++)
	{

		SCL_HIGH;
		if(READ_SDA() == 0){;}
		else{data |= 0x01;}
		data = (data << 1);
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
	I2CDelayUs(1);
	return data;

}


