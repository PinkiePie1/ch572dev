/****************************************************************
 * File Name          : Main.c
 * Author             : JCQ
 * Version            : V1.0
 * Date               : 2025/04/07
 * Description        : 比较器充当4位ADC
 *********************************************************************************/
#include "CH57x_common.h"
#include "main.h"

#define LED_PIN GPIO_Pin_8


static void GPIOInit(void)
{
    GPIOA_ModeCfg(LED_PIN,GPIO_ModeOut_PP_5mA);
}

void main(void)
{
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);
    GPIOA_ModeCfg(LED_PIN, GPIO_ModeOut_PP_5mA);
    GPIOInit();

    while( 1 )
    {
        GPIOA_SetBits(LED_PIN);
        DelayMs(500);
        GPIOA_ResetBits(LED_PIN);
        DelayMs(50);
    }

}
