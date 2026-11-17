/*******************************************************************************
* @文件名        : ocprotocol
* @描述          : oc通讯协议相关API函数
* @作者          : 物联网项目组
* @日期          : 2023/04/01
* @版本          : V2.0.0
--------------------------------------------------------------------------------
  本驱动函数使用指导流程：
  1、添加本驱动的.c和.h文件
  3、在main.c中的业务任务中添加函数 ptl_Heartbeat_Handle实现在联网时，定时发送
     心跳包数据，以及函数ptl_Handle对通讯部分进行处理
  4、在bc35g.c或者esp8285.c中，成功发送数据后，调用函数ptl_SendSuccess_Deal
  5、在control_task.c中，重新定义函数ptl_Receive_OCData_Handle，对OC下发的主动指令进行
     处理
  6、在control_task.c中，重新定义函数Task3_Deal、Task2_Deal等，对要发送的数据进行处理，
     即改变g_ptlSendBuf[]和g_ptlSendLen的值。
     要建立不同的发送任务，通过以下两条语句建立通讯发送任务，比如建立发送任务1：
     g_ptl_SendTack.TackName[g_ptl_SendTack.TackNo] = SendTask1;
     g_ptl_SendTack.TackNo ++;
     通讯模块就会发送经过Task1_Deal处理后的g_ptlSendBuf数据到OC平台。
  ------------------------------------------------------------------------------
  关联驱动文件：
               usart.c     描述：需要调用usart.c中的串口发送数据API
               net.c     描述：需要调用net.c中的网络发送数据API
*******************************************************************************/

#include "usart_api.h"
#include "net.h"
#include "ocprotocol.h"
#include "stm32l4xx_hal.h"
#include "string.h"
#include "stdio.h"
#include "log.h"
#include "task_inter.h"

#if NET_TYPE == NET_TYPE_WIFI
#include "cJSON.h"
#include "los_memory.h"
#endif


#define PTL_SEND_DATA(data,len)  Net_Module_Write_Data((char *)data,len);

/* 定义全局变量 */
PTL_SendTack_PACKAGE g_ptl_SendTack;
PTL_DATA_STRUCT g_pltSendData;
StatusStruct Scene = {1, 0};

unsigned char g_ptlSendBuf[50] = {0};
unsigned char g_ptlSendLen = 0;

unsigned char g_ptlRecBuf[100] = {0};
unsigned char g_ptlRecdLen = 0;

/* 声明内部使用函数 */
static void ptl_Send_TimerOutDeal(PTL_SendTack_PACKAGE* sendtask);

static void ptl_TaskSendDeal(PTL_SendTack_PACKAGE* sendtask);

