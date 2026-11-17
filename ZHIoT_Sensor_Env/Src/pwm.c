/*******************************************************************************
* @文件名        : pwm
* @描述          : 电机驱动配置相关函数
* @作者          : 物联网项目组
* @日期          : 2023/04/01
* @版本          : V2.0.0
--------------------------------------------------------------------------------
  本驱动函数使用指导流程：
  1、添加本驱动的.c和.h文件
  2、在user.c中调用PWM_Init函数进行GPIO初始化
  3、通过修改PWM_Handle函数的入口参数来修改PWM占空比，参数范围0~100
  4、用户要根据程序系统时钟修改SYSCLOCK的值，要不会影响PWM的周期
  5、电机的驱动为的PWM为10KHZ,要注意修改PERIOD_VALUE和SYSCLOCK的值
  6、修改PERIOD_VALUE值时，最好为大于等于100的倍数，这样会更加精确
*******************************************************************************/


#include "pwm.h"
#include "pwm_api.h"

/*****************************函数内部宏定义定义*******************************/
/* 系统时钟，用于延时,需要用户根据系统时钟进行修改，单位Mhz */
#define SYSCLOCK                    (SystemCoreClock/1000000)  
/* 预频器值  即 SYSCLOCK / (PRESCALER_VALUE + 1) = 1M */
#define PRESCALER_VALUE             (SYSCLOCK - 1)      
/* PWM周期参数， PWM fclk= 1M / (49 + 1) = 20Khz */
#define PERIOD_VALUE                 3999                       

/*****************************函数内部结构体定义*******************************/
TIM_HandleTypeDef htim15;   

/*****************************函数内部定义*************************************/
static void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle); 
static void MX_TIM15_Init(void);

/*******************************************************************************
* 名    称：pwm_open
* 功    能：开启PWM输出
* 参数说明：duty：占空比
* 返 回 值：无
* 说    明：定时器15通道2
* 调用方法：内部调用
*******************************************************************************/
void pwm_open(uint8_t duty)
{
    MX_TIM15_Init();

    pwm_set(duty);
    HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_2);
}

/*******************************************************************************
* 名    称：pwm_close
* 功    能：关闭PWM输出
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
void pwm_close(void)
{
    HAL_TIM_PWM_MspDeInit(&htim15);
}

/*******************************************************************************
* 名    称：pwm_set
* 功    能：设置pwm占空比
* 参数说明：duty：占空比
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
void pwm_set(uint8_t duty)
{
    if(duty >=100)
    {
        duty=100;
    }

    if(duty == 0)
    {
        TIM15->CCR2 = (uint32_t)(0.85 * duty * (PERIOD_VALUE + 1))/100; 
    }
    else
    {
        TIM15->CCR2 = (uint32_t)(0.85 * duty * (PERIOD_VALUE + 1))/100 + 600;     
    }
}

/************************************************************************
* 名    称：HAL_TIM_MspPostInit
* 功    能：硬件IO初始化
* 参数说明：tim_pwmHandle：定时器句柄类型指针
* 返 回 值：无
* 说    明：配置PWM相应的引脚
* 调用方法：内部调用
*******************************************************************************/
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(timHandle->Instance==TIM15)
  {
  /* USER CODE BEGIN TIM15_MspPostInit 0 */

  /* USER CODE END TIM15_MspPostInit 0 */
  
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_TIM15;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM15_MspPostInit 1 */

  /* USER CODE END TIM15_MspPostInit 1 */
  }

}

/************************************************************************
* 名    称：MX_TIM2_Init
* 功    能：定时器2初始化配置
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部函数
*******************************************************************************/
void MX_TIM15_Init(void)
{

  TIM_OC_InitTypeDef sConfigOC = {0};

  htim15.Instance = TIM15;
  htim15.Init.Prescaler = PRESCALER_VALUE;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = PERIOD_VALUE;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim15) != HAL_OK)
  {

  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 80;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim15, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {

  }

  HAL_TIM_MspPostInit(&htim15);

}

/*************************************************************************
* 名    称：HAL_TIM_PWM_MspInit
* 功    能：基本定时器硬件反初始化设置
* 参数说明：tim_pwmHandle：TIM2结构体
* 返 回 值：无
* 说    明：该函数被HAL库内部调用
* 调用方法：内部函数
*******************************************************************************/
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* tim_pwmHandle)
{

  if(tim_pwmHandle->Instance==TIM15)
  {
  /* USER CODE BEGIN TIM15_MspInit 0 */

  /* USER CODE END TIM15_MspInit 0 */
    /* TIM15 clock enable */
    __HAL_RCC_TIM15_CLK_ENABLE();
  /* USER CODE BEGIN TIM15_MspInit 1 */

  /* USER CODE END TIM15_MspInit 1 */
  }
}

/*************************************************************************
* 名    称：HAL_TIM_PWM_MspDeInit
* 功    能：基本定时器硬件反初始化设置
* 参数说明：tim_pwmHandle：定时器句柄类型指针
* 返 回 值：无
* 说    明：该函数被HAL库内部调用
* 调用方法：内部函数
*******************************************************************************/
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* tim_pwmHandle)
{

  if(tim_pwmHandle->Instance==TIM15)
  {
  /* USER CODE BEGIN TIM15_MspDeInit 0 */

  /* USER CODE END TIM15_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM15_CLK_DISABLE();
  /* USER CODE BEGIN TIM15_MspDeInit 1 */

  /* USER CODE END TIM15_MspDeInit 1 */
  }
} 

