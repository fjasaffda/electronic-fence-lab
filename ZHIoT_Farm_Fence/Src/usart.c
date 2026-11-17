/**
  ******************************************************************************
  * File Name          : USART.c
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

/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "usart_api.h"
#include "stdio.h"
#include "string.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

UART_TypeDef  midlpuart1;
UART_TypeDef  miduart1;
UART_TypeDef  miduart2;
UART_TypeDef  miduart3;

static void Uart_Init(UART_port port,uint32_t BaudRate);

void MX_LPUART1_UART_Init(uint32_t BaudRate);
static void MX_USART1_UART_Init(uint32_t BaudRate);
static void MX_USART2_UART_Init(uint32_t BaudRate);
static void MX_USART3_UART_Init(uint32_t BaudRate);
static uint16_t UART_Data_Rec(UART_HandleTypeDef *huart, uint32_t * uarttick,uint32_t Timeout);
#define MAX_BAUDRATE     921600

uint32_t ig_lpuart1tick = 0;              // 串口1接受数据时间
uint32_t ig_uart1tick = 0;                // 串口1接受数据时间
uint32_t ig_uart2tick = 0;                // 串口2接受数据时间
uint32_t ig_uart3tick = 0;                // 串口3接受数据时间
uint8_t uart_flag = 0;
__weak void uart_error_handle(void);

/*******************************************************************************
* 名    称：uart_open
* 功    能：打开串口x，并设置波特率
* 参数说明：port：串口号
			BaudRate：波特率
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void uart_open(UART_port port,uint32_t BaudRate)
{
    Uart_Init(port,BaudRate);//uart 时钟、io、初始化，中断配置
}

/*******************************************************************************
* 名    称：uart_close
* 功    能：关闭串口x
* 参数说明：port：串口号
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void uart_close(UART_port port)
{
    if(port > UART_NUM)
    {

        return ;
    }  
    if((port == lpuart1)&&(midlpuart1.State != UART_STATE_CLOSED))
    {
        HAL_UART_MspDeInit(&hlpuart1);
        midlpuart1.State = UART_STATE_CLOSED;
    }
    else if((port == uart1)&&(miduart1.State != UART_STATE_CLOSED))    
    {
        HAL_UART_MspDeInit(&huart1);
        miduart1.State = UART_STATE_CLOSED;
    }
    else if((port == uart2)&&(miduart2.State != UART_STATE_CLOSED) )   
    {
        HAL_UART_MspDeInit(&huart2);
        miduart2.State = UART_STATE_CLOSED;
    }
    else if((port == uart3)&& (miduart3.State != UART_STATE_CLOSED))    
    {
        HAL_UART_MspDeInit(&huart3);
        miduart3.State = UART_STATE_CLOSED;
    }
    else   
    {

    } 
    // 数据结构参数清零    
}

/*******************************************************************************
* 名    称：uart_write
* 功    能：串口写数据
* 参数说明：port：串口号
			length：数据长度
			*ptrdata：串口写入的数据
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void uart_write(UART_port port,uint16_t length,uint8_t* ptrdata)
{
    if((port > UART_NUM)||(ptrdata == NULL)||(length == 0))
    {
        printf("error");
        return ;
    }   
    if((port == lpuart1)&&((midlpuart1.State == UART_STATE_OPEN_IDLE)\
        ||(midlpuart1.State == UART_STATE_BUSY_RX)))
    {
        midlpuart1.State = UART_STATE_BUSY_TX;
        HAL_UART_Transmit(&hlpuart1,(unsigned char *)ptrdata,length,300);  
        midlpuart1.State = UART_STATE_OPEN_IDLE;        
    }
    else if((port == uart1)&&((miduart1.State == UART_STATE_OPEN_IDLE)\
        ||(miduart1.State == UART_STATE_BUSY_RX)))
    {
        miduart1.State = UART_STATE_BUSY_TX;        
        HAL_UART_Transmit(&huart1,(unsigned char *)ptrdata,length,300); 
        miduart1.State = UART_STATE_OPEN_IDLE;
    }    
    else if((port == uart2)&&((miduart2.State == UART_STATE_OPEN_IDLE)\
        ||(miduart2.State == UART_STATE_BUSY_RX)))
    {
        miduart2.State = UART_STATE_BUSY_TX;        
        HAL_UART_Transmit(&huart2,(unsigned char *)ptrdata,length,300); 
        miduart2.State = UART_STATE_OPEN_IDLE;
    }   
    else if((port == uart3)&&((miduart3.State == UART_STATE_OPEN_IDLE)\
        ||(miduart3.State == UART_STATE_BUSY_RX)))
    {
        miduart3.State = UART_STATE_BUSY_TX;      
        HAL_UART_Transmit(&huart3,(unsigned char *)ptrdata,length,300); 
        miduart3.State = UART_STATE_OPEN_IDLE;
    }
    else
    {

    }    
}

/*******************************************************************************
* 名    称：clearrxbuffer
* 功    能：清除接收缓存
* 参数说明：port：串口号
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void clearrxbuffer(UART_port port)
{
    if(port == lpuart1)
    {
        memset(midlpuart1.prxdata,0,UART_RX_MAX_SIZE);
    }
    if(port == uart1)
    {
        memset(miduart1.prxdata,0,UART_RX_MAX_SIZE);
    }
    if(port == uart2)
    {
        memset(miduart2.prxdata,0,UART_RX_MAX_SIZE);
    }
    if(port == uart3)
    {
        memset(miduart3.prxdata,0,UART_RX_MAX_SIZE);
    }
}

/*******************************************************************************
* 名    称：uart_read
* 功    能：读串口数据
* 参数说明：port：串口号
			*length：长度
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
uint8_t * uart_read(UART_port port,uint16_t *length)
{
    if(port > UART_NUM)
    {

        return NULL;
    }
    
    if(port == lpuart1)
    {
        if((midlpuart1.State == UART_STATE_CLOSED) || (midlpuart1.State == UART_STATE_ERROR))
        {
            return NULL;
        }
        midlpuart1.State = UART_STATE_BUSY_RX;        
        if(HAL_UART_Receive_IT(&hlpuart1,midlpuart1.prxdata, 100) == HAL_OK ) // 最多可接受100个数据
        {
            midlpuart1.State = UART_STATE_OPEN_IDLE; 
            ig_lpuart1tick = 0;
            *length = midlpuart1.rxlen;
            midlpuart1.rxlen = 0;
            return (uint8_t *)midlpuart1.prxdata;
        }   

        midlpuart1.rxlen = UART_Data_Rec(&hlpuart1,&ig_lpuart1tick,100);        
    }
    else if(port == uart1)
    {
        if((miduart1.State == UART_STATE_CLOSED) || (miduart1.State == UART_STATE_ERROR))
        {
            return NULL;
        }
        miduart1.State = UART_STATE_BUSY_RX;        
        if(HAL_UART_Receive_IT(&huart1,miduart1.prxdata, 400) == HAL_OK ) // 最多可接受100个数据
        {
            miduart1.State = UART_STATE_OPEN_IDLE; 
            ig_uart1tick = 0;
            *length = miduart1.rxlen;
            miduart1.rxlen = 0;
            return (uint8_t *)miduart1.prxdata;
        }   

        miduart1.rxlen = UART_Data_Rec(&huart1,&ig_uart1tick,200);
    }    
    else if(port == uart2)
    {
        if((miduart2.State == UART_STATE_CLOSED) || (miduart2.State == UART_STATE_ERROR))
        {
            return NULL;
        }
        miduart2.State = UART_STATE_BUSY_RX;        
        if(HAL_UART_Receive_IT(&huart2,miduart2.prxdata, 200) == HAL_OK ) // 最多可接受100个数据
        {
            miduart2.State = UART_STATE_OPEN_IDLE; 
            ig_uart2tick = 0;
            *length = miduart2.rxlen;
            miduart2.rxlen = 0;
            return (uint8_t *)miduart2.prxdata;
        }   

        miduart2.rxlen = UART_Data_Rec(&huart2,&ig_uart2tick,200);
    }   
    else if(port == uart3)
    {
        if((miduart3.State == UART_STATE_CLOSED) || (miduart3.State == UART_STATE_ERROR))
        {
            return NULL;
        }
        miduart3.State = UART_STATE_BUSY_RX;        
        if(HAL_UART_Receive_IT(&huart3,miduart3.prxdata, 100) == HAL_OK ) // 最多可接受100个数据
        {
            miduart3.State = UART_STATE_OPEN_IDLE; 
            ig_uart3tick = 0;
            *length = miduart3.rxlen;
            miduart3.rxlen = 0;
            return (uint8_t *)miduart3.prxdata;
        }   

        miduart3.rxlen = UART_Data_Rec(&huart3,&ig_uart3tick,100);
    }
    else
    {
        
    }  
    return NULL;
}

/*******************************************************************************
* 名    称：UART_Data_Rec
* 功    能：使用中断串口接受数据，根据串口接受时间判断接受完成
* 参数说明：huart ------ 串口句柄
         uarttick ------ 串口接受数据时间，在串口接受中断触发时会更新时间
          Timeout ------ 判断串口接受完成超时时间，如果串口接受数据和系统
                         tick的差值超过该值表示一次数据接受完成。
* 返 回 值：无
* 说    明：无
* 调用方法：需要配合函数HAL_UART_Receive_IT一起使用，先调用
            HAL_UART_Receive_IT函数，开始串口中断接受数据。
**********************************************************/ 
uint16_t  UART_Data_Rec(UART_HandleTypeDef *huart, uint32_t * uarttick,uint32_t Timeout)
{
    uint16_t RecDataLen = 0;     
   
    RecDataLen = huart->RxXferSize - huart->RxXferCount; // 接受到的字节数    
    
    if(RecDataLen > 0) // 表示串口开始接受数据，即开始判断串口时间是否超时
    {
       /* 如果串口时间和系统tick的差值大于设定值，则判断接受数据完成，需要清除相关寄存器 */
      if((HAL_GetTick() - *uarttick) > Timeout) // 
      {
          /* Disable the UART Parity Error Interrupt and RXNE interrupts */
    #if defined(USART_CR1_FIFOEN)
          CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE));
    #else
          CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
    #endif

          /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
          CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

          /* Rx process is completed, restore huart->RxState to Ready */
          huart->RxState = HAL_UART_STATE_READY;

          /* Clear RxISR function pointer */
          huart->RxISR = NULL;
      }
    }
    return RecDataLen;
}

