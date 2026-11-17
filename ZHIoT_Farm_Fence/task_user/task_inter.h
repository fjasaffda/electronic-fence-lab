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

typedef struct
{
    uint8_t state;
    uint8_t res[3];
    uint8_t ID[20];
	uint8_t Line1[20];
	uint8_t Line2[20];
	uint8_t Line3[20];
    uint8_t InfInput;
    uint8_t FireInput;
    uint8_t ShockInput;
    uint8_t EnclosureInput;  
    uint8_t Brightness[6];// max= 65535
	uint8_t res2[2];
    uint8_t Distance[6];//"0.0~50.0"
	uint8_t res3[2];

    uint8_t clr_screen_status;
	uint8_t res4[3];
} ViewDataStruct;

extern ViewDataStruct FarmView;
#endif

