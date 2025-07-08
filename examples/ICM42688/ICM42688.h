#ifndef __ICM42688_H_
#define __ICM42688_H_

#include "CH57x_common.h"

#define CS_HIGH GPIOA_SetBits(GPIO_Pin_4)
#define CS_LOW GPIOA_ResetBits(GPIO_Pin_4)


uint8_t ICM_Init(void);
void ICM_Begin(void);
void ICM_ReadAll(uint8_t *buffer);


#endif