/*******************************************************************************
* 名    称：ptl_Send_TimerOutDeal
* 功    能：通讯发送超时检测处理
* 参数说明：sendtask ------ 协议发送结构体
* 返 回 值：无
* 说    明：50ms调用一次
* 调用方法：内部使用
*******************************************************************************/
void ptl_Send_TimerOutDeal(PTL_SendTack_PACKAGE* sendtask)
{
    if (sendtask->TackState == SendTack_Doing)
    {
        //if (++sendtask->TackTime >= 6) // 超时判断时间，60*50 =3.0s
        if (HAL_GetTick() - sendtask->TackTime >= 1500) // 超时判断时间1.5s
        {
            //sendtask->TackTime = 0;
            sendtask->TackTime = HAL_GetTick();
            sendtask->TackState = SendTack_Wait;//

            if(++sendtask->TimeOutCnt >= 3)// 重发3次后，判断发送失败，暂停发送任务
            {
                sendtask->TimeOutCnt = 0;
                sendtask->TackState = SendTack_Error;// 状态改为错误异常状态
                DEBUG_PRINT(MODULE_OC, "%s", "\r\nSend timeout 3 times,Stop!");
            }
            else
            {
                DEBUG_PRINT(MODULE_OC, "\r\nSend timeout ,%d ReSend\r\n", sendtask->TimeOutCnt);
            }
        }
    }
}
/*******************************************************************************
* 名    称：ptl_SendSuccess_Deal
* 功    能：通讯发送成功处理
* 参数说明：sendtask ------ 协议发送结构体
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用，在bc35g.c 数据发送成功处调用
*******************************************************************************/
void ptl_SendSuccess_Deal(PTL_SendTack_PACKAGE* sendtask)
{
    if(sendtask->TackState == SendTack_Doing)
    {
        if(sendtask->TackNo-- <= 1)// 总任务数减-1，如果只有一个任务总数，则完成后没有任务
        {
            sendtask->TackNo = 0;
            memset((unsigned char *)&sendtask->TackName[0], 0, 5); //清除任务
        }
        else
        {
            // 当任务完成，当前任务编号置零..
            memcpy((unsigned char *)&sendtask->TackName[0], (unsigned char *)&sendtask->TackName[1],
                   sendtask->TackNo);// 任务列表移动一位
        }

        sendtask->TackTime = 0;              // 任务超时时间清零
        sendtask->TimeOutCnt = 0;	        // 任务超时计数清零
        sendtask->TackState = SendTack_Wait; // 任务状态改为空闲
    }
    if (sendtask->TackState == SendTack_Error)
    {
        sendtask->TackState = SendTack_Wait;  // 任务状态改为空闲
        DEBUG_PRINT(MODULE_OC, "%s", "\r\nptl Resumes OK");
    }
}
#if NET_TYPE == NET_TYPE_NB_IOT
/*******************************************************************************
* 名    称：Hex_to_Dec
* 功    能：把一个16进制数据拆成长度翻倍的ASCII字符串，如单个数据0x36，变为2个0x33,0x36。
* 参数说明：datain ------ 输入的16进制数据字符指针
           dataout ------ 转换后的输出ASCII字符串指针
* 返 回 值：无
* 说    明：无
* 调用方法：内部使用
*******************************************************************************/
void Hex_to_Dec(char *datain, unsigned char Len, char* dataout)
{
    unsigned char data_buf[2];
    unsigned char Hex_Len, CurNo = 0;
    Hex_Len = Len;
    for(CurNo = 0; CurNo < Hex_Len; CurNo++) // 处理收到的字符长度数据
    {
        data_buf[0] = *(datain + CurNo);
        data_buf[1] = *(datain + CurNo) << 4;
        if(data_buf[0] < 0xa0)
        {
            dataout[2 * CurNo] = (data_buf[0] >> 4) + 0x30;    // 数字字符处理
        }
        else
        {
            dataout[2 * CurNo] = (data_buf[0] >> 4) + 0x37;    // 大写字母字符处理
        }

        if(data_buf[1] < 0xa0)
        {
            dataout[2 * CurNo + 1] = (data_buf[1] >> 4) + 0x30;
        }
        else
        {
            dataout[2 * CurNo + 1] = (data_buf[1] >> 4) + 0x37;
        }
    }
}

/*******************************************************************************
* 名    称：ptl_send_data
* 功    能：通过物理外设发送通讯协议数据
* 参数说明：data ------ 待发送数据的字符串指针
             Len ------ 待发送的字符串长度
* 返 回 值：无
* 说    明：无
* 调用方法：内部使用
*******************************************************************************/
void ptl_send_data_nb_iot( char *data, unsigned char Len)
{
    char net_data[200] = {0};
    char bc35data[250] = {0};
    unsigned char bc35Len = 13;
    
    Hex_to_Dec(data, Len, net_data);
    sprintf(bc35data, "AT+NMGS=%d,%s\r\n", Len, net_data); // 把待发送的数据 拼接上AT发送数据指令
    bc35Len += Len * 2;
    DEBUG_PRINT(MODULE_OC, "%s\r\n", bc35data);
    
    PTL_SEND_DATA(bc35data, bc35Len); 

}
#endif

