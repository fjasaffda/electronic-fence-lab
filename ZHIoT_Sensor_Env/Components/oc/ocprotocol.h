#ifndef OCPROTOCOL_H
#define OCPROTOCOL_H

#include "oc_config.h"  //后面需要优化为配置头文件
#include "stdint.h"
/* 发送任务宏定义 */
#define SendTask0       0
#define SendTask1       1
#define SendTask2       2
#define SendTask3       3

/**/
#define PTL_SEND_MAX_NUM     1024
#define PTL_REC_MAX_NUM      1024

/*发送任务状态*/
#define SendTack_Wait                 0
#define SendTack_Doing                1
#define SendTack_Error                2


/* 通讯协议发送任务结构体 */

typedef  struct ptl_Tack_PACKAGE
{
    unsigned char     TackName[5];    /* 发送任务的队列 */
    unsigned char     CurTack;        /* 当前发送的任务名 */
    unsigned char     TackNo;         /* 发送任务的总数 */
    unsigned char     TackState;	   /* 发送任务的状态 */
    unsigned int      TackTime;	   /* 发送任务的时间 */
    unsigned char     TimeOutCnt;     /* 单次任务发送超时计数 */
    unsigned char     CheckTime;      /* 连接自检时间 */
} PTL_SendTack_PACKAGE;

/* OC通讯协议：包含三个场景协议 */
typedef struct
{
#if ENABLE_HOME
    unsigned char Type;                 /* 场景类型定义，家居场景：0x02 */
    char Ver[3];	                    /* 固件版本号 */
    unsigned char dOutput_Curtains;      /* 窗帘电机输出数据 开 0x01 关：0x00*/
    unsigned char dInput_Gas;           /* 烟雾状态输入检测数据 */
    unsigned char dInput_Body;	        /* 人体状态输入检测数据 */
    unsigned char dOutput_EV_State;     /* 继电器输出状态 */
    unsigned char dOutput_LED_State;    /* LED输出状态 */
	unsigned char dOutput_Fan_State;    /* 风扇电机输出状态 */
	unsigned char dOutput_Fan_Level;    /* 风扇档位状态 */
	unsigned char dOutput_Servo_State;	/* 舵机输出状态 */
	unsigned char dInput_Servo_Red;		/* 舵机红外状态 */
	char Light[5];	            		/* 5位光照数据 */
    char Temperature[5];	            /* 5位温度数据 */
    unsigned char err_code;	        	/* 错误上报 */
#endif
#if ENABLE_INDU
    unsigned char Type;                 			/* 场景类型定义，工业场景：0x00 */
    char Ver[3];	                    			/* 固件版本号 */
    unsigned char I_Bool_Input;           			/* 光电输入状态 */
    unsigned char I_Bool_OutPut_switch;     		/* 继电器输出状态 */
    unsigned char I_Motor_slide;	        		/* 电机PWM占空比 */
    char I_Temperature[5];		        			/* 5位温度数据 */	
    char I_RFID_ID[10];                      		/* 10位RFID卡号 */
	char I_punch_the_clock_time[20];                /* 20位打卡时间 */
	char I_punch_the_clock_name[10];                /* 10位打卡人姓名 */
	unsigned char I_warehouse_gate_OutPut_switch;   /* 仓库闸机输出状态 */
    char I_Air_switch;                              /* 空调状态 */
    unsigned char err_code;	        			/* 异常上报 */
#endif
#if ENABLE_TRAN
    unsigned char Type;                 /* 场景类型定义，交通场景：0x04 */
    char Ver[3];		                /* 固件版本号 */
    unsigned char dInput_Inf;         	/* 红外输入检测数据 */
    unsigned char dInput_Car;	        /* 车位输入检测数据 */
    unsigned char dOutput_SLight_State;	/* 路灯输出检测数据*/
    unsigned char dOutput_Gate;		    /* 道闸检测数据 */
    unsigned char dInput_Red_Light;		/* 闯红灯检测数据 */
    char Light[5];
    unsigned char err_code;	        	/* 错误上报 */
#endif
#if ENABLE_FARM
	unsigned char Type; 				/* 场景类型定义，农业场景：0x06 */
	char Ver[3];						/* 固件版本号 */
	char Temperature[5];				/* 温度 */
	char Humidity[5];					/* 湿度 */
	unsigned char dOutput_Irrigation;	/* 灌溉电机输出数据 */
	unsigned char dOutput_Light_Switch;	/* 补光灯输出数据 */
	unsigned char dOutput_Light_Level; 	/* 补光灯输出档位*/
	unsigned char dInput_Enclosure; 	/* 电子围栏 */
	char Carbon_Dioxide[5]; 	/* 二氧化碳数据 */
	char Pressure[5]; 	/* 二氧化碳数据 */
	char Light[5]; 	/* 二氧化碳数据 */
	unsigned char err_code; 			/* 错误上报 */
#endif


} PTL_DATA_STRUCT;

typedef struct
{
    //下面两个状态用来解决多线程中出现抢占资源的影响
    unsigned char state; //状态指示，1表示该结构体中的状态变话，0表示为变化；
    unsigned char    req_state; //状态应对指示，1表示对state的应答成功，0表示没接收到state 的1状态或者还没应答；
    unsigned char event_sendflag;
} StatusStruct;
/* 外部调用参数 */
extern PTL_SendTack_PACKAGE g_ptl_SendTack;
extern PTL_DATA_STRUCT g_pltSendData;

extern unsigned char g_ptlSendBuf[50];
extern unsigned char g_ptlSendLen;
extern unsigned char g_ptlRecBuf[100];
extern unsigned char g_ptlRecdLen;
extern StatusStruct Scene;
/* 外部调用函数 */
extern void ptl_Handle(void);
extern void ptl_SendSuccess_Deal(PTL_SendTack_PACKAGE* sendtask);
extern void ptl_Heartbeat_Handle(void);
extern void ptl_SendTack_Init(PTL_SendTack_PACKAGE* sendtask);
extern void ptl_Receive_OCData_Handle(char* data);
extern void Task3_Deal(void);
extern void Task2_Deal(void);
extern void Task1_Deal(void);
extern void Task0_Deal(void);
#endif

