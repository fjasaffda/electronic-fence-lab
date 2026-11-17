/*----------------------------------------------------------------------------
* @文件名        : mq_5
* @描述          : 激光测距传感器配置相关函数
* @作者          : 物联网项目组
* @日期          : 2023/04/01
* @版本          : V0.0.2
******************************************************************************
本驱动函数使用指导流程：
1、添加本驱动的.c和.h文件
2、在main.c中调用DYP_Init函数进行初始化
3、通过调用DYP_Read_State的返回值读取传感器值
******************************************************************************/
#include <stdio.h>
#include "stm32l4xx_hal.h"
#include "dyp_r01.h"
#include "usart.h"
#include "usart_api.h"
#include "error.h"
#include "control_task.h"

/*****************************IO引脚宏定义**************************************/	


extern UART_HandleTypeDef hlpuart1;
extern void MX_LPUART1_UART_Init(uint32_t BaudRate);


/*****************************IO操作函数****************************************/	

#define DYP_UART lpuart1

extern uint8_t uart_flag;
uint8_t send_data[] = {0x01,0x03,0x01,0x00,0x00,0x01,0x85,0xF6};
UINT32 g_FenceTskHandle;
uint16_t data1_cache;
uint16_t data2_cache;

void DYP_Send_Data()
{

	HAL_UART_Transmit(&hlpuart1, send_data, sizeof(send_data), 100);
}

void UART_Change()
{

	HAL_UART_DeInit(&hlpuart1);

	if(uart_flag)
	{
		uart_flag = 0;
	}
	else
	{
		uart_flag = 1;
	}

	HAL_Delay(1);
	MX_LPUART1_UART_Init(115200);
	HAL_Delay(1);
}




/********************************************************************************
* 名    称：DYP_Init
* 功    能：初始化DYP
* 参数说明：无
* 返 回 值：无
* 说    明：
* 调用方法：外部函数
********************************************************************************/
void DYP_Init(void)
{
    uart_open(lpuart1,115200);
}


void print_buf(char* data,int len)
{
    int i;
    printf("\r\n=============");
    for(i = 0; i < len ; i++)
    {
    	if(0 == i%16)
    	{
    		printf("\r\n");
    	}
    	printf("%02X ",data[i]);
    }
    printf("\r\n=============\r\n");
}

/********************************************************************************
* 名    称：DYP_Read_State
* 功    能：读取DYP的状态
* 参数说明：无
                      
* 说    明：
* 调用方法：外部函数
********************************************************************************/
void DYP_Read_State(uint16_t* data1,uint16_t* data2)
{
    uint16_t uart_len = 0;
	uint16_t dis = 0;
    static uint16_t dis1 = 0;
    static uint16_t dis2 = 0;
	static int cnt = 0;
	uint8_t uart_buffer[50] = {0};
    uint8_t* data_tmp=NULL;
    uint16_t data_len = 0;
    int time_cnt = 0;
    
    
    do
    {
        data_tmp = uart_read(lpuart1, &uart_len);
        LOS_TaskDelay(50);
        if(time_cnt++>10)
        {   
            return;
        }
        
    }while(uart_len==0);
    memcpy(uart_buffer,data_tmp,uart_len);
    if(uart_len > 0)
    {
        if((uart_buffer[0] == 0x01)&&(uart_buffer[1] == 0x03))
		{
			dis = (uart_buffer[3]<<8) +uart_buffer[4];
            
            if(dis > 9999)
            {
                dis = 9999;
            }

            if(uart_flag)
            {
            	dis2 = dis;
            }
            else
            {
            	dis1 = dis;
            }

            cnt = 0;

		    UART_Change();


		}
		else
		{

		}
    }else
    {
        Error_Msg_Send(ERROR_FARM_USART);
    }

    if(cnt++ > 2)
    {
    	cnt = 0;
        if(uart_flag)
        {
            dis2 = 0;
        }
        else
        {
            dis1 = 0;
        }
        
	    UART_Change();

    }
    
    *data1 = dis1;
    *data2 = dis2;
    clearrxbuffer(lpuart1);
}

/********************************************************************************
* 名    称：DYP_Read_Data
* 功    能：读取DYP的缓存数据
* 参数说明：无
                      
* 说    明：
* 调用方法：外部函数
********************************************************************************/
void DYP_Read_Data(uint16_t* data1,uint16_t* data2)
{
    *data1 = data1_cache;
    *data2 = data2_cache;
}

/*******************************************************************************
* 名    称： control_task(VOID)
* 功    能：数据获取、业务逻辑处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部函数
*******************************************************************************/
VOID fence_task(VOID)
{
    UINT32 uwRet = LOS_OK;


    while(1)
    {

        DYP_Send_Data();
        DYP_Read_State(&data1_cache,&data2_cache);

        uwRet = LOS_TaskDelay(100);
        if(uwRet != LOS_OK)
        {
            return;
        }
    }
}

/*******************************************************************************
* 名    称：creat_fence_task(VOID)
* 功    能：创建fence_task任务
* 参数说明：无
* 返 回 值：uwRet：LOS_OK 表示创建任务成功；否则表示失败
* 说    明：外部调用
* 调用方法：在main函数中调用
*******************************************************************************/

UINT32 creat_fence_task(VOID)
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;   //定义结构体

    task_init_param.usTaskPrio = 0;     //任务优先级
    task_init_param.pcName = "fence_task"; //命名任务名称
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)fence_task;  //创建任务入口
    task_init_param.uwStackSize = 0x200;   //任务堆栈大小
    uwRet = LOS_TaskCreate(&g_FenceTskHandle, &task_init_param);

    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}
