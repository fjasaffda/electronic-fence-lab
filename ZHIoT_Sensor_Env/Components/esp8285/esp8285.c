/*******************************************************************************
* @文件名        : esp8285.c
* @描述          : esp8285驱动文件
* @作者          : 物联网项目组
* @日期          : 2022/11/01
* @版本          : V2.0.0
--------------------------------------------------------------------------------
本驱动函数使用指导流程：
1、添加本驱动的.c和.h文件
2、在main.c中的WIFI任务中添加函数 WIFI_Handle(void)
3、在main.c中的sys1ms任务中添加函数 WIFI_Time_Deal(void)
4、完成后网络状态可以通过OLED显示屏和串口查看对应的网络状态
------------------------------------------------------------------------------
关联驱动文件：usart.c     描述：需要usart.c提供串口的发送函数
	   ocprotocol.c     描述：需要对通讯协议相关信息进行处理
*******************************************************************************/

/* User includes */
#include "usart_api.h"
#include "gpio_api.h"
#include "esp8285.h"
#include "ocprotocol.h"
#include "log.h"
/* Standard includes */
#include "stdio.h"
#include "string.h"
#include "HMACSHA.h"
#include "los_task.h"
#include "cJSON.h"
#include "los_memory.h"
#include "net.h"
#include "task_inter.h"

#if NET_TYPE == NET_TYPE_WIFI


#define WHERE_I_AM DEBUG_PRINT(MODULE_WIFI,"[%s,%d]\r\n",__FUNCTION__,__LINE__);
#define STR(x)  x,#x   

#define AT_GET 0
#define AT_SET 1
typedef enum
{
	AT_TCP_PING,
	AT_WIFI_MODE,
	AT_WIFI_STATE,
	AT_WIFI_CONNECT,
	AT_WIFI_STA_IP,
	AT_MQTT_USER_INFO,
	AT_MQTT_CONNECT,
	AT_MQTT_SUBCRIBE,
	AT_MQTT_PUBLISH,
    AT_RST,
    AT_NTP_CFG,
    AT_NTP_TIME,
    AT_MAX_NUM
}WIFI_AT_CMD;

typedef  struct 
{
	 unsigned int	 id;
	 unsigned char*  at_name;             
	 unsigned char*  content;
}AT_TABLE;

typedef  struct 
{
	#define TOPIC_MAX_LEN   128
	#define AT_DATA_MAX_LEN UART_SEND_MAX_LEN
	unsigned char  topic[TOPIC_MAX_LEN];             
	unsigned char  content[AT_DATA_MAX_LEN];
}AT_TRANS_DATA;

//event value
#define START_SET			1<<0
#define OPT_OK				1<<1
#define OPT_ERR				1<<2
#define OPT_BUSY			1<<3
#define OPT_TIMEOUT			1<<4
#define WIFI_COM_OK			1<<5
#define WIFI_COM_ERR		1<<6
#define MQTT_COM_OK			1<<7
#define MQTT_COM_ERR		1<<8
#define MQTT_PUB_ACT		1<<9
#define MQTT_PUB_OK			1<<10
#define MQTT_PUB_FAIL		1<<11
#define WATCH_DOG			1<<12
typedef unsigned long long int EVENT_VALUE; 


typedef enum
{
	STATE_OFFLINE = 0,
	STATE_SET_WIFI_MODE,
	STATE_SET_WIFI_STATE,
	STATE_WIFI_CONNECT,
	STATE_SET_MQTT_INFO,
	STATE_MQTT_CONNECT,
	STATE_CONNECTED_SUB,
	STATE_CONNECTED_IDLE,
	STATE_CONNECTED_PUB,
	STATE_DROP,
    STATE_RST,
    STATE_SET_NTP_CFG,
    STATE_GET_TIME,
	STATE_MAX_NUM
}STATE_VALUE;

typedef struct
{
	EVENT_VALUE event;
	unsigned char content[AT_DATA_MAX_LEN];
}EVENT_PACKAGE;

/* WIFI数据结构体 */
typedef  struct 
{
	STATE_VALUE   	    state;                  /* WIFI模块状态 */
	EVENT_PACKAGE		event_info;
	unsigned int    	set_time;               /* WIFI设置时间参数 */
	unsigned int    	feed_time;	            /* WIFI喂狗时间参数，当feed_time参数为0时，则表示模块异常*/

}WIFI_PACKAGE;

