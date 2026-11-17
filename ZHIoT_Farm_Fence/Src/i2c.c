/**
  ******************************************************************************
  * File Name          : I2C.c
  * Description        : This file provides code for the configuration
  *                      of the I2C instances.
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
/*
1、采用轮询方式实现
2、采用中断方式实现
*/
#include "i2c.h"
#include "i2c_api.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;

ZH_I2C_TypeDef  midi2c2;
ZH_I2C_TypeDef  midi2c3;

__weak void i2c_error_handler(void);

/*******************************************************************************
* 名    称：i2c_open
* 功    能：打开i2c
* 参数说明：port：i2c端口号
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void i2c_open(I2C_port  port)
{
        
    if(port & i2c_2)
    {
        midi2c2.State = I2C_STATE_OPEN_IDLE;   
        MX_I2C2_Init();
             
    }
    if(port & i2c_3)
    { 
        midi2c3.State = I2C_STATE_OPEN_IDLE; 
        MX_I2C3_Init();
        
    }
}

/*******************************************************************************
* 名    称：i2c_close
* 功    能：关闭i2c
* 参数说明：port：i2c端口号
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void i2c_close(I2C_port  port)
{
    
    if(port & i2c_2)
    {
        midi2c2.State = I2C_STATE_CLOSED; 
        HAL_I2C_MspDeInit(&hi2c2);
                
    }
    if(port & i2c_3)
    { 
        midi2c3.State = I2C_STATE_CLOSED; 
        HAL_I2C_MspDeInit(&hi2c3);
    }
}

/*******************************************************************************
* 名    称：i2c_write
* 功    能：通i2c写数据
* 参数说明：port：iic端口号
			DevAddress：写入数据的地址
			*ptrdata：需要写入的数据
			length：读取数据的长度
* 返 回 值：写入的值
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void i2c_write(I2C_port port,uint16_t DevAddress,uint8_t* ptrdata,uint16_t length)
{
    if(port == i2c_2)
    {
        midi2c2.State = I2C_STATE_OPEN_IDLE;         
        HAL_I2C_Master_Transmit(&hi2c2,DevAddress,ptrdata,length,500);
        midi2c2.State = I2C_STATE_BUSY_TX; 
    }
    else  if(port == i2c_3)
    {
        midi2c3.State = I2C_STATE_OPEN_IDLE;         
        HAL_I2C_Master_Transmit(&hi2c3,DevAddress,ptrdata,length,500);
        midi2c3.State = I2C_STATE_BUSY_TX; 
    }
}

/*******************************************************************************
* 名    称：i2c_read
* 功    能：通过I2C读取数据
* 参数说明：port：iic端口号
			DevAddress：读取数据的地址
			length：读取数据的长度
* 返 回 值：读取到的值
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
uint8_t * i2c_read(I2C_port port,uint16_t DevAddress,uint16_t length)
{
    if(port == i2c_2)
    {
        midi2c2.State = I2C_STATE_BUSY_RX; 
       if( HAL_I2C_Master_Receive(&hi2c2,DevAddress,midi2c2.prxdata,length,500) == HAL_OK)
       { 
           midi2c2.rxlen = length;
           return midi2c2.prxdata;           
       }
    }
    else  if(port == i2c_3)
    {
        midi2c3.State = I2C_STATE_BUSY_RX; 
       if( HAL_I2C_Master_Receive(&hi2c3,DevAddress,midi2c3.prxdata,1,500) == HAL_OK)
       { 
            midi2c3.rxlen = length;
           return midi2c3.prxdata;
           
       }
    }
    return NULL;
}

/* I2C2 init function */
void MX_I2C2_Init(void)
{
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x10909CEC;//10909CEC
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    i2c_error_handler();
  }
  /**Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    i2c_error_handler();
  }
  /**Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    i2c_error_handler();
  }
}
/* I2C3 init function */
void MX_I2C3_Init(void)
{

  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x10909CEC;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    i2c_error_handler();
  }
  /**Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    i2c_error_handler();
  }
  /**Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    i2c_error_handler();
  }

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C2)
  {
  /* USER CODE BEGIN I2C2_MspInit 0 */

  /* USER CODE END I2C2_MspInit 0 */
  
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C2 clock enable */
    __HAL_RCC_I2C2_CLK_ENABLE();
  /* USER CODE BEGIN I2C2_MspInit 1 */

  /* USER CODE END I2C2_MspInit 1 */
  }
  else if(i2cHandle->Instance==I2C3)
  {
  /* USER CODE BEGIN I2C3_MspInit 0 */

  /* USER CODE END I2C3_MspInit 0 */
  
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* I2C3 clock enable */
    __HAL_RCC_I2C3_CLK_ENABLE();
  /* USER CODE BEGIN I2C3_MspInit 1 */

  /* USER CODE END I2C3_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C2)
  {
  /* USER CODE BEGIN I2C2_MspDeInit 0 */

  /* USER CODE END I2C2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C2_CLK_DISABLE();
  

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13|GPIO_PIN_14);

  /* USER CODE BEGIN I2C2_MspDeInit 1 */

  /* USER CODE END I2C2_MspDeInit 1 */
  }
  else if(i2cHandle->Instance==I2C3)
  {
  /* USER CODE BEGIN I2C3_MspDeInit 0 */

  /* USER CODE END I2C3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C3_CLK_DISABLE();
  

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0|GPIO_PIN_1);

  /* USER CODE BEGIN I2C3_MspDeInit 1 */

  /* USER CODE END I2C3_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
