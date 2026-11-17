/*-----------------------------------------------------------------------------
* @文件名        : env_control
* @描述          : 灯（PB15）与风扇（PA8）控制驱动
* @作者          : 物联网项目组
* @日期          : 2024/05/01
* @版本          : V1.0.0
*******************************************************************************
功能介绍：
1、灯：PB15，使用 TIM15 通道 2 输出 PWM，可开关，也可用 0~100 档位调光
2、风扇：PA8，普通推挽输出，高电平开启、低电平关闭
******************************************************************************/

#ifndef __ENV_CONTROL_H
#define __ENV_CONTROL_H

#include "stm32l4xx_hal.h"

/* 风扇有效电平：1 表示 PA8 输出高电平时风扇转；如果硬件是低电平有效，改成 0 */
#define ENV_FAN_ACTIVE_HIGH      1

void Env_Actuator_Init(void);

void Env_Lamp_Switch(uint8_t on);          /* 灯开关：0 关，1 开 */
void Env_Lamp_Level(uint8_t level);        /* 灯亮度档位：0~100 */
void Env_Fan_Switch(uint8_t on);           /* 风扇开关：0 关，1 开 */

uint8_t Env_Lamp_Get_State(void);
uint8_t Env_Lamp_Get_Level(void);
uint8_t Env_Fan_Get_State(void);

#endif
