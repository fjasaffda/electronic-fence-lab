#ifndef __NET_H
#define __NET_H

#include "usart_api.h"

#define NET_TYPE_NB_IOT 0
#define NET_TYPE_WIFI 1


#define BC35G_UART_PORT uart3
#define ESP8285_UART_PORT uart1

#define NET_STR_LEN 20
typedef enum 
{
     Net_Starting=0,
     Net_Setting,  
     Net_Setted_over,
     Net_Connected,           
     Net_Drop                 
}NetState_Value;


typedef struct
{
	int type;
	NetState_Value  state;
	UART_port uart_port;
	char ID[NET_STR_LEN];
	char line1[NET_STR_LEN];
	char line2[NET_STR_LEN];
	char line3[NET_STR_LEN];
	
}NET_PACKAGE;




    
/* 声明提供外部调用结构体 */
extern char qrcodeflag;
/* 声明提供外部调用函数 */
extern void Net_Module_Init(int type);
extern void Net_Module_Handle(void);
extern void Net_Module_Time_Deal(void);
extern void Net_Module_Write_Data(char* data,int len);
extern void Net_Module_Read_Data(char* data,int len);
extern NetState_Value Get_Net_Module_State(void);
extern int Is_Net_Module_Connected(void);
extern UART_port Get_Net_Module_Uart_Port(void);

extern char* Get_Net_Module_ID(void);

extern char* Get_Net_Module_UI_Line1(void);
extern char* Get_Net_Module_UI_Line2(void);
extern char* Get_Net_Module_UI_Line3(void);




#endif
