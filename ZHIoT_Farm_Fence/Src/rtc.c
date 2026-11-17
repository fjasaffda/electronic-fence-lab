/**
  ******************************************************************************
  * File Name          : RTC.c
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"
#include "rtc_api.h"
#include "stdio.h"
#include "string.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
RTC_HandleTypeDef hrtc;
union RTC_time_struct rtc_time;

__weak void rtc_error_handler(void);

void rtc_open(void)
{
    MX_RTC_Init();
}
void rtc_close(void)
{
    HAL_RTC_MspDeInit(&hrtc);
}
void rtc_settime(char *ptimebuffer)//"2019/4/7,18:25:06"
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};    
    union RTC_time_struct timeset;
  
    memcpy(&timeset.time_buffer,ptimebuffer,20);
    if((timeset.time_buffer.slash1 != '/')||(timeset.time_buffer.comma != ',')\
        ||(timeset.time_buffer.colon1 != ':'))
    return ;
    
  sTime.Hours = ((timeset.time_buffer.hour[0]-0x30)<<4)+timeset.time_buffer.hour[1]-0x30;
  sTime.Minutes = ((timeset.time_buffer.minute[0]-0x30)<<4)+timeset.time_buffer.minute[1]-0x30;
  sTime.Seconds = ((timeset.time_buffer.second[0]-0x30)<<4)+timeset.time_buffer.second[1]-0x30;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    rtc_error_handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
  sDate.Month = ((timeset.time_buffer.month[0]-0x30)<<4)+timeset.time_buffer.month[1]-0x30;
  sDate.Date = ((timeset.time_buffer.day[0]-0x30)<<4)+timeset.time_buffer.day[1]-0x30;
  sDate.Year = ((timeset.time_buffer.year[2]-0x30)<<4)+timeset.time_buffer.year[3]-0x30;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    rtc_error_handler();
  }
}
void rtc_gettime(char *ptimebuffer)
{
    
      RTC_DateTypeDef sdatestructure;
      RTC_TimeTypeDef stimestructure;
    
      /* Get the RTC current Time ,must get time first*/
      HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
      /* Get the RTC current Date */
      HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
 
      /* Display date Format : yy/mm/dd */
      sprintf(rtc_time.timestr,"%02d/%02d/%02d,%02d:%02d:%02d",
    2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date
    ,stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds);
    for(uint8_t i=0;i<20;i++)
    *(ptimebuffer+i) = rtc_time.timestr[i];
}
/* RTC init function */
void MX_RTC_Init(void)
{


  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    rtc_error_handler();
  }

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
