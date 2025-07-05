
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
	uint8_t rawData[9];
	uint32_t humid = 0;
	uint32_t temperature = 0;

    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);
    EPD_Hal_Init();




	EPD_Init();	
	EPD_Sleep();

    R16_PIN_ALTERNATE = 0;
    GPIOA_ModeCfg(GPIO_Pin_1,GPIO_ModeOut_PP_5mA);
    GPIOA_SetBits(GPIO_Pin_1);
	GPIOA_ModeCfg(GPIO_Pin_8 | GPIO_Pin_9, GPIO_ModeIN_PU);

	SoftI2CInit();

	DelayMs(15);

	AHT20_beginMeasure();


	DelayMs(50);
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
	EPD_Printf(10,150,font14,BLACK,"TMP %d HUM: %d.",temperature,humid);
	EPD_Printf(40,150,font14,BLACK,"DAT:%02X,%02X,%02X,%02X,%02X,%02X,%02X",
		rawData[0],rawData[1],rawData[2],rawData[3],rawData[4],rawData[5],rawData[6]);

	
	EPD_Init();	
	EPD_SendDisplay(imageCache);
	GPIOA_ITModeCfg(EPD_BUSY_PIN, GPIO_ITMode_FallEdge);
    PFIC_EnableIRQ(GPIO_A_IRQn);
    PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Fsys_Delay_4096);
	LowPower_Sleep( RB_PWR_RAM12K | RB_PWR_EXTEND | RB_PWR_LDO5V_EN);
	EPD_Sleep();

	DelayMs(30000);
	memset(imageCache,0,2888);

	EPD_Init();	
	EPD_SendDisplay(imageCache);
	EPD_Sleep();

	while(1)
	{
	}
	
	
}
