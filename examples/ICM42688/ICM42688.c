#include "ICM42688.h"
#include "CH57x_common.h"


static void ICM_WriteReg(uint8_t Reg, uint8_t size, uint8_t *buffer)
{
	CS_LOW;
	SPI_MasterSendByte( Reg|(0<<8) );//Write 为0
	SPI_MasterDMATrans(buffer,size);
	CS_HIGH;
}

static void ICM_ReadReg(uint8_t Reg, uint8_t size, uint8_t *buffer)
{
	CS_LOW;
	SPI_MasterSendByte( Reg|(1<<7) );//Read 为1
	SPI_MasterRecv(buffer,size);
	CS_HIGH;
}


//回0成功，其他失败。
uint8_t ICM_Init()
{
	//CS,SCK,MOSI
	uint8_t buffer[10] = {0};
    GPIOA_ModeCfg(GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7, GPIO_ModeOut_PP_5mA );
    //MISO
    GPIOA_ModeCfg(GPIO_Pin_6, GPIO_ModeIN_PU);
    SPI_MasterDefInit();//默认的SPI初始化，三线全双工。后续需要改
    SPI_CLKCfg(2); //2分频

    CS_HIGH;

    ICM_ReadReg(0x75,1,buffer);
    if(buffer[0]==0x47){
    	return 0;
    } else
    {
    	return 10;
    }
}

void ICM_Begin()
{
	uint8_t buffer[1] = {0};
	buffer[0] = 0x0F;
	//put gyro and accl to LN mode.
	ICM_WriteReg(0x4E,1,buffer);
}

void ICM_ReadAll(uint8_t *buffer)
{
	ICM_ReadReg(0x1D,14,buffer);
}
