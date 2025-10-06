#include "AS7341.h"

uint8_t err=0;
//初始化输入
void SoftI2CInit()
{
	R32_PA_PU |= SDA_PIN;//SDA开启上拉电阻，变为上拉输入
	SCL_HIGH;
	R32_PA_DIR |= SCL_PIN;//SCL输出

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

void AS7341_ReadReg(uint8_t reg, uint8_t *buffer, uint16_t len)
{
	I2CStart();
	I2C_Write( 0x39 << 1 ); // address write
	I2C_Write(reg); // register address write
	I2CStart(); //restart
	I2C_Write( (0x39 << 1) | 0x01 ); //address read
	while( len > 1 )
	{
		*buffer = I2C_Read(1);
		len--;
		buffer++;
	}
	*buffer = I2C_Read(0);
	I2CStop();
}

void AS7341_WriteReg(uint8_t reg, uint8_t *buffer, uint16_t len)
{
	I2CStart();
	I2C_Write( 0x39 << 1 ); // read sequence 
	I2C_Write(reg); // read address
	for(uint16_t i = 0; i < len; i++)
	{
		I2C_Write(buffer[i]);
	}
	I2CStop();
}
