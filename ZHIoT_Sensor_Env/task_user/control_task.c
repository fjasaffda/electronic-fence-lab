/*******************************************************************************
* @文件名        : control_task
* @描述          : 任务处理
* @作者          : 物联网项目组
* @日期          : 2024/05/01
* @版本          : V1.0.0
--------------------------------------------------------------------------------
  功能包括：
  1、按键任务处理
  2、读取串口指令和网络状态值
  3、云端协议数据的打包与下发指令解析
  4、创建任务入口及任务处理函数
*******************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gpio_api.h"
#include "rtc_api.h"
#include "usart_api.h"
#include "log.h"
#include "lcd.h"
#include "net.h"
#include "ocprotocol.h"
#include "task_inter.h"
#include "error.h"
#include "control_task.h"
#include "env_control.h"

UINT32 g_ControlTskHandle;

/******************************内部变量定义************************************/

uint16_t net_uart_len;      //串口数据长度变量
uint16_t debug_uart_len;    //串口数据长度变量

uint8_t *net_uart_buffer;    //串口数据接收数组
uint8_t *debug_uart_buffer;  //串口数据接收数组

ViewDataStruct EnvView;      //状态结构体


/******************************内部函数声明************************************/
void Ptl_Data_Init(void);
static void Get_Net_UI_Info(void);
/*******************************************************************************
* 名    称：key1_end_handle(void)
* 功    能：按键Key1结束处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：该函数为弱函数的重新定义函数
* 调用方法：外部调用
*******************************************************************************/
void key1_end_handle(void)
{

}

/*******************************************************************************
* 名    称：key2_end_handle(void)
* 功    能：按键Key2结束处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：该函数为弱函数的重新定义函数
* 调用方法：外部调用
*******************************************************************************/
void key2_end_handle(void)
{

}

/*******************************************************************************
* 名    称：key3_end_handle(void)
* 功    能：按键Key3结束处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：该函数为弱函数的重新定义函数，用于打开/关闭调试打印
* 调用方法：外部调用
*******************************************************************************/
void key3_end_handle(void)
{
    debugflag = !debugflag;
}

/*******************************************************************************
* 名    称：key4_end_handle(void)
* 功    能：按键Key4结束处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：联网成功后按键无效，仅提示等待
* 调用方法：外部调用
*******************************************************************************/
void key4_end_handle(void)
{
    if(Is_Net_Module_Connected())
    {
        printf("请等待联网成功\r\n");
        return;
    }

}

/*******************************************************************************
* 名    称：Ptl_Data_Init(void)
* 功    能：OC通信协议初始化函数
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部函数
*******************************************************************************/
void Ptl_Data_Init(void)
{
    g_pltSendData.Type = 0x06;           // 0x06是农业场景
    /*固件版本号200*/
    g_pltSendData.Ver[0] = '2';
    g_pltSendData.Ver[1] = '0';
    g_pltSendData.Ver[2] = '0';
}

/*******************************************************************************
* 名    称：read_uart_data(void)
* 功    能：读取串口数据
* 参数说明：无
* 返 回 值：无
* 说    明：由系统 1ms 定时调用
* 调用方法：外部调用
*******************************************************************************/
void read_uart_data(void)
{
    net_uart_buffer = uart_read(Get_Net_Module_Uart_Port(), &net_uart_len);       //net串口读取数据

    if(net_uart_len > 0)
    {
        Net_Module_Read_Data((char*)net_uart_buffer,net_uart_len);
        net_uart_len = 0;             //串口接收数据长度为0
        clearrxbuffer(Get_Net_Module_Uart_Port());      //清除数据
    }
    debug_uart_buffer = uart_read(DEBUG_UART, &debug_uart_len);       //debug串口读取数据
    if(debug_uart_len > 0)
    {
        printf("%s", debug_uart_buffer);              //打印AT指令
        uart_write(Get_Net_Module_Uart_Port(), debug_uart_len, debug_uart_buffer); //debug串口中数据写入net串口

        clearrxbuffer(DEBUG_UART);            //清除数据
        debug_uart_len = 0;                   //debug串口数据接收长度赋0
    }
}

/*******************************************************************************
* 名    称：Get_Net_UI_Info
* 功    能：更新网络状态
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部函数
*******************************************************************************/
static void Get_Net_UI_Info(void)
{
    char* p = NULL;

    EnvView.state = Get_Net_Module_State();

    p = Get_Net_Module_UI_Line1();
    if(0 != p[0])
    {
        memset(EnvView.Line1, 0, sizeof(EnvView.Line1));
        strncpy((char*)EnvView.Line1, p, sizeof(EnvView.Line1));
    }

    p = Get_Net_Module_UI_Line2();
    if(0 != p[0])
    {
        memset(EnvView.Line2, 0, sizeof(EnvView.Line2));
        strncpy((char*)EnvView.Line2, p, sizeof(EnvView.Line2));
    }

    p = Get_Net_Module_UI_Line3();
    if(0 != p[0])
    {
        memset(EnvView.Line3, 0, sizeof(EnvView.Line3));
        strncpy((char*)EnvView.Line3, p, sizeof(EnvView.Line3));
    }
}

