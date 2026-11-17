/*-----------------------------------------------------------------------------
* @文件名        : env_task
* @描述          : 环境数据采集与云端上报任务
* @作者          : 物联网项目组
* @日期          : 2024/05/01
* @版本          : V1.0.0
*******************************************************************************
功能介绍：
1、每 2 秒通过 I2C2 读取一次大气压、温湿度、光照强度、二氧化碳
2、把数据填入云端协议结构体 g_pltSendData，由控制任务发送到云端平台
3、读取结果在串口 2 打印，供调试查看
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "env_task.h"
#include "env_sensor.h"
#include "env_control.h"
#include "ocprotocol.h"
#include "task_inter.h"
#include "usart_api.h"
#include "gpio_api.h"

UINT32 g_EnvTskHandle;

/********************************内部定义**************************************/
#define ENV_SAMPLE_TIME       2000      /* 采样周期，单位 ms */
#define ENV_FIELD_LEN         4         /* 云端协议中字符串字段的有效字符数 */

/********************************************************************************
* 名    称：Env_Fmt_One_Decimal
* 功    能：把浮点数格式化成 1 位小数的字符串（不依赖 printf 的浮点格式）
* 参数说明：out：输出缓冲   size：缓冲长度   val：待转换的数值
* 返 回 值：无
* 说    明：工程使用 MicroLIB，不支持 %f 格式化，所以手工拆分整数和小数
* 调用方法：内部函数
********************************************************************************/
static void Env_Fmt_One_Decimal(char *out, uint16_t size, float val)
{
    int32_t v;
    int32_t a;

    v = (val >= 0.0f) ? (int32_t)(val * 10.0f + 0.5f) : (int32_t)(val * 10.0f - 0.5f);
    a = (v >= 0) ? v : -v;

    snprintf(out, size, "%s%d.%d", (v < 0) ? "-" : "", (int)(a / 10), (int)(a % 10));
}

/********************************************************************************
* 名    称：Env_Field_Update
* 功    能：更新云端协议中的字符串字段
* 参数说明：field：协议字段首地址   size：字段总长度   str：数据字符串
* 返 回 值：无
* 说    明：协议字段长度有限（如 char[5]），只保留前 4 个字符
* 调用方法：内部函数
********************************************************************************/
static void Env_Field_Update(char *field, uint16_t size, const char *str)
{
    if(size == 0)
    {
        return;
    }

    memset(field, 0, size);
    strncpy(field, str, size - 1);
}

/********************************************************************************
* 名    称：Env_Data_To_Protocol
* 功    能：把传感器数据填入云端协议结构体
* 参数说明：无
* 返 回 值：无
* 说    明：温度、湿度保留 1 位小数，气压、光照取整数
* 调用方法：内部函数
********************************************************************************/
static void Env_Data_To_Protocol(void)
{
    char buf[16];
    float temperature;
    float humidity;

    if(g_env_data.sht30_ok)
    {
        /* 协议字段只有 4 个字符，做上下限保护，避免出现 "100." 这类截断值 */
        temperature = g_env_data.temperature;
        humidity = g_env_data.humidity;

        if(temperature > 99.9f)
        {
            temperature = 99.9f;
        }
        if(temperature < -40.0f)
        {
            temperature = -40.0f;
        }
        if(humidity > 99.9f)
        {
            humidity = 99.9f;
        }
        if(humidity < 0.0f)
        {
            humidity = 0.0f;
        }

        Env_Fmt_One_Decimal(buf, sizeof(buf), temperature);
        Env_Field_Update(g_pltSendData.Temperature, sizeof(g_pltSendData.Temperature), buf);

        Env_Fmt_One_Decimal(buf, sizeof(buf), humidity);
        Env_Field_Update(g_pltSendData.Humidity, sizeof(g_pltSendData.Humidity), buf);
    }

    if(g_env_data.cps121_ok)
    {
        snprintf(buf, sizeof(buf), "%d", (int)(g_env_data.pressure + 0.5f));
        Env_Field_Update(g_pltSendData.Pressure, sizeof(g_pltSendData.Pressure), buf);
    }

    if(g_env_data.bh1750_ok)
    {
        uint16_t lux = g_env_data.light;

        if(lux > 9999)              /* 协议字段只有 4 个字符，超出范围按 9999 上报 */
        {
            lux = 9999;
        }

        snprintf(buf, sizeof(buf), "%d", (int)lux);
        Env_Field_Update(g_pltSendData.Light, sizeof(g_pltSendData.Light), buf);
    }

    if(g_env_data.sgp30_ok)
    {
        uint16_t co2 = g_env_data.co2;

        if(co2 > 9999)              /* 协议字段只有 4 个字符，超出范围按 9999 上报 */
        {
            co2 = 9999;
        }

        snprintf(buf, sizeof(buf), "%d", (int)co2);
        Env_Field_Update(g_pltSendData.Carbon_Dioxide, sizeof(g_pltSendData.Carbon_Dioxide), buf);
    }

    /* 设备输出状态一并上报，云端可以确认控制指令是否生效 */
    g_pltSendData.dOutput_Light_Switch = Env_Lamp_Get_State();
    g_pltSendData.dOutput_Light_Level  = Env_Lamp_Get_State() ? Env_Lamp_Get_Level() : 0;
    g_pltSendData.dOutput_Irrigation   = Env_Fan_Get_State();
    g_pltSendData.dInput_Enclosure     = 0;
}

