#include "ocprotocol.h"
#include "error.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "control_task.h"
#include "lcd.h"
#include "gui.h"

/******************************内部参数宏定义***********************************/
#define   PRINTF_COLOR     BLACK    //打印输出颜色
#define   PRINTF_Y         150      //打印数据Y轴位置
#define   PRINTF_X         0        //打印数据X轴位置
#define   Appoint_Y        55       //模块状态Y轴位置
#define   Appoint_X        0        //模块状态X轴位置
#define   Appoint_COLOR    RED    //打印输出颜色

typedef struct
{
	unsigned int head;
	unsigned int tail;
	ERROR_MSG_VALUE event_buf[ERR_MQ_MAX_NUM];
}EVENT_QUEUE;

static EVENT_QUEUE  _Event_Queue;
static uint32_t sys_time = 0;
static uint8_t view_flag = 0;
static ERROR_MSG_VALUE msg;


static void _Event_Queue_Init(EVENT_QUEUE* event_queue)
{
	event_queue->head = 0;
	event_queue->tail = 0;
	memset(event_queue->event_buf,0,sizeof(ERROR_MSG_VALUE)*ERR_MQ_MAX_NUM);
}

static int _Get_Event_Queue_Len(EVENT_QUEUE* event_queue)
{
	return (event_queue->tail + ERR_MQ_MAX_NUM - event_queue->head) % ERR_MQ_MAX_NUM;
}

static void _Event_Queue_Push(EVENT_QUEUE* event_queue,ERROR_MSG_VALUE event)
{
	if(_Get_Event_Queue_Len(event_queue) >= ERR_MQ_MAX_NUM)
	{

		return;
	}
	
	event_queue->event_buf[event_queue->tail] = event;
	event_queue->tail++;
    if(event_queue->tail >= ERR_MQ_MAX_NUM)
    {
        event_queue->tail = 0;
    }
}

static void _Event_Queue_Pop(EVENT_QUEUE* event_queue,ERROR_MSG_VALUE *event)
{
	if(_Get_Event_Queue_Len(event_queue) <= 0)
	{

		return;
	}
	
	*event = event_queue->event_buf[event_queue->head];
	event_queue->head++;
    if(event_queue->head >= ERR_MQ_MAX_NUM)
    {
        event_queue->head = 0;
    }
}

void Error_Msg_View()
{

    static uint8_t state = 0;
    
    switch(state)
    {
        case 0:
            if(view_flag)
            {
                view_flag = 0;
                switch(msg)
                {
                    case ERROR_HOME_SERVO:
                    case ERROR_FARM_USART:
                    case ERROR_LIGHT_I2C:
                    case ERROR_TEMP_I2C:
                    case ERROR_CO2_I2C:
                    case ERROR_LIGHT_DATA:
                    case ERROR_TEMP_DATA:
                    case ERROR_CO2_DATA:
                        Show_Str(Appoint_X, Appoint_Y + 16 * 14, Appoint_COLOR, YELLOW, (uint8_t *)"数据读取失败，请排查故障", 16, 1);
                        break;
                    default:
                        break;
                }
                sys_time = HAL_GetTick();
                state++;
            }
            break;
        case 1:
            if(HAL_GetTick() - sys_time > 5000) 
            {
                LCD_Fill(Appoint_X, Appoint_Y + 16 * 14, 240, Appoint_Y + 16 * (14 + 1), WHITE);
                state = 0;
            }
            break;
        default:
            state = 0;
            break;
    }
	
}

void Error_Msg_Init()
{
	_Event_Queue_Init(&_Event_Queue);
}

void Error_Msg_Send(ERROR_MSG_VALUE msg)
{
	_Event_Queue_Push(&_Event_Queue,msg);
}

void Error_Msg_Handle()
{

	
	if(_Get_Event_Queue_Len(&_Event_Queue) >= 1)
	{
       
		_Event_Queue_Pop(&_Event_Queue,&msg);
		g_pltSendData.err_code = (unsigned char)msg;
        view_flag = 1;

	}
}