typedef struct
{
	STATE_VALUE state;
	EVENT_VALUE event;
	STATE_VALUE next_state;
	void (*action)(WIFI_PACKAGE *wifi);
}STATE_RELATION;

typedef struct
{
	unsigned int head;
	unsigned int tail;
	EVENT_PACKAGE event_buf[MQ_MAX_NUM];
}EVENT_QUEUE;

/* WIFI数据结构体 */
typedef  struct 
{
    unsigned char ucRecState;
    char RecInfo[400];
    
}WIFI_UART_PACKAGE;

extern char octimebuf[60];
extern char viewtimebuf[60];
extern char time_flag;
char request_id[40] = {0};
/* 定义WIFI结构体 */
static EVENT_QUEUE  _Event_Queue;
WIFI_PACKAGE _WIFI;
WIFI_UART_PACKAGE stUart_WIFI;
static AT_TABLE _WIFI_AT_Table[AT_MAX_NUM] = {	
	{STR(AT_TCP_PING),"AT+PING"},
	{STR(AT_WIFI_MODE),"AT+CWMODE"},
	{STR(AT_WIFI_STATE),"AT+CWSTATE"},
	{STR(AT_WIFI_CONNECT),"AT+CWJAP"},
	{STR(AT_WIFI_STA_IP),"AT+CIPSTA"},
	{STR(AT_MQTT_USER_INFO),"AT+MQTTUSERCFG"},
	{STR(AT_MQTT_CONNECT),"AT+MQTTCONN"},
	{STR(AT_MQTT_SUBCRIBE),"AT+MQTTSUB"},
	{STR(AT_MQTT_PUBLISH),"AT+MQTTPUBRAW"},
    {STR(AT_RST),"AT+RST"},
    {STR(AT_NTP_CFG),"AT+CIPSNTPCFG"},
    {STR(AT_NTP_TIME),"AT+CIPSNTPTIME"},
	};


/* 声明内部使用函数 */
static void _WIFI_Rec_Data_Handle(WIFI_PACKAGE* wifi);
static void _Set_WIFI_Module_Event(EVENT_VALUE	event);
static void _AT_Cmd_Send(WIFI_AT_CMD cmd_id,unsigned char opt,AT_TRANS_DATA *at_data);
static void _WIFI_Set_Mode(WIFI_PACKAGE* wifi);
static void _WIFI_Set_Connect(WIFI_PACKAGE* wifi);
static void _MQTT_Set_User_Info(WIFI_PACKAGE* wifi);
static void _MQTT_Connect(WIFI_PACKAGE* wifi);
static void _MQTT_Connected_Sub(WIFI_PACKAGE* wifi);
static void _MQTT_Connected_Pub(WIFI_PACKAGE* wifi);
static void _Drop_Deal(WIFI_PACKAGE* wifi);
static void _Deal_WIFI_Module_Event(EVENT_VALUE event);
void ESP8285_Finger_Set_Time_Cfg(void);

STATE_RELATION _State_Relation[] = {

{STATE_OFFLINE        ,START_SET   				,STATE_SET_WIFI_MODE    	,_WIFI_Set_Mode},

{STATE_SET_WIFI_MODE  ,OPT_OK   				,STATE_WIFI_CONNECT    	,_WIFI_Set_Connect},
{STATE_SET_WIFI_MODE  ,OPT_ERR|OPT_TIMEOUT  	,STATE_DROP    				,_Drop_Deal},

{STATE_WIFI_CONNECT   ,WIFI_COM_OK|OPT_OK   	,STATE_SET_MQTT_INFO   		,_MQTT_Set_User_Info},
{STATE_WIFI_CONNECT   ,OPT_ERR|OPT_TIMEOUT|WIFI_COM_ERR 		,STATE_DROP ,_Drop_Deal},

{STATE_SET_MQTT_INFO  ,OPT_OK   				,STATE_MQTT_CONNECT    		,_MQTT_Connect},
{STATE_SET_MQTT_INFO  ,OPT_ERR|OPT_TIMEOUT|WIFI_COM_ERR 	,STATE_DROP    	,_Drop_Deal},

{STATE_MQTT_CONNECT   ,MQTT_COM_OK   			,STATE_CONNECTED_SUB    	,_MQTT_Connected_Sub},
{STATE_MQTT_CONNECT   ,OPT_ERR|OPT_TIMEOUT|MQTT_COM_ERR		,STATE_DROP 	,_Drop_Deal},

{STATE_CONNECTED_SUB  ,OPT_OK   				,STATE_CONNECTED_IDLE    	,NULL},
{STATE_CONNECTED_SUB  ,OPT_ERR|OPT_TIMEOUT|WIFI_COM_ERR|MQTT_COM_ERR		,STATE_DROP ,_Drop_Deal},

{STATE_CONNECTED_IDLE ,MQTT_PUB_ACT   			,STATE_CONNECTED_PUB    	,_MQTT_Connected_Pub},
{STATE_CONNECTED_IDLE ,WIFI_COM_ERR|MQTT_COM_ERR,STATE_DROP					,_Drop_Deal},

{STATE_CONNECTED_PUB  ,MQTT_PUB_OK|OPT_OK|MQTT_PUB_ACT   	,STATE_CONNECTED_IDLE    	,NULL},
{STATE_CONNECTED_PUB  ,OPT_ERR|OPT_TIMEOUT|WIFI_COM_ERR|MQTT_COM_ERR,STATE_DROP,_Drop_Deal},

{STATE_DROP           ,START_SET                ,STATE_SET_WIFI_MODE        ,_WIFI_Set_Mode},

};

