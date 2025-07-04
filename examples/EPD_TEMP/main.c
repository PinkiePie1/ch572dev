
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
#include "AHT20.h"
__attribute__((aligned(4))) uint8_t imageCache[2888] = {0};//显存，为了提高memcpy的速度需要四字节对齐。


void main(void)
{
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);
	SoftI2CInit();
	uint8_t rawData[9];
	uint32_t humid;
	uint32_t temperature;

	AHT20_beginMeasure();

	DelayMs(45);
	AHT20_getDat(rawData);
	humid = rawData[1];
	humid <<= 8; 
	humid |= rawData[2];
	humid <<= 8; 
	humid |= (rawData[3] & 0xF0);
	humid >>= 4;
	humid = (humid >> 7)+
	(humid >> 10)+
	(humid >> 11)+
	(humid >> 12)+
	(humid >> 16);

	temperature = 0;
	temperature = rawData[3] & 0x0F;
	temperature <<= 8;
	temperature |= rawData[4];
	temperature <<= 8;
	temperature |= rawData[5];
	temperature = (temperature >> 6) + 
				(temperature >> 9) +
				(temperature >> 10) +
				(temperature >> 11) +
				(temperature >> 15) ;
									
	temperature = temperature - 5000;

	paint_SetImageCache(imageCache);
	EPD_Printf(10,150,font14,BLACK,"TMP: %d, HUM: %d.",temperature,humid);
//	drawStr(50,150,"test 1",font14,WHITE);
	
	EPD_Init();	
	EPD_SendDisplay(imageCache);
	EPD_Sleep();

	DelayMs(30000);
	memset(imageCache,0,2888);

	EPD_Init();	
	EPD_SendDisplay(imageCache);
	EPD_Sleep();

	while(1)
	{
		GPIOA_SetBits(GPIO_Pin_9);
		DelayMs(500);
		GPIOA_ResetBits(GPIO_Pin_9);
		DelayMs(500);
	}
	
	
}
