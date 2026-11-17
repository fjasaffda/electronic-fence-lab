#ifndef ESP8285_H
#define ESP8285_H

#include "stdint.h"

#define SERVER_IP                   "121.36.42.100" 
#define SERVER_PORT              	1883

#define WIFI_SSID					"OPPO"
#define WIFI_PSWD					"YOUR_WIFI_PASSWORD"

#define WIFI_STA_MODE				"1"



#define PROD_ID                     "655c46d4f672eb209deac6aa_866636069326324"
#define USER_PSWD                   "YOUR_DEVICE_SECRET"

#define TIMESTAMP                   "2023011215"

#define CLIENT_ID            		(PROD_ID"_0_0_"TIMESTAMP)


#define MQTT_NO_RECONNECT			0	  
#define MQTT_RECONNECT				1	 

#define MQTT_QOS   					0
#define MQTT_SUB_TOPIC              "123"
#define MQTT_PUB_TOPIC              ("$oc/devices/"PROD_ID"/sys/properties/report")

#define MQTT_PUB_RESPONSE_TOPIC     ("$oc/devices/"PROD_ID"/sys/commands/response/request_id=")


#define UART_SEND_MAX_LEN           400
#define MQ_MAX_NUM           		4

#define True        			   	1
#define False						0


/* Wifi模块状态 */
typedef enum 
{
     ESP8285_Starting=0,
     ESP8285_Setting,   
     ESP8285_Setted_over,
     ESP8285_Connected,           
     ESP8285_Drop                 
}ESP8285_State_Value;


/*WIFI串口状态*/
#define WIFIUART_REC_IDLE           0
#define WIFIUART_REC_OVER           0x01U

#define  WIFI_LED_ON       
#define  WIFI_LED_OFF      
#define  ESP8285UartSend(data,len)   uart_write(uart1,len,(unsigned char *)data);




    
/* 声明提供外部调用结构体 */
extern char qrcodeflag;
/* 声明提供外部调用函数 */
extern void ESP8285_Init(void);
extern void ESP8285_Handle(void);
extern void ESP8285_Time_Deal(void);
extern void ESP8285_Write_Data(char* data,int len);
extern void ESP8285_Read_Data(char* data,int len);
extern ESP8285_State_Value Get_ESP8285_State(void);
extern int Is_ESP8285_Connected(void);
extern void Finger_Get_Time(void);
extern void ESP8285_Finger_Get_Time(void);
#endif

