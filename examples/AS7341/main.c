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
#include "AS7341.h"
#include "EPD_2IN9_SSD1680.h"
#include <stdlib.h>

uint8_t *imageCache;


void main(void)
{
	sys_safe_access_enable();
	R8_CLK_SYS_CFG = CLK_SOURCE_HSE_PLL_100MHz;
	sys_safe_access_disable();


	SoftI2CInit();
	I2CStart();
	uint8_t ack = I2C_Write(0x39<<1);
	I2CStop();
	uint8_t buffer[3] = {0};
	AS7341_ReadReg(0x92,buffer,1);


	EPD_Hal_Init();
	imageCache = malloc(4736);
	uint8_t textcolor = BLACK;

	if(imageCache != NULL)
	{
		memset(imageCache,0x00,4736);
		paint_SetImageCache(imageCache);
		EPD_Printf(1,50,font16,textcolor,"Yay.");
		EPD_Printf(1,70, font16, textcolor, "ack returns:%d", ack);
		EPD_Printf(1,90, font16, textcolor, "chipid:%d", buffer[0]);
		EPD_Init();
		EPD_SendDisplay(imageCache);
	}

	DelayMs(2000);
	EPD_Sleep();
	free(imageCache);
	while(1);

}
