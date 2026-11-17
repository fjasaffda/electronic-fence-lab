/**
  ******************************************************************************
  * File Name          : GPIO_API.h
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gpio_api_H
#define __gpio_api_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "stdint.h"
#include "gpio.h"

#define KEY1          0x01
#define KEY2          0x02
#define KEY3          0x04
#define KEY4          0x08

#define LED1_PORT      LED0_Pin
#define LED2_PORT      LED1_Pin
#define LED3_PORT      LED2_Pin
#define LED4_PORT      LED3_Pin

typedef enum 
{
    BC35G_SW=0x01,
    BEEP=0x02,
    TFTS=0x04,
    LEDS=0x08,
    KEYS=0x10,
    E2PROOM=0x20
}GPIO_MODULE;

typedef enum 
{
    LED0=(uint16_t)0x0004,
    LED1=(uint16_t)0x0008,
    LED2=(uint16_t)0x0010,  
    LED3=(uint16_t)0x0020,     
}MODULE_LEDNUM;


typedef enum
{
  PIN_RESET = 0U,
  PIN_SET
}GPIO_PINSTATE;

/*
beep
*/
void gpio_module_open(uint8_t module);
void gpio_module_close(uint8_t module);

void led_open(uint16_t lednum);
void led_close(uint16_t lednum);
void led_toggle(uint16_t lednum);
void beep_open(void);
void beep_close(void);
uint8_t keyscan(void);

/*
TFT gpio
*/
#define LCD_LED_CLR             HAL_GPIO_WritePin(TFT_BL_GPIO_Port, TFT_BL_Pin, GPIO_PIN_RESET)     	// 关背光 
#define LCD_LED_SET             HAL_GPIO_WritePin(TFT_BL_GPIO_Port, TFT_BL_Pin, GPIO_PIN_SET)  			// 开背光

#define LCD_CS_CLR              HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET)       	// 片选置低
#define LCD_CS_SET              HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET)         	// 片选置高

#define LCD_RST_CLR             HAL_GPIO_WritePin(TFT_RESET_GPIO_Port,  TFT_RESET_Pin, GPIO_PIN_RESET)	// 复位引脚置高
#define LCD_RST_SET             HAL_GPIO_WritePin(TFT_RESET_GPIO_Port,  TFT_RESET_Pin, GPIO_PIN_SET)	// 复位引脚置低

#define LCD_RS_CLR              HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET)       	// 数据/命令引脚置高
#define LCD_RS_SET              HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET)         	// 数据/命令引脚置低
/*
BC35G gpio
*/
#define BC35G_PWR_H 	HAL_GPIO_WritePin(BC35G_SW_GPIO_Port, BC35G_SW_Pin, GPIO_PIN_RESET);// 给BC35模块上电
#define BC35G_PWR_L 	HAL_GPIO_WritePin(BC35G_SW_GPIO_Port, BC35G_SW_Pin, GPIO_PIN_SET);  // 给BC35模块掉电


#ifdef __cplusplus
}
#endif
#endif /*__gpio_api_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
