/****************************************************************
 * File Name          : Main.c
 * Author             : JCQ
 * Version            : V1.0
 * Date               : 2025/04/07
 * Description        : 裸机驱动墨水屏
 *********************************************************************************/
#include "CH57x_common.h"
#include "main.h"
#include "ST7305.h"

#define LED_PIN GPIO_Pin_9

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

    for(uint16_t i = 0; i < 5; i++)
    {
        GPIOA_SetBits(LED_PIN);
        DelayMs(100);
        GPIOA_ResetBits(LED_PIN);
        DelayMs(100);
    }


    ST7305_Init();
    DelayMs(10);
    uint8_t * display = {0};
    ST7305_RAM(display);

    while(1)
    {
        GPIOA_SetBits(LED_PIN);
        DelayMs(3000);
        GPIOA_ResetBits(LED_PIN);
        DelayMs(3000);
        ST7305_RAM(display);
    }

    while(1);

}
