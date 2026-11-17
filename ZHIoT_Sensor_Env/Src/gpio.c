/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "gpio_api.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/

#define KEY_PRESS     0

/*******************************************************************************
* 名    称：gpio_module_open
* 功    能：初始化对应模块的IO
* 参数说明：module：模块名
* 返 回 值：无
* 说    明：无
********************************************************************************/
void gpio_module_open(uint8_t module)   
{
     GPIO_InitTypeDef GPIO_InitStruct = {0};
     
    if(module & BC35G_SW)
    {
       /* GPIO Ports Clock Enable */
       __HAL_RCC_GPIOA_CLK_ENABLE(); 
      /*Configure GPIO pin Output Level */
       HAL_GPIO_WritePin(BC35G_SW_GPIO_Port, BC35G_SW_Pin, GPIO_PIN_SET);   
      /*Configure GPIO pin : PtPin */
      GPIO_InitStruct.Pin = BC35G_SW_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      HAL_GPIO_Init(BC35G_SW_GPIO_Port, &GPIO_InitStruct);              
    }
    if(module & BEEP)
    {
      /* GPIO Ports Clock Enable */
       __HAL_RCC_GPIOE_CLK_ENABLE();       
      /*Configure GPIO pin Output Level */
       HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);           
      /*Configure GPIO pins : PEPin PEPin */
      GPIO_InitStruct.Pin = BEEP_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      HAL_GPIO_Init(BEEP_GPIO_Port, &GPIO_InitStruct);      
    }
    if(module & E2PROOM)
    {
      /* GPIO Ports Clock Enable */
       __HAL_RCC_GPIOE_CLK_ENABLE();       
      /*Configure GPIO pin Output Level */
       HAL_GPIO_WritePin(W25Q128_GPIO_Port, W25Q128_Pin, GPIO_PIN_RESET);           
      /*Configure GPIO pins : PEPin PEPin */
      GPIO_InitStruct.Pin = W25Q128_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
      HAL_GPIO_Init(W25Q128_GPIO_Port, &GPIO_InitStruct);      
    }    
    if(module & TFTS)
    {
        /* GPIO Ports Clock Enable */
         __HAL_RCC_GPIOE_CLK_ENABLE(); 
        /*Configure GPIO pin Output Level */
        HAL_GPIO_WritePin(GPIOE, TFT_DC_Pin, GPIO_PIN_RESET);  
        HAL_GPIO_WritePin(GPIOE, TFT_CS_Pin|TFT_RESET_Pin|TFT_BL_Pin, GPIO_PIN_SET);
      /*Configure GPIO pins : PEPin PEPin PEPin */
      GPIO_InitStruct.Pin = TFT_CS_Pin|TFT_RESET_Pin|TFT_BL_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
      HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);  
        
      GPIO_InitStruct.Pin = TFT_DC_Pin;
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;  
      HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);          
    }
    if(module & LEDS)
    {
      /* GPIO Ports Clock Enable */
       __HAL_RCC_GPIOA_CLK_ENABLE();      
      /*Configure GPIO pin Output Level */
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED0_Pin|LED1_Pin|LED2_Pin|LED3_Pin, GPIO_PIN_SET);        
      /*Configure GPIO pins : PDPin PDPin PDPin PDPin */
      GPIO_InitStruct.Pin = LED0_Pin|LED1_Pin|LED2_Pin|LED3_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);      
    }
    if(module & KEYS)
    { 
      /* GPIO Ports Clock Enable */
       __HAL_RCC_GPIOD_CLK_ENABLE();
      /*Configure GPIO pin Output Level */
       HAL_GPIO_WritePin(KEY1_GPIO_Port, KEY1_Pin|KEY2_Pin|KEY3_Pin|KEY4_Pin, GPIO_PIN_SET);          
      /*Configure GPIO pins : PAPin PAPin PAPin PAPin */
      GPIO_InitStruct.Pin = KEY1_Pin|KEY2_Pin|KEY3_Pin|KEY4_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      HAL_GPIO_Init(KEY1_GPIO_Port, &GPIO_InitStruct);  

    }      
}

/*******************************************************************************
* 名    称：gpio_module_close
* 功    能：关闭对应模块的IO
* 参数说明：module：模块名
* 返 回 值：无
* 说    明：无
********************************************************************************/
void gpio_module_close(uint8_t module)
{
     
    if(module & BC35G_SW)
    {
        HAL_GPIO_DeInit(BC35G_SW_GPIO_Port,BC35G_SW_Pin);
    }
    if(module & BEEP)
    {
        HAL_GPIO_DeInit(BEEP_GPIO_Port,BEEP_Pin);        
    }
    if(module & TFTS)
    {  
        HAL_GPIO_DeInit(GPIOE,TFT_CS_Pin|TFT_RESET_Pin|TFT_BL_Pin|TFT_DC_Pin);          
    }
    if(module & LEDS)
    { 
        HAL_GPIO_DeInit(LED1_GPIO_Port,LED0_Pin|LED1_Pin|LED2_Pin|LED3_Pin); 
    }
    if(module & KEYS)
    { 
        HAL_GPIO_DeInit(KEY1_GPIO_Port,KEY1_Pin|KEY2_Pin|KEY3_Pin|KEY4_Pin);     
    }    
}

/*******************************************************************************
* 名    称：led_open
* 功    能：打开LED灯
* 参数说明：lednum：需要打开的灯的编号
* 返 回 值：无
* 说    明：无
********************************************************************************/
void led_open(uint16_t lednum)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, lednum,GPIO_PIN_RESET); 
}

/*******************************************************************************
* 名    称：led_close
* 功    能：关闭LED灯
* 参数说明：lednum：需要关闭的灯的编号
* 返 回 值：无
* 说    明：无
********************************************************************************/
void led_close(uint16_t lednum)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, lednum,GPIO_PIN_SET); 
}

/*******************************************************************************
* 名    称：led_toggle
* 功    能：翻转LED灯状态
* 参数说明：lednum：需要翻转的灯的编号
* 返 回 值：无
* 说    明：无
********************************************************************************/
void led_toggle(uint16_t lednum)
{
    HAL_GPIO_TogglePin(LED1_GPIO_Port, lednum); 
}

/*******************************************************************************
* 名    称：beep_open
* 功    能：打开蜂鸣器
* 参数说明：无
* 返 回 值：无
* 说    明：无
********************************************************************************/
void beep_open(void)
{
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin,GPIO_PIN_SET); 
}

/*******************************************************************************
* 名    称：beep_close
* 功    能：关闭蜂鸣器
* 参数说明：无
* 返 回 值：无
* 说    明：无
********************************************************************************/
void beep_close(void)
{
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin,GPIO_PIN_RESET); 
}

/*******************************************************************************
* 名    称：keyscan
* 功    能：扫描按键
* 参数说明：无
* 返 回 值：无
* 说    明：无
********************************************************************************/
uint8_t keyscan(void)
{ 
    uint8_t keynum=0;
    
    if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin) == KEY_PRESS)
      keynum+=KEY1;
    
    if(HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin) == KEY_PRESS)
      keynum+=KEY2;
    
    if(HAL_GPIO_ReadPin(KEY3_GPIO_Port,KEY3_Pin) == KEY_PRESS)
      keynum+=KEY3;
    
    if(HAL_GPIO_ReadPin(KEY4_GPIO_Port,KEY4_Pin) == KEY_PRESS)
      keynum+=KEY4;
    
    return keynum;
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