#define RELATION_MAX_NUM (sizeof(_State_Relation)/sizeof(STATE_RELATION))

static int _Is_WIFI_Module_Connected()
{
	if((_WIFI.state==STATE_CONNECTED_IDLE)||(_WIFI.state == STATE_CONNECTED_PUB))
	{
		return True;
	}
	return False;
}



static void _Event_Queue_Init(EVENT_QUEUE* event_queue)
{
	event_queue->head = 0;
	event_queue->tail = 0;
	memset(event_queue->event_buf,0,sizeof(EVENT_PACKAGE)*MQ_MAX_NUM);
}

static int _Get_Event_Queue_Len(EVENT_QUEUE* event_queue)
{
	return (event_queue->tail + MQ_MAX_NUM - event_queue->head) % MQ_MAX_NUM;
}

static void _Event_Queue_Push(EVENT_QUEUE* event_queue,EVENT_PACKAGE* event)
{
	if(_Get_Event_Queue_Len(event_queue) >= MQ_MAX_NUM)
	{

		return;
	}
	
	memcpy(&event_queue->event_buf[event_queue->tail],event,sizeof(EVENT_PACKAGE));
	event_queue->tail++;
    if(event_queue->tail >= MQ_MAX_NUM)
    {
        event_queue->tail = 0;
    }
    
}

static void _Event_Queue_Pop(EVENT_QUEUE* event_queue,EVENT_PACKAGE* event)
{
	if(_Get_Event_Queue_Len(event_queue) <= 0)
	{

		return;
	}
	
	memcpy(event,&event_queue->event_buf[event_queue->head],sizeof(EVENT_PACKAGE));
	event_queue->head++;
	if(event_queue->head >= MQ_MAX_NUM)
    {
        event_queue->head = 0;
    }
}

void _WIFI_Set_Mode(WIFI_PACKAGE* wifi)
{
	_AT_Cmd_Send(AT_WIFI_MODE,AT_SET,NULL);
}

void _WIFI_Get_Time(WIFI_PACKAGE* wifi)
{
    char data[30] = {0};
	snprintf((char*)data,sizeof(data),"%s?\r\n",_WIFI_AT_Table[AT_NTP_TIME].content);
    ESP8285UartSend(data,strlen((char*)data));
}

void _WIFI_Set_NTP_Cfg(WIFI_PACKAGE* wifi)
{
	_AT_Cmd_Send(AT_NTP_CFG,AT_SET,NULL);
}

void _WIFI_Set_Rst(WIFI_PACKAGE* wifi)
{
	_AT_Cmd_Send(AT_RST,AT_SET,NULL);
}

void _WIFI_Set_Connect(WIFI_PACKAGE* wifi)
{
	_AT_Cmd_Send(AT_WIFI_CONNECT,AT_SET,NULL);
}

void _MQTT_Set_User_Info(WIFI_PACKAGE* wifi)
{
	_AT_Cmd_Send(AT_MQTT_USER_INFO,AT_SET,NULL);
}

void _MQTT_Connect(WIFI_PACKAGE* wifi)
{
	_AT_Cmd_Send(AT_MQTT_CONNECT,AT_SET,NULL);
}

void _MQTT_Connected_Sub(WIFI_PACKAGE* wifi)
{
	WIFI_LED_ON;
	AT_TRANS_DATA trans_data = {0};
	strncpy((char*)trans_data.topic,(char*)MQTT_SUB_TOPIC,sizeof(trans_data.topic));

	_AT_Cmd_Send(AT_MQTT_SUBCRIBE,AT_SET,&trans_data);
}

