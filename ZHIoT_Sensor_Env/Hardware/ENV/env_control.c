/*-----------------------------------------------------------------------------
* @文件名        : env_control
* @描述          : 灯（PB15）与风扇（PA8）控制驱动
* @作者          : 物联网项目组
* @日期          : 2024/05/01
* @版本          : V1.0.0
******************************************************************************/

#include "env_control.h"
#include "pwm_api.h"

/********************************内部定义**************************************/
#define ENV_LAMP_Pin            GPIO_PIN_15
#define ENV_LAMP_GPIO_Port      GPIOB

#define ENV_FAN_Pin             GPIO_PIN_8
#define ENV_FAN_GPIO_Port       GPIOA

static uint8_t env_lamp_state = 0;      /* 灯当前状态 0：关  1：开 */
static uint8_t env_lamp_level = 60;     /* 灯当前亮度档位 */
static uint8_t env_fan_state  = 0;      /* 风扇当前状态 0：关  1：开 */

/********************************************************************************
* 名    称：Env_Actuator_Init
* 功    能：灯、风扇初始化
* 参数说明：无
* 返 回 值：无
* 说    明：风扇 PA8 配置为推挽输出并默认关闭；灯 PB15 由 TIM15 通道 2 输出 PWM
* 调用方法：外部函数
********************************************************************************/
void Env_Actuator_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* 风扇 PA8：推挽输出，默认关闭 */
    HAL_GPIO_WritePin(ENV_FAN_GPIO_Port, ENV_FAN_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = ENV_FAN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ENV_FAN_GPIO_Port, &GPIO_InitStruct);

    /* 灯 PB15：TIM15_CH2，上电默认占空比 0（关灯） */
    pwm_open(0);

    env_lamp_state = 0;
    env_lamp_level = 60;
    env_fan_state  = 0;
}

/********************************************************************************
* 名    称：Env_Lamp_Switch
* 功    能：灯的开关控制
* 参数说明：on：0 关灯  1 开灯
* 返 回 值：无
* 说    明：开灯时按当前亮度档位输出 PWM，关灯时占空比置 0
* 调用方法：外部函数
********************************************************************************/
void Env_Lamp_Switch(uint8_t on)
{
    env_lamp_state = (on == 0) ? 0 : 1;

    if(env_lamp_state)
    {
        pwm_set(env_lamp_level);
    }
    else
    {
        pwm_set(0);
    }
}

/********************************************************************************
* 名    称：Env_Lamp_Level
* 功    能：灯亮度档位控制
* 参数说明：level：0~100
* 返 回 值：无
* 说    明：灯处于打开状态时立即生效
* 调用方法：外部函数
********************************************************************************/
void Env_Lamp_Level(uint8_t level)
{
    if(level > 100)
    {
        level = 100;
    }

    env_lamp_level = level;

    if(env_lamp_state)
    {
        pwm_set(env_lamp_level);
    }
}

/********************************************************************************
* 名    称：Env_Fan_Switch
* 功    能：风扇开关控制
* 参数说明：on：0 关  1 开
* 返 回 值：无
* 说    明：PA8 高电平开启风扇，低电平关闭风扇
* 调用方法：外部函数
********************************************************************************/
void Env_Fan_Switch(uint8_t on)
{
    GPIO_PinState on_level;
    GPIO_PinState off_level;

    env_fan_state = (on == 0) ? 0 : 1;

#if ENV_FAN_ACTIVE_HIGH
    on_level  = GPIO_PIN_SET;
    off_level = GPIO_PIN_RESET;
#else
    on_level  = GPIO_PIN_RESET;
    off_level = GPIO_PIN_SET;
#endif

    HAL_GPIO_WritePin(ENV_FAN_GPIO_Port, ENV_FAN_Pin, env_fan_state ? on_level : off_level);
}

uint8_t Env_Lamp_Get_State(void)
{
    return env_lamp_state;
}

uint8_t Env_Lamp_Get_Level(void)
{
    return env_lamp_level;
}

uint8_t Env_Fan_Get_State(void)
{
    return env_fan_state;
}