/*******************************************************************************
* 名    称：Uart_Init
* 功    能：串口初始化
* 参数说明：port：串口号
			BaudRate：波特率
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void Uart_Init(UART_port port,uint32_t BaudRate)
{
    if((port > UART_NUM)||(BaudRate> MAX_BAUDRATE ))
    {

        return ;
    }
    
    if(port == lpuart1)
    {
        MX_LPUART1_UART_Init(BaudRate);
        midlpuart1.State = UART_STATE_OPEN_IDLE;
    }
    else if(port == uart1)
    {
        MX_USART1_UART_Init(BaudRate);    
        miduart1.State = UART_STATE_OPEN_IDLE;
    }
    else if(port == uart2)
    {
        MX_USART2_UART_Init(BaudRate);  
        miduart2.State = UART_STATE_OPEN_IDLE;        
    }
    else if(port == uart3)
    {
        MX_USART3_UART_Init(BaudRate);    
        miduart3.State = UART_STATE_OPEN_IDLE;
    }
    else
    {

    }        
}

/* LPUART1 init function */

void MX_LPUART1_UART_Init(uint32_t BaudRate)
{
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = BaudRate;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    uart_error_handle();
  }
}
/* USART1 init function */

void MX_USART1_UART_Init(uint32_t BaudRate)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = BaudRate;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    uart_error_handle();
  }

}
/* USART2 init function */