void _MQTT_Connected_Pub(WIFI_PACKAGE* wifi)
{
	AT_TRANS_DATA trans_data = {0};

	strncpy((char*)trans_data.topic,(char*)MQTT_PUB_TOPIC,sizeof(trans_data.topic));
	strncpy((char*)trans_data.content,(char*)wifi->event_info.content,sizeof(trans_data.content));
	
	_AT_Cmd_Send(AT_MQTT_PUBLISH,AT_SET,&trans_data);
}

void _MQTT_Response(WIFI_PACKAGE* wifi)
{
	AT_TRANS_DATA trans_data = {0};

	strncpy((char*)trans_data.topic,(char*)MQTT_PUB_RESPONSE_TOPIC,sizeof(trans_data.topic));
    strcat((char*)trans_data.topic,request_id);
    

    char net_data[30] = {0};

    cJSON* root =  cJSON_CreateObject();
    
    cJSON_AddNumberToObject(root,"result_code",0);
    
    char* str = cJSON_PrintUnformatted(root);
    snprintf(net_data,sizeof(net_data),"%s",str);

    if(str)
    {
        my_free(str);
    }
    if(root)
    {
        cJSON_Delete(root);
    }


    
    DEBUG_PRINT(MODULE_OC, "%s", net_data);

    
	strncpy((char*)trans_data.content,(char*)net_data,sizeof(trans_data.content));
	
	_AT_Cmd_Send(AT_MQTT_PUBLISH,AT_SET,&trans_data);
}

void _Drop_Deal(WIFI_PACKAGE* wifi)
{
	DEBUG_PRINT(MODULE_WIFI,"%s","_Drop_Deal\r\n");
	WIFI_LED_OFF;
    
    _AT_Cmd_Send(AT_RST,AT_SET,NULL);
    LOS_TaskDelay(500);
    _Set_WIFI_Module_Event(START_SET);
}