#if NET_TYPE == NET_TYPE_WIFI
/*******************************************************************************
* 名    称：ptl_send_data_wifi
* 功    能：通过物理外设发送通讯协议数据
* 参数说明：data ------ 待发送数据的字符串指针
             Len ------ 待发送的字符串长度
* 返 回 值：无
* 说    明：无
* 调用方法：内部使用
*******************************************************************************/
void ptl_send_data_wifi( char *data, unsigned char Len)
{
    char net_data[400] = {0};
    unsigned short net_len = 0;

    PTL_DATA_STRUCT* p = (PTL_DATA_STRUCT*)data;
	char temp_str[30] = {0};

    cJSON* root =  cJSON_CreateObject();
    cJSON* jservices = cJSON_CreateArray();
    cJSON* item =  cJSON_CreateObject();
    cJSON* jdata =  cJSON_CreateObject();

    cJSON_AddItemToObject(root,"services",jservices);

	#if ENABLE_HOME
	//内层数据
	
	cJSON_AddNumberToObject(jdata,"Header",p->Type);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->Ver,sizeof(p->Ver));
	cJSON_AddStringToObject(jdata,"Version",temp_str);
	cJSON_AddNumberToObject(jdata,"H_Curtains_Motor_switch",p->dOutput_Curtains);
	cJSON_AddNumberToObject(jdata,"H_Gas_Bool_Input",p->dInput_Gas);
	cJSON_AddNumberToObject(jdata,"H_Body_Bool_Input",p->dInput_Body);
	cJSON_AddNumberToObject(jdata,"H_EV_Bool_OutPut_switch",p->dOutput_EV_State);
	cJSON_AddNumberToObject(jdata,"H_Bool_OutPut_switch",p->dOutput_LED_State);

	cJSON_AddNumberToObject(jdata,"H_Fan_OutPut_switch",p->dOutput_Fan_State);
	cJSON_AddNumberToObject(jdata,"H_Fan_speed_level",p->dOutput_Fan_Level);
	cJSON_AddNumberToObject(jdata,"H_Door_Servo_Output_switch",p->dOutput_Servo_State);
	cJSON_AddNumberToObject(jdata,"H_Door_Servo_Bool_Input",p->dInput_Servo_Red);
	
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->Light,sizeof(p->Light));
	cJSON_AddStringToObject(jdata,"H_Light",temp_str);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->Temperature,sizeof(p->Temperature));
	cJSON_AddStringToObject(jdata,"H_Temperature",temp_str);
	
	cJSON_AddNumberToObject(jdata,"Error_msg",p->err_code);
	//外层
	
	cJSON_AddStringToObject(item,"service_id","Home");
	cJSON_AddItemToObject(item,"properties",jdata);
    cJSON_AddItemToArray(jservices,item);
    
	#endif
	#if ENABLE_TRAN
	
	//内层数据
	
	cJSON_AddNumberToObject(jdata,"Header",p->Type);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->Ver,sizeof(p->Ver));
	cJSON_AddStringToObject(jdata,"Version",temp_str);
	cJSON_AddNumberToObject(jdata,"T_Light_Bool_Count",p->dInput_Inf);
	cJSON_AddNumberToObject(jdata,"T_Car_Bool_Input",p->dInput_Car);
	cJSON_AddNumberToObject(jdata,"T_Bool_OutPut_switch",p->dOutput_SLight_State);
	cJSON_AddNumberToObject(jdata,"T_Road_gate_Bool_switch",p->dOutput_Gate);
	cJSON_AddNumberToObject(jdata,"T_run_red_light_Bool_Input",p->dInput_Red_Light);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->Light,sizeof(p->Light));
	cJSON_AddStringToObject(jdata,"T_Light",temp_str);
	cJSON_AddNumberToObject(jdata,"Error_msg",p->err_code);
	//外层
	
	cJSON_AddStringToObject(item,"service_id","Tran");
	cJSON_AddItemToObject(item,"properties",jdata);
    cJSON_AddItemToArray(jservices,item);
	
	#endif
	#if ENABLE_INDU
	//内层数据

	cJSON_AddNumberToObject(jdata,"Header",p->Type);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->Ver,sizeof(p->Ver));
	cJSON_AddStringToObject(jdata,"Version",temp_str);
	cJSON_AddNumberToObject(jdata,"I_Bool_Input",p->I_Bool_Input);
	cJSON_AddNumberToObject(jdata,"I_Bool_OutPut_switch",p->I_Bool_OutPut_switch);
	cJSON_AddNumberToObject(jdata,"I_Motor_slide",p->I_Motor_slide);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->I_Temperature,sizeof(p->I_Temperature));
	cJSON_AddStringToObject(jdata,"I_Temperature",temp_str);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->I_RFID_ID,sizeof(p->I_RFID_ID));
	cJSON_AddStringToObject(jdata,"I_RFID_ID",temp_str);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->I_punch_the_clock_time,sizeof(p->I_punch_the_clock_time));
	cJSON_AddStringToObject(jdata,"I_punch_the_clock_time",temp_str);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->I_punch_the_clock_name,sizeof(p->I_punch_the_clock_name));
	cJSON_AddStringToObject(jdata,"I_punch_the_clock_name",temp_str);
	cJSON_AddNumberToObject(jdata,"I_warehouse_gate_OutPut_switch",p->I_warehouse_gate_OutPut_switch);
    cJSON_AddNumberToObject(jdata,"I_Air_switch",p->I_Air_switch);
	cJSON_AddNumberToObject(jdata,"Error_msg",p->err_code);
	//外层
	
	cJSON_AddStringToObject(item,"service_id","Indu");
	cJSON_AddItemToObject(item,"properties",jdata);
    cJSON_AddItemToArray(jservices,item);
	#endif

	#if ENABLE_FARM
	//内层数据

	cJSON_AddNumberToObject(jdata,"Header",p->Type);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->Ver,sizeof(p->Ver));
	cJSON_AddStringToObject(jdata,"Version",temp_str);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->Temperature,sizeof(p->Temperature));
	cJSON_AddStringToObject(jdata,"F_Temperature_Input",temp_str);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->Humidity,sizeof(p->Humidity));
	cJSON_AddStringToObject(jdata,"F_humidity_Input",temp_str);
	cJSON_AddNumberToObject(jdata,"F_irrigation_switch",p->dOutput_Irrigation);
	cJSON_AddNumberToObject(jdata,"F_fill_light_switch",p->dOutput_Light_Switch);
	cJSON_AddNumberToObject(jdata,"F_illumination_adjust_Input",p->dOutput_Light_Level);
	cJSON_AddNumberToObject(jdata,"F_enclosure_Output",p->dInput_Enclosure);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->Carbon_Dioxide,sizeof(p->Carbon_Dioxide));
	cJSON_AddStringToObject(jdata,"F_Carbon_dioxide_Input",temp_str);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->Pressure,sizeof(p->Pressure));
	cJSON_AddStringToObject(jdata,"F_pressure_Input",temp_str);
	memset(temp_str,0,sizeof(temp_str));
	strncpy(temp_str,p->Light,sizeof(p->Light));
	cJSON_AddStringToObject(jdata,"F_light",temp_str);
	cJSON_AddNumberToObject(jdata,"Error_msg",p->err_code);
	//外层
	
	cJSON_AddStringToObject(item,"service_id","Farm");
	cJSON_AddItemToObject(item,"properties",jdata);
    cJSON_AddItemToArray(jservices,item);
	#endif


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

    net_len = strlen(net_data);
    
    PTL_SEND_DATA(net_data, net_len); 
    DEBUG_PRINT(MODULE_WIFI,"ptl_send_data_wifi[%d]:%s\r\n",net_len,net_data);

}
#endif