/********************************************************************************
* 名    称：Env_Debug_Print
* 功    能：串口打印本次采集结果
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部函数
********************************************************************************/
static void Env_Debug_Print(void)
{
    int32_t t10, h10, a_t, a_h;

    if(g_env_data.sht30_ok)
    {
        t10 = (int32_t)(g_env_data.temperature * 10.0f);
        h10 = (int32_t)(g_env_data.humidity * 10.0f);
        a_t = (t10 >= 0) ? t10 : -t10;
        a_h = (h10 >= 0) ? h10 : -h10;
        printf("** 温度:%s%d.%d C  湿度:%d.%d %%RH\r\n",
               (t10 < 0) ? "-" : "", (int)(a_t / 10), (int)(a_t % 10),
               (int)(a_h / 10), (int)(a_h % 10));
    }
    else
    {
        printf("** 温湿度传感器读取失败\r\n");
    }

    if(g_env_data.cps121_ok)
    {
        printf("** 大气压:%d hPa\r\n", (int)(g_env_data.pressure + 0.5f));
    }
    else
    {
        printf("** 大气压传感器读取失败\r\n");
    }

    if(g_env_data.bh1750_ok)
    {
        printf("** 光照强度:%d lx\r\n", (int)g_env_data.light);
    }
    else
    {
        printf("** 光照传感器读取失败\r\n");
    }

    if(g_env_data.sgp30_ok)
    {
        printf("** 二氧化碳:%d ppm  TVOC:%d ppb\r\n", (int)g_env_data.co2, (int)g_env_data.tvoc);
    }
    else
    {
        printf("** 二氧化碳传感器读取失败\r\n");
    }

    printf("** 灯:%s(%d)  风扇:%s\r\n\r\n",
           Env_Lamp_Get_State() ? "开" : "关", (int)Env_Lamp_Get_Level(),
           Env_Fan_Get_State() ? "开" : "关");
}

/********************************************************************************
* 名    称： env_task
* 功    能：环境数据采集任务
* 参数说明：无
* 返 回 值：无
* 说    明：周期读取传感器，更新云端上报数据
* 调用方法：内部函数
********************************************************************************/
VOID env_task(VOID)
{
    UINT32 uwRet = LOS_OK;
    uint32_t tick = 0;

    Env_Sensor_Init();

    while(1)
    {
        if((HAL_GetTick() - tick) >= ENV_SAMPLE_TIME)
        {
            tick = HAL_GetTick();

            Env_Sensor_Read_All();
            Env_Data_To_Protocol();
            Env_Debug_Print();

            /* 数据有变化，通知控制任务立即上报云端 */
            Scene.event_sendflag = 1;
        }

        uwRet = LOS_TaskDelay(20);
        if(uwRet != LOS_OK)
        {
            return;
        }
    }
}

/********************************************************************************
* 名    称：creat_env_task
* 功    能：创建环境数据采集任务
* 参数说明：无
* 返 回 值：LOS_OK 表示创建成功，其它表示失败
* 说    明：外部调用
* 调用方法：在 main 函数中调用
********************************************************************************/
UINT32 creat_env_task(VOID)
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 2;                 /* 任务优先级 */
    task_init_param.pcName = "env_task";            /* 任务名称 */
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)env_task;
    task_init_param.uwStackSize = 0xA00;            /* 任务堆栈大小 */
    uwRet = LOS_TaskCreate(&g_EnvTskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }

    return uwRet;
}
