/****************************************************************
 * File Name          : Main.c
 * Author             : JCQ
 * Version            : V1.0
 * Date               : 2025/04/07
 * Description        : 裸机驱动墨水屏
 *********************************************************************************/
#include "CH57x_common.h"
#include "main.h"
#include "ws2812.h"


static void GPIOInit(void)
{
	GPIOA_ModeCfg(GPIO_Pin_9,GPIO_ModeOut_PP_5mA);
}

void main(void)
{
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
	GPIOInit();

	uint8_t dat[9] = {
		0x0F,
		0x00,
		0x00,
		0x00,
		0x0F,
		0x00,
		0x00,
		0x00,
		0x0F
	};

	uint8_t dat2[9] = {
		0x00,
		0x0F,
		0x00,
		0x00,
		0x00,
		0x0F,
		0x0F,
		0x00,
		0x00
	};
	while(1)
	{
		int i = 16;
		do{
		WS2812BSimpleSend(GPIO_Pin_9,dat,9);
		} while (i--);
		DelayMs(1500);
		
		i = 16;
		do{
		WS2812BSimpleSend(GPIO_Pin_9,dat2,9);
		} while (i--);
		DelayMs(1500);
	}



}