/*******************************************************************************
* 名    称：Task0_Deal
* 功    能：发送任务0处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：该函数为弱函数，当使用时，需要在user.c中重新定义
* 调用方法：外部使用
*******************************************************************************/
__weak void Task0_Deal(void)
{

}

/*******************************************************************************
* 名    称： Task1_Deal(void)
* 功    能：发送任务1处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：在ocprotocol.c中进行了弱函数定义，此处为重定义
* 调用方法：外部调用
*******************************************************************************/
__weak void Task1_Deal(void)
{

}
/*******************************************************************************
* 名    称：Task2_Deal
* 功    能：发送任务2处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：该函数为弱函数，当使用时，需要在user.c中重新定义
* 调用方法：外部使用
*******************************************************************************/
__weak void Task2_Deal(void)
{

}

/*******************************************************************************
* 名    称：Task3_Deal
* 功    能：发送任务3处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：该函数为弱函数，当使用时，需要在user.c中重新定义
* 调用方法：外部使用
*******************************************************************************/
__weak void Task3_Deal(void)
{

}

/*******************************************************************************
* 名    称：ptl_Receive_OCData_Handle(char* data)
* 功    能：接受到OC平台数据下发指令后的处理函数
* 参数说明：data：接受到OC的下发指令数据
* 返 回 值：无
* 说    明：对指令的解析，需要遵循OC平台和终端使用同一协议
* 调用方法：外部调用
*******************************************************************************/
__weak void ptl_Receive_OCData_Handle(char* data)
{

}

