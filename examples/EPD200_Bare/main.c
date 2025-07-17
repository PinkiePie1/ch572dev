
#include "CH57x_common.h"
#include "main.h"
#include "imageData.h"
#include <stdio.h>
__attribute__((aligned(4))) uint8_t imageCache[5000] = {0};//显存，为了提高memcpy的速度需要四字节对齐。

//和memset
static inline void mymemset(void *dest, int c, size_t n) { unsigned char *s = dest; for (; n; n--, s+=4) *s = c; }

static void GPIOInit(void)
{
	EPD_Hal_Init();
}

void main(void)
{
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);
	GPIOInit();

	memcpy(imageCache, gImage_white1, 5000);
	
	paint_SetImageCache(imageCache);

	fastDrawString(0,0,"fast !\"#$%%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOP",font16,BLACK);

	EPD_Init();	
	EPD_SendDisplay(imageCache);
	DelayMs(2000);
	EPD_Sleep();


	while(1)
	{
	}
	
	
}
