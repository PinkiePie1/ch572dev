/****************************************************************
 * File Name          : Main.c
 * Author             : JCQ
 * Version            : V1.0
 * Date               : 2025/04/07
 * Description        : 裸机驱动墨水屏
 *********************************************************************************/
#include "CH57x_common.h"
#include "main.h"


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

	while( 1 )
        {
            GPIOA_SetBits(GPIO_Pin_9);
            DelayMs(500);
            GPIOA_ResetBits(GPIO_Pin_9);
            DelayMs(50);
        }

}