void MX_USART2_UART_Init(uint32_t BaudRate)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = BaudRate;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    uart_error_handle();
  }

}
/* USART3 init function */

void MX_USART3_UART_Init(uint32_t BaudRate)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = BaudRate;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    uart_error_handle();
  }

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==LPUART1)
  {
  /* USER CODE BEGIN LPUART1_MspInit 0 */

  /* USER CODE END LPUART1_MspInit 0 */
    /* LPUART1 clock enable */
    __HAL_RCC_LPUART1_CLK_ENABLE();
  
    if(uart_flag == 0)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }

    if(uart_flag == 0)
    {
        GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    else
    {
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    }

    /* LPUART1 interrupt Init */
    HAL_NVIC_SetPriority(LPUART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(LPUART1_IRQn);
  /* USER CODE BEGIN LPUART1_MspInit 1 */

  /* USER CODE END LPUART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();
  
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==LPUART1)
  {
  /* USER CODE BEGIN LPUART1_MspDeInit 0 */

  /* USER CODE END LPUART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPUART1_CLK_DISABLE();
  

    if(uart_flag == 0)
    {
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
        
    }
    else
    {
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0 | GPIO_PIN_1);
        
    }

    /* LPUART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(LPUART1_IRQn);
  /* USER CODE BEGIN LPUART1_MspDeInit 1 */

  /* USER CODE END LPUART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();
  

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();
  

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();
  

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_4|GPIO_PIN_5);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
