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

    ST7305_Init();
    DelayMs(10);
    uint8_t flip  = 0;
    uint8_t buf[8064] = {0};
    while(1)
    {
        
        flip = flip == 0x00?0xFF:0x00;
        for(uint16_t i=0;i<8064;i++){buf[i]=flip;}
        DelayMs(900);
        ST7305_RAM(buf);
    }

    while(1);

}
