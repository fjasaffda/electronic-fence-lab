/*-----------------------------------------------------------------------------
* @文件名        : env_task
* @描述          : 环境数据采集与云端上报任务
* @作者          : 物联网项目组
* @日期          : 2024/05/01
* @版本          : V1.0.0
******************************************************************************/

#ifndef __ENV_TASK_H
#define __ENV_TASK_H
#ifdef __cplusplus
extern "C" {
#endif

/* Includes LiteOS------------------------------------------------------------------*/
#include "los_base.h"
#include "los_config.h"
#include "los_sys.h"
#include "los_typedef.h"
#include "los_task.ph"
#include "los_hwi.h"
#include "los_sem.h"
#include "los_event.h"
#include "los_memory.h"
#include "los_queue.ph"

UINT32 creat_env_task(VOID);

#ifdef __cplusplus
}
#endif
#endif
