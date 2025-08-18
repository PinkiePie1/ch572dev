/****************************************************************
 * File Name          : Main.c
 * Author             : JCQ
 * Version            : V1.0
 * Date               : 2025/04/07
 * Description        : 裸机驱动墨水屏
 *********************************************************************************/
#include "CH57x_common.h"
#include "main.h"
#include "ICM42688.h"
#include <stdio.h>


static void GPIOInit(void)
{
	GPIOA_ModeCfg(GPIO_Pin_9,GPIO_ModeOut_PP_5mA);
}

static void blink(void)
{
	while(1)
	{
		GPIOA_InverseBits(GPIO_Pin_9);
		DelayMs(20);
	}
}

static void DebugInit(void)
{
    GPIOA_SetBits(bTXD_0);
    GPIOA_ModeCfg(bRXD_0, GPIO_ModeIN_PU);      // RXD-配置上拉输入
    GPIOA_ModeCfg(bTXD_0, GPIO_ModeOut_PP_5mA); // TXD-配置推挽输出，注意先让IO口输出高电平
    UART_Remap(ENABLE, UART_TX_REMAP_PA3, UART_RX_REMAP_PA2);
    UART_DefInit();
}

void main(void)
{
        HSECFG_Capacitance(HSECap_18p);
        SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);
        GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);

        DebugInit();
        DelayMs(200);
        PRINT("hi!");

	    DelayMs(500);
	    char buffer[40] = "init success.";

        //GPIOA_SetBits(GPIO_Pin_9);
        if(ICM_Init() == 0)
        {
        	GPIOA_SetBits(GPIO_Pin_9);
        	UART_SendString(buffer, sizeof(buffer));
        } else {
        	blink();
        }

        ICM_Begin();
        DelayMs(20);
        while(1)
        {
        	DelayMs(2000);
        	ICM_ReadAll(buffer);
        	UART_SendString(buffer, 14);
		GPIOA_ResetBits(GPIO_Pin_9);
		DelayMs(20);
        	GPIOA_SetBits(GPIO_Pin_9);
        }
 
}