void ESP8285_Now_State()
{
	DEBUG_PRINT(MODULE_WIFI,"wifi nowstate: %d \r\n",_WIFI.state);
}
void _Event_Handle(WIFI_PACKAGE* wifi)
{
	int i = 0;
	for(i = 0 ; i < RELATION_MAX_NUM; i++)
	{
		if((_State_Relation[i].state == wifi->state) && (0 != ((_State_Relation[i].event) & (wifi->event_info.event))))
		{
			DEBUG_PRINT(MODULE_WIFI,"wifi nowstate: %d ,nextstate: %d,event: %llu\r\n",wifi->state,_State_Relation[i].next_state,wifi->event_info.event);
			wifi->state = _State_Relation[i].next_state;
			if(_State_Relation[i].action)
			{
				_State_Relation[i].action(wifi);
			}
            break;
		}
	}

	if(RELATION_MAX_NUM == i)
	{
        DEBUG_PRINT(MODULE_WIFI,"err event!!wifi nowstate: %d ,event: %llu\r\n",wifi->state,wifi->event_info.event);
	}
}
static char sha256_buf[1024] = {0};
/********************************************************************************
* 名    称：get_sn_val
* 功    能：根据PASSWORD获取mqtt登录OC的密码
* 参数说明：*buf: 指针
* 返 回 值：无
* 说    明：无
********************************************************************************/
static void get_sn_val(uint8_t *buf )
{

	char key[64], *data, out[64];
	int lenKey, lenData, lenOut;
    
    data = (char*)sha256_buf;
    memset(data,0,1024);
	strcpy(data,USER_PSWD);
	strcpy(key,TIMESTAMP);
	lenKey = strlen(key);
	lenData = strlen(data);

	//HMACSHA256
	lenOut = 32;
	hmac_sha256(key, lenKey, data, lenData, out, &lenOut);

    sprintf((char *)buf,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", \
        out[0],out[1],out[2],out[3],out[4],out[5],out[6],out[7],out[8],out[9],out[10],out[11],out[12],out[13],out[14],out[15], \
        out[16],out[17],out[18],out[19],out[20],out[21],out[22],out[23],out[24],out[25],out[26],out[27],out[28],out[29],out[30],out[31]);


}



void _AT_Cmd_Send(WIFI_AT_CMD cmd_id,unsigned char opt,AT_TRANS_DATA *at_data)
{
	unsigned char data[UART_SEND_MAX_LEN] = {0};
	uint8_t pwd[65] = { 0 };
	
	if(AT_GET == opt)
	{
		snprintf((char*)data,UART_SEND_MAX_LEN,"%s?\r\n",_WIFI_AT_Table[cmd_id].content);
        ESP8285UartSend(data,strlen((char*)data));
	}
	else if(AT_SET == opt)
	{
		switch ( cmd_id )
		{
            case AT_RST:
		        snprintf((char*)data,UART_SEND_MAX_LEN,"%s\r\n",_WIFI_AT_Table[cmd_id].content);
				ESP8285UartSend(data,strlen((char*)data));
		        break;
            case AT_NTP_CFG:
		        snprintf((char*)data,UART_SEND_MAX_LEN,"%s=1,8\r\n",_WIFI_AT_Table[cmd_id].content);
				ESP8285UartSend(data,strlen((char*)data));
		        break;
		    case AT_TCP_PING:
		        snprintf((char*)data,UART_SEND_MAX_LEN,"%s=\"%s\"\r\n",_WIFI_AT_Table[cmd_id].content,SERVER_IP);
				ESP8285UartSend(data,strlen((char*)data));
		        break;
		    case AT_WIFI_MODE:
		        snprintf((char*)data,UART_SEND_MAX_LEN,"%s=%s\r\n",_WIFI_AT_Table[cmd_id].content,WIFI_STA_MODE);
				ESP8285UartSend(data,strlen((char*)data));
		        break;
		    case AT_WIFI_CONNECT:
		        snprintf((char*)data,UART_SEND_MAX_LEN,"%s=\"%s\",\"%s\"\r\n",_WIFI_AT_Table[cmd_id].content,WIFI_SSID,WIFI_PSWD);
				ESP8285UartSend(data,strlen((char*)data));
		        break;
		    case AT_MQTT_USER_INFO:
		    	get_sn_val(pwd);
		        snprintf((char*)data,UART_SEND_MAX_LEN,"%s=%d,%d,\"%s\",\"%s\",\"%s\",%d,%d,\"%s\"\r\n",_WIFI_AT_Table[cmd_id].content,0,1,CLIENT_ID,PROD_ID,pwd,0,0,"");
				ESP8285UartSend(data,strlen((char*)data));
		        break;
		    case AT_MQTT_CONNECT:
		        snprintf((char*)data,UART_SEND_MAX_LEN,"%s=%d,\"%s\",%d,%d\r\n",_WIFI_AT_Table[cmd_id].content,0,SERVER_IP,SERVER_PORT,MQTT_NO_RECONNECT);
				ESP8285UartSend(data,strlen((char*)data));
		        break;
		    case AT_MQTT_SUBCRIBE:
		        snprintf((char*)data,UART_SEND_MAX_LEN,"%s=%d,\"%s\",%d\r\n",_WIFI_AT_Table[cmd_id].content,0,at_data->topic,MQTT_QOS);
				ESP8285UartSend(data,strlen((char*)data));
		        break;
		    case AT_MQTT_PUBLISH:
		    {
		    	int len = strlen((char*)at_data->content);
		    	len = UART_SEND_MAX_LEN<len?UART_SEND_MAX_LEN:len;
		    	
		    	
		        snprintf((char*)data,UART_SEND_MAX_LEN,"%s=%d,\"%s\",%d,%d,%d\r\n",_WIFI_AT_Table[cmd_id].content,0,at_data->topic,len,MQTT_QOS,0);
				ESP8285UartSend(data,strlen((char*)data));

				LOS_TaskDelay(2);
				memset(data,0,sizeof(data));
		        snprintf((char*)data,UART_SEND_MAX_LEN,"%s",at_data->content);
		        ESP8285UartSend(data,len);

		    }
		        break;
		    default:

		        break;
		}
	}

	
}

void _Json_Data_Analyse(char* data_str)
{
    cJSON* root = NULL;
    cJSON* item = NULL;
    cJSON* subitem = NULL;
    char databuf[20] = {0};
    char tmp[10] = {0};
    
    if(!data_str)
    {
        DEBUG_PRINT(MODULE_WIFI,"%s","_Json_Data_Analyse err\r\n");
        return;
    }
    

    root = cJSON_Parse(data_str);
    if(!root)
    {
        DEBUG_PRINT(MODULE_WIFI,"%s","_Json_Data_Analyse root err\r\n");
        return;
    }
    
    item = cJSON_GetObjectItem(root,"paras");
    if(item)
    {
        #if ENABLE_TRAN
        subitem = cJSON_GetObjectItem(item,"Header");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"05");    
        }
        
        subitem = cJSON_GetObjectItem(item,"T_FunCode");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);           
        }
        else
        {
            strcat(databuf,"00"); 
        }
        
        subitem = cJSON_GetObjectItem(item,"T_Road_gate_Bool_switch");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }
        
        subitem = cJSON_GetObjectItem(item,"T_Bool_OutPut_switch");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }
        
        subitem = cJSON_GetObjectItem(item,"T_License_plate_Bool");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }
        #endif
        
        #if ENABLE_INDU
        subitem = cJSON_GetObjectItem(item,"Header");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"01"); 
        }
        
        subitem = cJSON_GetObjectItem(item,"I_FunCode");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);           
        }
        else
        {
            strcat(databuf,"00"); 
        }      
        
        subitem = cJSON_GetObjectItem(item,"I_Bool_OutPut_switch");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }              
        
        subitem = cJSON_GetObjectItem(item,"I_warehouse_gate_OutPut_switch");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        
        subitem = cJSON_GetObjectItem(item,"I_Motor_slide");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }
        
        subitem = cJSON_GetObjectItem(item,"I_Air_switch");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        #endif
        
        #if ENABLE_HOME
        subitem = cJSON_GetObjectItem(item,"Header");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"03"); 
        }  
        
        subitem = cJSON_GetObjectItem(item,"H_FunCode");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);           
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        
        subitem = cJSON_GetObjectItem(item,"H_EV_Bool_OutPut_switch");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        
        subitem = cJSON_GetObjectItem(item,"H_Curtains_Motor_switch");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        
        subitem = cJSON_GetObjectItem(item,"H_Fan_OutPut_switch");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        
        subitem = cJSON_GetObjectItem(item,"H_Fan_OutPut_level");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        
        subitem = cJSON_GetObjectItem(item,"H_Door_Servo_OutPut_switch");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        
        subitem = cJSON_GetObjectItem(item,"H_Bool_OutPut_switch");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        #endif
        
        #if ENABLE_FARM
        subitem = cJSON_GetObjectItem(item,"Header");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"07"); 
        }  
        
        subitem = cJSON_GetObjectItem(item,"F_FunCode");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);           
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        
        subitem = cJSON_GetObjectItem(item,"F_fill_light_switch");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        
        subitem = cJSON_GetObjectItem(item,"F_illumination_adjust_Output");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        
        subitem = cJSON_GetObjectItem(item,"F_irrigation_switch");
        if(subitem)
        {
            memset(tmp,0,sizeof(tmp));
            snprintf(tmp,sizeof(tmp),"%02x",(int)subitem->valueint);
            strcat(databuf,tmp);
        }
        else
        {
            strcat(databuf,"00"); 
        }  
        
        #endif
    }

    DEBUG_PRINT(MODULE_WIFI,"databuf:%s\r\n",databuf);
    
    ptl_Receive_OCData_Handle((char *)databuf);
    
    if(root)
    {
        cJSON_Delete(root);
    }
    
}

