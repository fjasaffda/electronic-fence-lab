#ifndef __LED_H
#define __LED_H

#include "stm32l4xx_hal.h"

/************************LED宏定义声明**************************************/
#define LED_OPEN  1
#define LED_OFF   0

/************************LED外部函数声明************************************/
extern void LED_Init(void);
extern void LED_Handle(uint8_t date);

#endif
