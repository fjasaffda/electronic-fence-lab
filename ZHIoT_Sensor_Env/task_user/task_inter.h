#ifndef __task_inter_H
#define __task_inter_H

#include "stdint.h"
#include "ocprotocol.h"

#define NET_TYPE NET_TYPE_WIFI

void task_view_init(void);
void task_control_init(void);

void uart_taskhandle(void);
void control_taskhandle(void);
void task_view(void);

/* 界面显示使用的设备状态结构体 */
typedef struct
{
    uint8_t state;
    uint8_t res[3];
    uint8_t Line1[20];
    uint8_t Line2[20];
    uint8_t Line3[20];
    uint8_t clr_screen_status;
    uint8_t res2[3];
} ViewDataStruct;

extern ViewDataStruct EnvView;
#endif
