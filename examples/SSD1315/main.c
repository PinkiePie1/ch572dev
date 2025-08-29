/****************************************************************
 * File Name          : Main.c
 * Author             : JCQ
 * Version            : V1.0
 * Date               : 2025/04/07
 * Description        : 裸机驱动墨水屏
 *********************************************************************************/
#include "CH57x_common.h"
#include "main.h"
#include "SSD1315.h"

__attribute__((__aligned__(4))) uint8_t imageCache[1024]={0};

#define LED_PIN GPIO_Pin_11

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
    uint16_t td = 300;
#if 0
    SoftI2CInit();
	I2CStart();
	td = I2C_Write(0x78);//SSD1315 address
	td = I2C_Write(0x40);//control bit, indicating the next data is a command.
	td = I2C_Write(0xAF);
	I2CStop();

    if (td == 1){td = 800;}
    else {td = 100;}

#else

    OLED_Init();
    imageCache[0] = 0x01;
    imageCache[2] = 0xFF;

    for (uint16_t i = 0; i < 1024; i++) 
    {
        imageCache[i] = (i & 0x00FF);
    }

    OLED_GDDRAM(imageCache);
    OLED_TurnOn();
    DelayMs(5000);

    memset(imageCache,0x0F,1024);
    OLED_GDDRAM(imageCache);

    while(1){
        DelayMs(1000);
        memset(imageCache,0x0F,1024);
        OLED_GDDRAM(imageCache);
        DelayMs(1000);
        memset(imageCache,0xF0,1024);
        OLED_GDDRAM(imageCache);
    }
    //DelayMs(5000);
    //OLED_TurnOff();
#endif

    while( 1 )
    {
        GPIOA_SetBits(LED_PIN);
        DelayMs(td);
        GPIOA_ResetBits(LED_PIN);
        DelayMs(td);
    }

}