void time_analyse(char* data)
{
    char week[10] = {0};
    char month[20] = {0};
    char day[5] = {0};
    char time[10] = {0};
    char year[10] = {0};
    char data_out[40] = {0};
    char time_out[40] = {0};
    int month_num = 0;
    
    sscanf(data,"%s %s %s %s %s",week,month,day,time,year);
    

    
    if(0 == strcmp(month,"Jan"))
    {
         month_num = 1;
    }
    else if(0 == strcmp(month,"Feb"))
    {
         month_num = 2;
    }
    else if(0 == strcmp(month,"Mar"))
    {
         month_num = 3;
    }
    else if(0 == strcmp(month,"Apr"))
    {
         month_num = 4;
    }
    else if(0 == strcmp(month,"May"))
    {
         month_num = 5;
    }
    else if(0 == strcmp(month,"Jun"))
    {
         month_num = 6;
    }
    else if(0 == strcmp(month,"Jul"))
    {
         month_num = 7;
    }
    else if(0 == strcmp(month,"Aug"))
    {
         month_num = 8;
    }
    else if(0 == strcmp(month,"Sep"))
    {
         month_num = 9;
    }
    else if(0 == strcmp(month,"Oct"))
    {
         month_num = 10;
    }
    else if(0 == strcmp(month,"Nov"))
    {
         month_num = 10;
    }
    else if(0 == strcmp(month,"Dec"))
    {
         month_num = 12;
    }
    else 
    {
        month_num = 1;
    }
    snprintf(data_out,sizeof(data_out),"%s.%d.%s %s",year,month_num,day,time);
    
    sscanf(data,"%s %s %s %s %s",week,month,day,time,year);
    snprintf(time_out,sizeof(time_out),"%s",time);
    
    memset(octimebuf,0,sizeof(octimebuf));
    strncpy(octimebuf,data_out,sizeof(octimebuf));

    
    memset(viewtimebuf,0,sizeof(viewtimebuf));
    strncpy(viewtimebuf,time_out,sizeof(viewtimebuf));

    time_flag = 1;
}

