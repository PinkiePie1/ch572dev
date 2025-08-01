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
#include "EPD_1IN54_SSD1681.h"
#include <stdlib.h>

uint32_t humid;
uint32_t temperature;
uint8_t *imageCache;
uint8_t refreshCount = 250;
uint8_t img_index = 0;
uint8_t rawData[9];

//sleep.
void MySleep(uint8_t use5v);


void main(void)
{
	SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);
	EPD_Hal_Init();
    GPIOA_ITModeCfg(EPD_BUSY_PIN, GPIO_ITMode_FallEdge); 
    PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Fsys_Delay_4096);
	EPD_Init();	
	EPD_Sleep();
	SoftI2CInit();

measure:
	SHT40_beginMeasure();
	DelayMs(3);
	SHT40_getDat(rawData);

	humid = (rawData[3] << 8);
	humid |= rawData[4];
	humid = (humid >> 3) + (humid >> 4) + (humid >> 9) + (humid >> 10);

	temperature = (rawData[0] << 8);
	temperature |= rawData[1];
	temperature = (temperature >> 2) + (temperature >> 6) + (temperature >> 10);

	temperature = temperature - 4500;
	humid = humid-600;

	imageCache = malloc(5000);
	uint8_t textcolor = BLACK;
	img_index = (0x0001&temperature);
	if(imageCache != NULL)
	{
		if (img_index == 0)
		{
			memcpy(imageCache,gImage_alicew,5000);
			textcolor = BLACK;
		}
		else if (img_index == 1)
		{
			memcpy(imageCache,gImage_aliceb,5000);
			textcolor = WHITE;
		}
		else
		{
			img_index = 0;
		}

		paint_SetImageCache(imageCache);

		EPD_Printf(0,0,font16,textcolor,"T:%02d.%02d",temperature/100,temperature%100);
		EPD_Printf(0,16,font16,textcolor,"H:%02d.%02d%%",humid/100,humid%100);

		//send dispaly data, partial refresh 8 times.

		if(refreshCount < 5)
		{	
			EPD_PartialDisplay(imageCache);
			refreshCount++;
		}
		else
		{
			EPD_Init();	
			EPD_SendDisplay(imageCache);
			refreshCount = 0;			
		}
	}

	free(imageCache);
	
    PFIC_EnableIRQ(GPIO_A_IRQn);
	MySleep(0);
	EPD_Sleep();

	sys_safe_access_enable();
	R32_RTC_TRIG = 0;
	R32_RTC_CTRL |= RB_RTC_LOAD_HI;
	R32_RTC_CTRL |= RB_RTC_LOAD_LO;
	R8_RTC_MODE_CTRL |= RB_RTC_TRIG_EN;  //enable RTC trigger
   	R8_SLP_WAKE_CTRL |= RB_SLP_RTC_WAKE; // enable wakeup control
	sys_safe_access_disable();

	uint32_t alarm = (uint32_t) R16_RTC_CNT_LSI | ( (uint32_t) R16_RTC_CNT_DIV1 << 16 );
	alarm += 32000*30;

	sys_safe_access_enable();
	R32_RTC_TRIG = alarm;   
	sys_safe_access_disable();

	PFIC_EnableIRQ(RTC_IRQn);

	MySleep(0);
	goto measure;
}


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
    ldoconfig |= RB_PWR_PLAN_EN | RB_PWR_CORE | RB_PWR_RAM12K |(1<<12) ;
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

__INTERRUPT
__HIGH_CODE
void GPIOA_IRQHandler(void)
{
    GPIOA_ClearITFlagBit(EPD_BUSY_PIN);
    
}

__INTERRUPT
__HIGH_CODE
void RTC_IRQHandler(void)
{
	R8_RTC_FLAG_CTRL =  RB_RTC_TRIG_CLR;

}
