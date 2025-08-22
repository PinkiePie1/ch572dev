/****************************************************************
 * File Name          : Main.c
 * Author             : JCQ
 * Version            : V1.0
 * Date               : 2025/04/07
 * Description        : 裸机驱动墨水屏
 *********************************************************************************/
#include "CH57x_common.h"
#include "main.h"
#include "miniGUI.h"
#include "imageData.h"
#include "MAX30102.h"
#include "EPD_1IN54_SSD1680.h"
#include <CH572rf.h>
//#include <stdlib.h>

#define POWER_PIN 0 //如果用5V，这里改成1

uint32_t humid;
uint32_t temperature;
__attribute__((__aligned__(4))) uint8_t imageCache[2888]={0};
uint8_t refreshCount = 250;
uint8_t img_index = 0;
uint8_t rawData[9];
uint8_t MacAddr[6]={0x1A,0x2A,0x3A,0x4A,0x5A,0x6A};
rfRoleConfig_t conf ={0};

void MySleep(uint8_t use5v);

rfipTx_t gTxParam;
uint8_t tx_flag = 0;
__attribute__((__aligned__(4))) uint8_t TxBuf[64];

const uint8_t advert_data[]={
	0x02,
	0x00,//长度,后面再填入
	0x1A,0x2A,0x3A,0x4A,0x5A,0x6A,//MAC地址，反过来的
	0x05,0x09,'t','e','m','p', //完整名字
	0x07,0xFF,0xFF,0xFF,0x00,0x00,0x01,0x02 //18-21	
};

__HIGH_CODE
void MySleep(uint8_t use5v)
{
    uint16_t ldoconfig = 0;
	if(use5v)
    {
        ldoconfig |= RB_PWR_LDO5V_EN;
    }
    sys_safe_access_enable();
	R8_CLK_SYS_CFG = CLK_SOURCE_HSE_PLL_60MHz;
    sys_safe_access_disable();

	PFIC->SCTLR |= (1 << 2); //deep sleep
    ldoconfig |= RB_PWR_PLAN_EN | RB_PWR_CORE | RB_PWR_RAM12K | RB_PWR_EXTEND | RB_XT_PRE_EN |(1<<12) ;
    sys_safe_access_enable();
 	R8_SLP_POWER_CTRL |= 0x40; //longest wake up delay
  	R16_POWER_PLAN = ldoconfig;
    sys_safe_access_disable();
	
	asm volatile ("wfi\nnop\nnop" );
	uint16_t i = 400;
    do {
    __nop();
    }while (i--);
    sys_safe_access_enable();
    R16_POWER_PLAN &= ~RB_PWR_PLAN_EN;
    R16_POWER_PLAN &= ~RB_XT_PRE_EN;
    R8_CLK_SYS_CFG = CLK_SOURCE_HSE_PLL_100MHz;
    sys_safe_access_disable();

}

__HIGH_CODE
void RF_ProcessCallBack( rfRole_States_t sta,uint8_t id  )
{

	if( sta&RF_STATE_TX_FINISH )
    {
    	tx_flag = 0;
    }
    if( sta&RF_STATE_TIMEOUT )
    {
    	  	
    }
}

void main(void)
{
    sys_safe_access_enable();
    R8_CLK_SYS_CFG = CLK_SOURCE_HSE_PLL_100MHz;
    sys_safe_access_disable();
    
	//GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);

	EPD_Hal_Init();
    //GPIOA_ITModeCfg(EPD_BUSY_PIN, GPIO_ITMode_FallEdge); 
    //PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Fsys_Delay_4096);
	EPD_Init();
	paint_SetImageCache(imageCache);	
	EPD_Printf(0, 0, font16, BLACK, "hi.");
	EPD_SendDisplay(imageCache);
	DelayMs(2000);
	EPD_Sleep();

	SoftI2CInit();
	I2CStart();
	uint8_t ack = I2C_Write(0xAE);
	I2C_Write(0xFF);
	I2CStart();
	I2C_Write(0xAE| 0x01);
	uint8_t deviceid = I2C_Read(0);
	I2CStop();

	EPD_Printf(0, 16,font16, BLACK,"err:%d, deviceid:%02X", ack, deviceid);
	if(deviceid == 0x15){
		EPD_Printf(0, 32,font16, BLACK,"MAX30102 OK");
	} else {
		EPD_Printf(0, 32,font16, BLACK,"MAX30102 ERR");
	}
	EPD_PartialDisplay(imageCache);
	DelayMs(2000);
	EPD_Sleep();

	while(1);

}

__INTERRUPT
__HIGH_CODE
void GPIOA_IRQHandler(void)
{
    GPIOA_ClearITFlagBit(0xFFFF);
    
}

__INTERRUPT
__HIGH_CODE
void RTC_IRQHandler(void)
{
	R8_RTC_FLAG_CTRL =  RB_RTC_TRIG_CLR;
	
}

//这个不能漏掉
__INTERRUPT
__HIGH_CODE
void LLE_IRQHandler( void )
{
    LLE_LibIRQHandler( );
}

__INTERRUPT
__HIGH_CODE
void BB_IRQHandler( void )
{
    BB_LibIRQHandler( );
}