/*******************************************************************************
* 名    称：WIFI_connect_reqinfo_handle
* 功    能：连接成功是接收数据处理
* 参数说明：reqinfo：接收的信息
			wifi：wifi结构数据
* 返 回 值：无
* 说    明：无
* 调用方法：内部使用
*******************************************************************************/
void _WIFI_Recv_Date_Analyse(char * reqinfo,WIFI_PACKAGE* wifi)
{    
         if(strstr((char*)reqinfo,"+MQTTSUBRECV"))// 收到Mqtt数据
		 {
             
             char* req_start_index = strstr((char*)reqinfo,"request_id=");
             char* req_end_index = strchr((char*)req_start_index,'"');
             char* data_index = req_start_index+11;
             
             DEBUG_PRINT(MODULE_WIFI,"reqinfo=%s\r\n",reqinfo);
             
             memset(request_id,0,sizeof(request_id));
             memcpy(request_id,data_index,req_end_index-data_index);

             
             char* pdata = strchr((char*)req_end_index,'{');
             _Json_Data_Analyse(pdata);
             _MQTT_Response(&_WIFI);
             
		 }
         
         else if(strstr((char*)reqinfo,"+CIPSNTPTIME"))// WIFI 获取时间
		 {
             char* time_index = strstr((char*)reqinfo,"+CIPSNTPTIME:")+13;            
			if (Is_Net_Module_Connected())
            {
             	time_analyse(time_index);
			}
             DEBUG_PRINT(MODULE_WIFI,"timebuf1=%s",time_index);
		 }
         
		 else if(strstr((char*)reqinfo,"+MQTTPUB:FAIL"))// Mqtt发布失败
		 {
			_Deal_WIFI_Module_Event(MQTT_PUB_FAIL);
		 }
		 else if(strstr((char*)reqinfo,"+MQTTPUB:OK"))// Mqtt发布成功
		 {
			_Deal_WIFI_Module_Event(MQTT_PUB_OK);
		 }
         else if(strstr((char*)reqinfo,"+MQTTDISCONNECTED"))// 网络掉线
         {
             _Deal_WIFI_Module_Event(MQTT_COM_ERR);
         }
         else if(strstr((char*)reqinfo,"+MQTTCONNECTED"))// 网络建立
         {

             _Deal_WIFI_Module_Event(MQTT_COM_OK);
         } 

		 else if(strstr((char*)reqinfo,"ERROR")) 
		 {  				 		  
		     _Deal_WIFI_Module_Event(OPT_ERR);
		 }           
		  
         else if(strstr((char*)reqinfo,"OK"))// 发送数据成功
		 {

             _Deal_WIFI_Module_Event(OPT_OK);
         }
         
         if(strstr((char*)reqinfo,"OK"))// 发送数据成功
		 {
             ptl_SendSuccess_Deal(&g_ptl_SendTack);
		 }         
}

/*******************************************************************************
* 名    称：WIFI_Rec_Data_Handle
* 功    能：WIFI 接收数据处理
* 参数说明：*wifi：wifi数据结构体
* 返 回 值：无
* 说    明：无
*******************************************************************************/
void _WIFI_Rec_Data_Handle(WIFI_PACKAGE* wifi)
{
	if (stUart_WIFI.ucRecState == WIFIUART_REC_OVER) 	 
	{	
		stUart_WIFI.ucRecState = WIFIUART_REC_IDLE ;
		_WIFI_Recv_Date_Analyse(stUart_WIFI.RecInfo,wifi);
		memset(stUart_WIFI.RecInfo,0,sizeof(stUart_WIFI.RecInfo));      
	}      
}

