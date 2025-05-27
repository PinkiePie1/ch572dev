/****************************************************************
 * File Name          : Main.c
 * Author             : JCQ
 * Version            : V1.0
 * Date               : 2025/04/07
 * Description        : 裸机驱动墨水屏
 *********************************************************************************/
#include "CH58x_common.h"
#include "main.h"


static void GPIOInit(void)
{
	GPIOA_ModeCfg(GPIO_Pin_9,GPIO_ModeOut_PP_5mA);
}

void main(void)
{
	//在sys.c里已经写了highcode_init，并且会放到startup之后
	//所以在这里初始化时钟是不必要的。	
	tickDelayInit();
	GPIOInit();

	while( 1 )
        {
            GPIOA_SetBits(GPIO_Pin_9);
            tickDelayMs(900);
            GPIOA_ResetBits(GPIO_Pin_9);
           tickDelayMs(900);
        }

}
