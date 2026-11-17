#ifndef BC35G_H
#define BC35G_H

#include "stdint.h"
/*
商用：117.78.42.93
培训；49.4.85.232
旧：139.159.140.34
*/
#define OC_IP                   "117.78.42.93" 
#define OC_IP_PORT              "5683"

#define UE_UART_CHECK           "AT\r\n"
#define AT_GET_SIM_CIMI         "AT+CIMI\r\n"
#define AT_GET_SIM_CPIN         "AT+CPIN?\r\n"
#define AT_CMD_ENABLE_PRINT     "AT+CMEE=1\r\n"
#define AT_CMD_ENABLE_REMIND    "AT+NNMI=1\r\n"
#define GET_UE_IMEI             "AT+CGSN=1\r\n"
#define GET_UE_CSQ              "AT+CSQ\r\n"  
#define RESET_NB              		"AT+NRB\r\n"  
#define CFUN                    "AT+CFUN=1\r\n"
#define GET_UE_CCLK             "AT+CCLK?\r\n"
#define CGATT_ATTACH_SEARCH     "AT+CGATT?\r\n"
#define CGATT_ATTACH            "AT+CGATT=1\r\n"
#define AT_CMD_ENABLE_NET_SEARCH "AT+CEREG?\r\n"
#define AT_CMD_ENABLE_NET       "AT+CEREG=1\r\n"
#define GET_UE_STATE            "AT+NUESTATS\r\n"
#define GET_UE_IP               "AT+CGPADDR\r\n"
#define GET_UE_NET_STATE        "AT+NMSTATUS?\r\n"
#define OPEN_AUTOCONNECT 		"AT+NCONFIG=AUTOCONNECT,TRUE\r\n"
/* BC35G模块状态 */
typedef enum 
{
     BC35G_Starting=0,
     BC35G_Setting,             
     BC35G_Setted_over ,          
     BC35G_Connected,           
     BC35G_Drop                 
}BC35GState_Value;
/* BC35G模块上电状态 */
typedef enum 
{
     POWER_OFF=0,
     POWER_ING,             
     POWER_ON ,                       
}BC35GPower_Value;


/* BC35G发生错误后执行功能 */
#define  RestartPWR                  1


/*BC35G串口状态*/
#define BC35UART_REC_DEIL           0
#define BC35UART_REC_DOING          0x01U
#define BC35UART_REC_OVER           0x02U

#define  NET_LED_ON       led_open(LED2_PORT)
#define  NET_LED_OFF      led_close(LED2_PORT)  
#define  UartSend(data)   uart_write(uart3,sizeof(data)-1,(unsigned char *)data);
#define  bc35delay(x)     HAL_Delay(x)

/* BC35G错误结构体 */
typedef  struct 
{
     unsigned char   ErrComn;               /* 错误指令 */
     unsigned char   ErrFunc;               /* 发生错误后需要执行的功能操作码 */
     unsigned int    ErrFlag;               /* 错误标志位 */
     unsigned char   ATComnbuf[50];         /* 发生错误的AT指令字符 */
}BC35G_Err_PACKAGE;

/* BC35G数据结构体 */
typedef  struct 
{
     BC35GState_Value   State;                  /* BC35G模块状态 */
	 unsigned char   set_state;              /* BC35G的设置状态 */
	 unsigned char   set_num;                /* BC35G的设置状态编号 */
     BC35GPower_Value   power_state;            /* BC35G的电源状态 */
     unsigned int    set_time;               /* BC35G设置时间参数 */
	 unsigned int    feed_time;	             /* BC35G喂狗时间参数，当feed_time参数为0时，则表示模块异常*/
     unsigned char   First_Connected_Flag;   /* BC35G首次成功联网标志 */
	 unsigned int    UECSQ;                 /* BC35G模块信号 范围0~31，31表示信号极好 */
     unsigned char   UEIMEI[16];
     unsigned char   UECCLK[30];
     unsigned char   UESignal[10];              //"-328.3" dBm 范围：
     unsigned char   UESnr[7];          //"3872" dB  范围：
     unsigned char   UEPCI[7];          //"32323"  范围：    
	 unsigned char   net_state[3];
	 unsigned char   connect_state[200];
}BC35G_PACKAGE;

typedef  struct 
{
    unsigned char ucRecState;
    char RecInfo[400];
    
}BC35G_UART_PACKAGE;
    
/* 声明提供外部调用结构体 */
extern BC35G_PACKAGE _BC35G;
extern BC35G_UART_PACKAGE stUart_BC35G;
extern BC35G_Err_PACKAGE BC35G_Err;
/* 声明提供外部调用函数 */
extern void BC35G_Write_Data(char* data,int len);
extern void BC35G_Read_Data(char* data,int len);
extern void BC35G_Handle(void);
extern void BC35G_Time_Deal(void);
extern void BC35_Init(void);
extern BC35GState_Value Get_BC35G_State(void);
extern int Is_BC35G_Connected(void);

extern void BC35_Finger_Get_Time(void);
extern char qrcodeflag;

void _BC35G_Get_Time(void);

#endif
