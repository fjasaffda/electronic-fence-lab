#ifndef __CONTROL_TASK_H
#define __CONTROL_TASK_H
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

#define BZERO(a)  (memset(&a, 0, sizeof(a)))


#define DEBUG_UART uart2

UINT32 creat_control_task(VOID);
void read_uart_data(void);
#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */
