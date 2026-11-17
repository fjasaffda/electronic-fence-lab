/*******************************************************************************
* @文件名        : main
* @描述          : 环境监测场景（大气压、温湿度、光照 + 云端控制）
* @作者          : 物联网项目组
* @日期          : 2024/05/01
* @版本          : V1.0.0
--------------------------------------------------------------------------------
功能介绍：
1、I2C2 总线读取 CPS121 大气压、SHT30 温湿度、BH1750 光照强度
2、数据通过 ESP8285 模组上报华为云 IoT 平台（农业场景协议）
3、云端可下发指令控制灯（PB15，PWM 调光）和风扇（PA8）
*******************************************************************************/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "task_inter.h"
#include "sysclk.h"
/* Private function prototypes -----------------------------------------------*/
#include "control_task.h"
#include "view_task.h"
#include "env_task.h"
/*******************************************************************************
* 名    称： main(void)
* 功    能：主函数
* 参数说明：无
* 返 回 值：uwRet：LOS_OK 表示正常，LOS_NOK 表示异常
* 说    明：无
* 调用方法：内部函数
*******************************************************************************/
int main(void)
{
    UINT32 uwRet = LOS_OK;
    /*初始化*/
    HAL_Init();                    //硬件初始化
    sysclkinit();                  //系统时钟初始化
    task_view_init();              //显示部分初始化
    task_control_init();           //控制部分初始化

    uwRet = LOS_KernelInit();      //操作系统初始化

    if(uwRet != LOS_OK)
    {
        return LOS_NOK;
    }
    uwRet = creat_control_task();    //创建控制任务
    if(uwRet != LOS_OK)
    {
        return LOS_NOK;
    }
    uwRet = creat_view_task();       //创建显示任务
    if(uwRet != LOS_OK)
    {
        return LOS_NOK;
    }
    uwRet = creat_env_task();        //创建环境采集任务
    if(uwRet != LOS_OK)
    {
        return LOS_NOK;
    }
    LOS_Start();	     //启动操作系统
    while (1)
    {

    }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