void ESP8285_Read_Data(char* data,int len)
{
	if(len>sizeof(stUart_WIFI.RecInfo))
	{
		DEBUG_PRINT(MODULE_WIFI,"%s","ESP8285_Read_Data data too long\r\n");
		return;	
	}
	memcpy(stUart_WIFI.RecInfo, data, len);  //串口2接收到的数据复制到WIFI数据接收组
	stUart_WIFI.ucRecState = WIFIUART_REC_OVER;            //WIFI状态--接收数据完成
}


int Is_ESP8285_Connected()
{
	if(_Is_WIFI_Module_Connected())
	{
		return True;
	}
	return False;
}

void ESP8285_Init(void)
{
	_Event_Queue_Init(&_Event_Queue);
	uart_open(uart1, 115200); //打开串口1，设置波特率115200
	qrcodeflag=0;
    
    _AT_Cmd_Send(AT_RST,AT_SET,NULL);
    
    HAL_Delay(2000);
    
    ESP8285_Finger_Set_Time_Cfg();
    
    HAL_Delay(100);
    
	_Set_WIFI_Module_Event(START_SET);
}

/*******************************************************************************
* 名    称：ESP8285_Time_Deal
* 功    能：时间处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：必须1ms调用一次
* 调用方法：外部使用
*******************************************************************************/
void ESP8285_Time_Deal(void)
{
	if(_WIFI.feed_time)
	{
        _WIFI.feed_time--;
		if(0 == _WIFI.feed_time)
		{
			_Set_WIFI_Module_Event(WATCH_DOG);
		}
    }

	if(_WIFI.set_time)
	{
        _WIFI.set_time--;
		if(0 == _WIFI.set_time)
		{
			_Set_WIFI_Module_Event(OPT_TIMEOUT);
		}
    }
}

/*******************************************************************************
* 名    称：ESP8285_Handle
* 功    能：数据处理
* 参数说明：无
* 返 回 值：无
* 说    明：初始化模块，数据处理，错误判断
* 调用方法：外部使用
*******************************************************************************/
void ESP8285_Handle()
{
	if(0 != _Get_Event_Queue_Len(&_Event_Queue))
	{
		_Event_Queue_Pop(&_Event_Queue, &_WIFI.event_info);
		_Event_Handle(&_WIFI);
	}
	
    /* WIFI数据接受处理 */
    _WIFI_Rec_Data_Handle(&_WIFI);
}

void _Set_WIFI_Module_Event(EVENT_VALUE	event)
{
	EVENT_PACKAGE	event_info = {0};
	event_info.event = event;
	_Event_Queue_Push(&_Event_Queue,&event_info);
}

void _Deal_WIFI_Module_Event(EVENT_VALUE event)
{
	EVENT_PACKAGE	event_info = {0};
	event_info.event = event;
	
	memcpy(&_WIFI.event_info,&event_info,sizeof(EVENT_PACKAGE));
	_Event_Handle(&_WIFI);
}


void _MQTT_Data_Publish(char* data,int len)
{
	EVENT_PACKAGE	event_info = {0};

	if(!_Is_WIFI_Module_Connected())
	{
		return;
	}
	if(len >= sizeof(event_info.content))
	{

		return;
	}
	event_info.event = MQTT_PUB_ACT;
	memcpy(event_info.content,data,len);
	_Event_Queue_Push(&_Event_Queue,&event_info);
}

void ESP8285_Write_Data(char* data,int len)
{
	_MQTT_Data_Publish(data,len);
}

ESP8285_State_Value Get_ESP8285_State()
{
	ESP8285_State_Value ret = ESP8285_Starting;
	switch (_WIFI.state)
	{
		case STATE_OFFLINE:

			ret = ESP8285_Starting;
			break;
		case STATE_SET_WIFI_MODE:
		case STATE_SET_WIFI_STATE:
		case STATE_WIFI_CONNECT:
		case STATE_SET_MQTT_INFO:
		case STATE_MQTT_CONNECT:

			ret = ESP8285_Setting;
			break;
		case STATE_CONNECTED_SUB:
			ret = ESP8285_Setted_over;
			break;
		case STATE_CONNECTED_IDLE:
		case STATE_CONNECTED_PUB:
			ret = ESP8285_Connected;
			break;
		case STATE_DROP:
			ret = ESP8285_Drop;
			break;
		default:
			break;
	}
	return ret;
}


void ESP8285_Finger_Get_Time(void)
{
	_WIFI_Get_Time(&_WIFI);
}

void ESP8285_Finger_Set_Time_Cfg()
{
	_WIFI_Set_NTP_Cfg(&_WIFI);
}

#endif
