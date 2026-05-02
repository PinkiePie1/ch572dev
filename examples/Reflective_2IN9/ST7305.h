#ifndef __ST7305_H__
#define __ST7305_H__

#include "CH57x_common.h"

#ifndef ST7305_SPI_MODE
#define ST7305_SPI_MODE 1
//soft SPI
#endif


//bit bang SPI pin definitions
#define TE_PIN GPIO_Pin_2
#define CS_PIN GPIO_Pin_11
#define DC_PIN GPIO_Pin_10
#define RES_PIN GPIO_Pin_6
#define SDI_PIN GPIO_Pin_7
#define SCLK_PIN GPIO_Pin_5

#define TE_ISHIGH (GPIOA_ReadPortPin(TE_PIN) != 0)
#define TE_ISLOW (GPIOA_ReadPortPin(TE_PIN) == 0)
#define CS_LOW GPIOA_ResetBits(CS_PIN)
#define CS_HIGH GPIOA_SetBits(CS_PIN)
#define DC_LOW GPIOA_ResetBits(DC_PIN)
#define DC_HIGH GPIOA_SetBits(DC_PIN)
#define RES_LOW GPIOA_ResetBits(RES_PIN)
#define RES_HIGH GPIOA_SetBits(RES_PIN)
#define SDI_LOW GPIOA_ResetBits(SDI_PIN)
#define SDI_HIGH GPIOA_SetBits(SDI_PIN)
#define SCLK_LOW GPIOA_ResetBits(SCLK_PIN)
#define SCLK_HIGH GPIOA_SetBits(SCLK_PIN)

//Probabily won't need this
#define Dev_DelayUs(x) DelayUs(x)
#define Dev_DelayMs(x) DelayMs(x)

void ST7305_Init(void);
void ST7305_RAM(uint8_t * display);
#endif