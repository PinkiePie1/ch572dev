
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
	EPD_Hal_Init();
	GPIOA_ModeCfg(GPIO_Pin_9,GPIO_ModeOut_PP_5mA);
	GPIOA_SetBits(GPIO_Pin_9);
}

void main(void)
{
	//在sys.c里已经写了highcode_init，并且会放到startup之后
	//所以在这里初始化时钟是不必要的。	
	GPIOInit();
	
	paint_SetImageCache(imageCache);
	drawStr(50,150,"test 1",font14,WHITE);
	fastDrawString(70,150,"fast",font14);

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