/*******************************************************************************
* 名    称： Task1_Deal
* 功    能：发送任务1处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：固件版本号、场景类型在这里统一填写，传感器数据由 env_task 负责填充
* 调用方法：外部调用
*******************************************************************************/
void Task1_Deal(void)//获取数据 5s一次
{
    g_ptlSendLen = sizeof(PTL_DATA_STRUCT);	 //计算OC通讯协议数据内存
    memcpy((uint8_t *)&g_ptlSendBuf[0], (uint8_t *)&g_pltSendData.Type,
           g_ptlSendLen); //复制pltSendData到ptlSendBuf
    /*农业场景*/
    g_pltSendData.Type = 0x06;
    /*固件版本号200*/
    g_pltSendData.Ver[0] = 0x32;
    g_pltSendData.Ver[1] = 0x30;
    g_pltSendData.Ver[2] = 0x30;
}

/*******************************************************************************
* 名    称：ptl_Receive_OCData_Handle(char* data)
* 功    能：接受到OC平台数据下发指令后的处理函数
* 参数说明：data：接收到的下发指令（十六进制字符串）
* 返 回 值：无
* 说    明：农业场景下发数据格式（十六进制字符串）：
            Header(1字节) + F_FunCode(1字节) + 补光灯开关(1字节)
            + 补光灯档位(1字节) + 灌溉/风扇开关(1字节)
* 调用方法：外部调用
*******************************************************************************/
void ptl_Receive_OCData_Handle(char* data)
{
    unsigned int header = 0;
    unsigned int funcode = 0;
    unsigned int light_switch = 0;
    unsigned int light_level = 0;
    unsigned int fan_switch = 0;

    if(data == NULL)
    {
        return;
    }

    DEBUG_PRINT(MODULE_OC, "Rec data =%s ", data); // 打印出接收到的数据

    if(strlen(data) < 6)
    {
        return;
    }

    sscanf(data, "%2x%2x%2x%2x%2x", &header, &funcode, &light_switch, &light_level, &fan_switch);

    /* 0x06 为农业场景；平台未携带场景头时默认填 0x07，两种都按农业场景处理 */
    if((header != 0x06) && (header != 0x07))
    {
        printf("** 非本场景指令，忽略：头=%02X\r\n", (int)header);
        return;
    }

    printf("** 收到云端控制指令：灯开关=%d 档位=%d 风扇=%d\r\n",
           (int)light_switch, (int)light_level, (int)fan_switch);

    Env_Lamp_Level((uint8_t)light_level);        /* 先给档位，再决定开关 */
    Env_Lamp_Switch((uint8_t)light_switch);
    Env_Fan_Switch((uint8_t)fan_switch);

    /* 状态已经改变，立即上报云端确认 */
    Scene.event_sendflag = 1;
}

/*******************************************************************************
* 名    称：task_control_init(void)
* 功    能：控制初始化函数
* 参数说明：无
* 返 回 值：无
* 说    明：外部调用接口函数
* 调用方法：外部函数
*******************************************************************************/
void task_control_init(void)
{
    gpio_module_open(LEDS);

    Net_Module_Init(NET_TYPE_WIFI);
    Error_Msg_Init();
    Env_Actuator_Init();
    uart_open(DEBUG_UART, 115200); //打开串口，设置波特率115200
    printf("\r\n     环境监测场景实训     \r\n");
    printf("------------------------------\r\n");
    printf("开始本实验：\r\n\r\n");
    /* 发送协议数据初始化 */
    Ptl_Data_Init();

}

/*******************************************************************************
* 名    称： control_task(VOID)
* 功    能：数据获取、业务逻辑处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部函数
*******************************************************************************/
VOID control_task(VOID)
{
    UINT32 uwRet = LOS_OK;
    static uint32_t sys_time = 0;

    while(1)
    {
        ptl_Handle();     //通讯部分处理
        Error_Msg_Handle();

        if(HAL_GetTick() - sys_time > 1000)    //LED灯1S闪一次
        {
            sys_time = HAL_GetTick();
            led_toggle(LED1_PORT);
        }

        Get_Net_UI_Info();
        ptl_Heartbeat_Handle();    //发送心跳包

        Net_Module_Handle();       //网络处理函数

        if(Scene.req_state == 1)
        {
            Scene.state = 0;//状态出现变化才赋值1，用于通知其他线程状态变化，并做出响应
        }

        uwRet = LOS_TaskDelay(10);
        if(uwRet != LOS_OK)
        {
            return;
        }
    }
}

/*******************************************************************************
* 名    称：creat_control_task(VOID)
* 功    能：创建control_task任务
* 参数说明：无
* 返 回 值：uwRet：LOS_OK 表示创建任务成功；否则表示失败
* 说    明：外部调用
* 调用方法：在main函数中调用
*******************************************************************************/
UINT32 creat_control_task(VOID)
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;   //定义结构体

    task_init_param.usTaskPrio = 0;     //任务优先级
    task_init_param.pcName = "control_task"; //命名任务名称
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)control_task;  //创建任务入口
    task_init_param.uwStackSize = 0xA00;   //任务堆栈大小
    uwRet = LOS_TaskCreate(&g_ControlTskHandle, &task_init_param);
    printf("** 联网模块与执行器初始化\r\n");
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}
