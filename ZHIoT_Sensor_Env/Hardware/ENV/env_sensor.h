/*-----------------------------------------------------------------------------
* @文件名        : env_sensor
* @描述          : 大气压、温湿度、光照强度、二氧化碳检测驱动（I2C2 总线）
* @作者          : 物联网项目组
* @日期          : 2024/05/01
* @版本          : V1.0.0
*******************************************************************************
功能介绍：
1、CPS121 大气压传感器  I2C 地址 0x6C
2、SHT30  温湿度传感器  I2C 地址 0x44
3、BH1750 光照强度传感器 I2C 地址 0x23
4、SGP30  二氧化碳传感器 I2C 地址 0x58
四个传感器共用 I2C2 总线：SCL = PB13，SDA = PB14

使用流程：
1、main.c 中调用 Env_Sensor_Init 完成 I2C2 初始化
2、周期性调用 Env_Sensor_Read_All，结果放在 g_env_data 中
3、每路数据的读取结果由 ok 标志指示，0 表示本次读取失败
******************************************************************************/

#ifndef __ENV_SENSOR_H
#define __ENV_SENSOR_H

#include "stm32l4xx_hal.h"

/* 传感器数据 */
typedef struct
{
    float    temperature;      /* 温度，单位 ℃ */
    float    humidity;         /* 湿度，单位 %RH */
    float    pressure;         /* 大气压，单位 hPa */
    uint16_t light;            /* 光照强度，单位 lx */
    uint16_t co2;              /* 二氧化碳，单位 ppm */
    uint16_t tvoc;             /* 总挥发性有机物，单位 ppb */
    uint8_t  sht30_ok;         /* 温湿度读取成功标志 */
    uint8_t  bh1750_ok;        /* 光照读取成功标志 */
    uint8_t  cps121_ok;        /* 气压读取成功标志 */
    uint8_t  sgp30_ok;         /* 二氧化碳读取成功标志 */
} ENV_DATA;

extern ENV_DATA g_env_data;

void Env_Sensor_Init(void);
void Env_Sensor_Read_All(void);

#endif
