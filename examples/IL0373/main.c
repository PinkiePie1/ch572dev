
/****************************************************************
 * File Name          : Main.c
 * Author             : JCQ
 * Version            : V1.0
 * Date               : 2025/04/07
 * Description        : 裸机驱动墨水屏
 *********************************************************************************/
#include "CH57x_common.h"
#include "main.h"
#include <stdio.h>
__attribute__((aligned(4))) uint8_t imageCache[2888] = {0};//显存，为了提高memcpy的速度需要四字节对齐。

//和memset
static inline void mymemset(void *dest, int c, size_t n) { unsigned char *s = dest; for (; n; n--, s+=4) *s = c; }

static void GPIOInit(void)
{
	GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
	EPD_Hal_Init();
}

void main(void)
{
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);    
	GPIOInit();


	paint_SetImageCache(imageCache);
	drawStr(50,150,"test 1",font14,WHITE);
	fastDrawString(70,150,"fast",font14);


	EPD_Init();	
	EPD_SendDisplay(imageCache);
    GPIOA_ITModeCfg(EPD_BUSY_PIN, GPIO_ITMode_RiseEdge); 
    PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Fsys_Delay_4096);
    PFIC_EnableIRQ( GPIO_A_IRQn) ;
    LowPower_Sleep(RB_PWR_RAM12K);
	EPD_Sleep();
	LowPower_Sleep(RB_PWR_RAM12K);
	EPD_DeepSleep();
	PFIC_DisableIRQ( GPIO_A_IRQn);
	PWR_PeriphWakeUpCfg(DISABLE, RB_SLP_GPIO_WAKE, Fsys_Delay_4096);
	LowPower_Sleep(RB_PWR_RAM12K);
	while(1);


	
	
}


__INTERRUPT
__HIGH_CODE
void GPIOA_IRQHandler(void)
{
    GPIOA_ClearITFlagBit(0xFFFF);
    
}
