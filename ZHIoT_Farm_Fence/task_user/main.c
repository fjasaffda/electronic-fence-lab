/*******************************************************************************
* @文件名        : main
* @描述          : NBV2.0主板涵盖功能测试
* @作者          : 物联网项目组
* @日期          : 2023/04/01
* @版本          : V2.0.0
--------------------------------------------------------------------------------

*******************************************************************************/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "task_inter.h"
#include "sysclk.h"
/* Private function prototypes -----------------------------------------------*/
#include "control_task.h"
#include "view_task.h"
#include "dyp_r01.h"
extern UINT32 creat_fence_task(VOID);
/*******************************************************************************
* 名    称： main(void)
* 功    能：主函数
* 参数说明：无
* 返 回 值：uwRet：LOS_OK 表示正常；LOS_NOK 表示异常
* 说    明：无
* 调用方法：内部函数
*******************************************************************************/
int main(void)
{
    UINT32 uwRet = LOS_OK;
    /*初始化*/
    HAL_Init();                    //硬件初始化
    sysclkinit();                  //系统时钟初始化
    task_view_init();              //显示任务初始化
    task_control_init();           //控制任务初始化

    uwRet = LOS_KernelInit();      //操作系统初始化

    if (uwRet != LOS_OK)
    {
        return LOS_NOK;
    }
    uwRet = creat_control_task();    //创建控制任务
    if (uwRet != LOS_OK)
    {
        return LOS_NOK;
    }
    uwRet = creat_view_task();       //创建显示任务
    if (uwRet != LOS_OK)
    {
        return LOS_NOK;
    }
    uwRet = creat_fence_task();       //创建围栏任务
    if (uwRet != LOS_OK)
    {
        return LOS_NOK;
    }
    LOS_Start();	     //启动操作系统
    while (1)
    {

    }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
