#ifndef __AHT20_H_
#define __AHT20_H_

#define SDA_PIN GPIO_Pin_9
#define SCL_PIN GPIO_Pin_8

#include "CH57x_common.h"

#define ACK 1
#define NACK 0
//CH570D还是没有开漏或者弱上拉输出，所以拉低是强拉低，释放
//是设置成上拉输入
#define SDA_LOW  (R32_PA_DIR |= SDA_PIN) //变为输出，输出值本来就是0
#define SDA_HIGH (R32_PA_DIR &= ~(SDA_PIN))//变为输入上拉
#define SCL_LOW  R32_PA_CLR |= SCL_PIN
#define SCL_HIGH R32_PA_SET |= SCL_PIN
#define READ_SDA() (R32_PA_PIN & SDA_PIN)

#define I2CDelayUs(x) 

void SoftI2CInit(void);
void I2CStart(void);
void I2CStop(void);
uint8_t I2C_Write(uint8_t dat);
uint8_t I2C_Read(uint8_t ack);
void OLED_Init(void);
void OLED_GDDRAM(uint8_t * data);
void OLED_TurnOn(void);
void OLED_TurnOff(void);

#endif
