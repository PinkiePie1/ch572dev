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
#include "SHT40.h"
#include "EPD_1IN54_SSD1680.h"
#include <stdlib.h>
#include <CH572rf.h>

#define POWER_PIN 0 //如果用5V，这里改成1

uint32_t humid;
uint32_t temperature;
uint8_t *imageCache;
uint8_t refreshCount = 250;
uint8_t img_index = 0;
uint8_t rawData[9];
uint8_t MacAddr[6]={0x1A,0x2A,0x3A,0x4A,0x5A,0x6A};
rfRoleConfig_t conf ={0};

//sleep.



void MySleep(uint8_t use5v);

rfipTx_t gTxParam;
uint8_t tx_flag = 0;
__attribute__((__aligned__(4))) uint8_t TxBuf[64];

const uint8_t advert_data[]={
	0x02,
	0x00,//长度,后面再填入
	0x1A,0x2A,0x3A,0x4A,0x5A,0x6A,//MAC地址，反过来的
	0x05,0x09,'T','E','M','P', //完整名字
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

	GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);

	EPD_Hal_Init();
	GPIOA_ITModeCfg(EPD_BUSY_PIN, GPIO_ITMode_FallEdge); 
	PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Fsys_Delay_4096);

	//initialize RTC to 0.
	sys_safe_access_enable();
	R32_RTC_TRIG = 0;
	R32_RTC_CTRL |= RB_RTC_LOAD_HI;
	R32_RTC_CTRL |= RB_RTC_LOAD_LO;
	R8_RTC_MODE_CTRL |= RB_RTC_TRIG_EN;  //enable RTC trigger
   	R8_SLP_WAKE_CTRL |= RB_SLP_RTC_WAKE; // enable wakeup control
	sys_safe_access_disable();

    RTC_InitClock(Count_2047);
	RTC_InitTime(2025,8,22,3,20,0);
	PFIC_EnableIRQ( RTC_IRQn );
	PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_RTC_WAKE, Fsys_Delay_4096);//开启RTC唤醒使能

	imageCache = malloc(4736);
	uint8_t textcolor = BLACK;

	if(imageCache != NULL)
	{
		memset(imageCache,0x00,4736);
		//imageCache[0] = 0x01;
		//imageCache[1] = 0x0F;
		memcpy(imageCache,gImage_full,4736);
		paint_SetImageCache(imageCache);
		EPD_Printf(50,50,font16,textcolor,"Yay.");
		EPD_Init();
		EPD_SendDisplay(imageCache);
	}

	PFIC_EnableIRQ( GPIO_A_IRQn) ;
	MySleep(POWER_PIN);
	EPD_Sleep();
	//RFIP_WakeUpRegInit();
	//PFIC_DisableIRQ( GPIO_A_IRQn) ;
	RTC_TRIGFunCfg(32768*5);
	MySleep(POWER_PIN);

	EPD_Printf(50,70,font16,textcolor,"partial.");
	EPD_PartialDisplay(imageCache);
	MySleep(POWER_PIN);
	EPD_Sleep();

	uint16_t counter = 0;
	while(1)
	{
		counter++;
		uint16_t ph;
		uint16_t pm;
		uint16_t ps;
		
		RTC_GetTime(NULL,NULL,NULL,&ph,&pm,&ps);
		EPD_Printf(35,90,font16,textcolor,"count:%d",counter);
		EPD_Printf(35,110,font16,textcolor,
		"%02d:%02d:%02d",ph,pm,ps);
		if (refreshCount > 8)
		{
			EPD_Init();
			EPD_SendDisplay(imageCache);
			refreshCount = 0;
		}
		else
		{
			refreshCount++;
			EPD_PartialDisplay(imageCache);
		}

		MySleep(POWER_PIN);
		EPD_Sleep();

		RTC_TRIGFunCfg(32768*30);
		MySleep(POWER_PIN);

	}

	free(imageCache);
	//RTC_TRIGFunCfg(32768*300);
	MySleep(POWER_PIN);
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