/*******************************************************************************
* 名    称：ptl_TaskSendDeal
* 功    能：通讯任务发送处理
* 参数说明：sendtask ------ 协议发送结构体
* 返 回 值：无
* 说    明：无
* 调用方法：内部使用
*******************************************************************************/
void ptl_TaskSendDeal(PTL_SendTack_PACKAGE* sendtask)
{
    if(!Is_Net_Module_Connected()) //在联网之后的状态可进入该函数
    {
        return;
    }
    if(sendtask->TackNo <= 0)//任务数大于0才进入任务执行
    {
        return;
    }
    if(sendtask->TackState == SendTack_Doing)
    {
        return;
    }

    if( sendtask->TackState == SendTack_Error) // 如果是异常状态，则返回
    {
        return;
    }
    else
    {
        sendtask->CurTack = sendtask->TackName[0]; // 发送最近的任务
        sendtask->TackState = SendTack_Doing; // 状态标记为任务执行状态
        sendtask->TackTime = HAL_GetTick();
    }
    /*根据不同的任务进行发送任务*/
    switch(sendtask->CurTack)
    {
    case SendTask0 :
        Task0_Deal();
        break;
    case SendTask1 :
        Task1_Deal();
        break;
    case SendTask2 :
        Task2_Deal();
        break;
    case SendTask3 :
        Task3_Deal();
        break;

    default        :
        DEBUG_PRINT(MODULE_OC, "%s", "\r\nSend task error ");
        return;//直接退出函数
    }
    #if NET_TYPE == NET_TYPE_NB_IOT
    ptl_send_data_nb_iot((char *)g_ptlSendBuf, g_ptlSendLen); //发送数据
    #else
    ptl_send_data_wifi((char *)g_ptlSendBuf, g_ptlSendLen); //发送数据
    #endif
    Scene.event_sendflag=0;
    g_pltSendData.err_code = 0;
}

/*******************************************************************************
* 名    称：ptl_SendTack_Init
* 功    能：通讯任务发送初始化处理
* 参数说明：sendtask ------ 协议发送结构体
* 返 回 值：无
* 说    明：无
* 调用方法：内部使用
*******************************************************************************/
void ptl_SendTack_Init(PTL_SendTack_PACKAGE* sendtask)
{
    sendtask->TackNo = 1;
    sendtask->TackState = SendTack_Wait;
    sendtask->TackName[0] = SendTask1;
}

/*******************************************************************************
* 名    称：ptl_Heartbeat_Handle
* 功    能：BC35G 心跳包发送频率
* 参数说明：HeartbeatTime ------ 心跳发送频率大于等于5S
* 返 回 值：无
* 说    明：1s调用一次
            当时间大于设定时间时，开始发送数据；
            在发完数据后的第2s，开始检测信号值
* 调用方法：外部使用
*******************************************************************************/
void ptl_Heartbeat_Handle(void)
{
    static uint32_t time = 0;
   
    if(!Is_Net_Module_Connected()) //在联网之后的状态可进入该函数
    {
        return;
    }
    if((HAL_GetTick() - time >5000) || (Scene.event_sendflag == 1))
    {
          time= HAL_GetTick();
        if(Is_Net_Module_Connected())
        {
            g_ptl_SendTack.TackName[g_ptl_SendTack.TackNo] = SendTask1;
            g_ptl_SendTack.TackNo =1;       
        }
    }
}

/*******************************************************************************
* 名    称：ptl_Handle
* 功    能：通讯部分的处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：500ms调用一次
* 调用方法：外部使用
*******************************************************************************/
void ptl_Handle(void)
{
    ptl_TaskSendDeal(&g_ptl_SendTack); // 通讯协议发送处理函数
    ptl_Send_TimerOutDeal(&g_ptl_SendTack);
}

