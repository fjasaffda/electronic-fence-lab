/*******************************************************************************
* @文件名        : net.c
* @描述          : net驱动文件
* @作者          : 物联网项目组
* @日期          : 2022/11/01
* @版本          : V2.0.0
--------------------------------------------------------------------------------
本驱动函数使用指导流程：
1、添加本驱动的.c和.h文件
2、在main.c中的WIFI任务中添加函数 Net_Module_Handle(void)
3、在main.c中的sys1ms任务中添加函数 Net_Module_Time_Deal(void)
4、完成后网络状态可以通过OLED显示屏和串口2查看对应的网络状态
------------------------------------------------------------------------------
关联驱动文件
			 esp8285.c     描述：提供wifi上云功能支持
	   bc35g.c     描述：提供bc35上云功能支持
*******************************************************************************/
#include "esp8285.h"
#include "bc35g.h"
#include "net.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "task_inter.h"

NET_PACKAGE net;
char qrcodeflag=1;    //二维码生成标志

char octimebuf[60] = {0};
char viewtimebuf[60] = {0};
char time_flag = 0;

#if NET_TYPE == NET_TYPE_NB_IOT
void BC35_Info_Update()
{
	memset(net.ID,0,sizeof(net.ID));
	snprintf(net.ID,sizeof(net.ID),"%s",_BC35G.UEIMEI);

	memset(net.line1,0,sizeof(net.line1));
	if(net.state == Net_Starting)
	{
	    snprintf(net.line1,sizeof(net.line1),"STATUS:Starting");
	}
	else if(net.state == Net_Setting)
	{
	    snprintf(net.line1,sizeof(net.line1),"STATUS:Setting");
	}
	else  if(net.state == Net_Setted_over)
	{
	    snprintf(net.line1,sizeof(net.line1),"STATUS:Setted_over");
	}
	else  if(net.state == Net_Connected)
	{
	    snprintf(net.line1,sizeof(net.line1),"STATUS:Connected");
	}
	else  if(net.state == Net_Drop)
	{
	    snprintf(net.line1,sizeof(net.line1),"STATUS:Drop");
	}

	memset(net.line2,0,sizeof(net.line2));
	snprintf(net.line2,sizeof(net.line2),"ID:%s",_BC35G.UEIMEI);

	memset(net.line3,0,sizeof(net.line3));
	snprintf(net.line3,sizeof(net.line3),"CSQ:%d",_BC35G.UECSQ);

}
#elif NET_TYPE == NET_TYPE_WIFI
void ESP8285_Info_Update()
{
	memset(net.ID,0,sizeof(net.ID));
	snprintf(net.ID,sizeof(net.ID),"%s",PROD_ID);

	memset(net.line1,0,sizeof(net.line1));
	if(net.state == Net_Starting)
	{
	    snprintf(net.line1,sizeof(net.line1),"STATUS:Starting");
	}
	else if(net.state == Net_Setting)
	{
	    snprintf(net.line1,sizeof(net.line1),"STATUS:Setting");
	}
	else  if(net.state == Net_Connected)
	{
	    snprintf(net.line1,sizeof(net.line1),"STATUS:Connected");
	}
	else  if(net.state == Net_Drop)
	{
	    snprintf(net.line1,sizeof(net.line1),"STATUS:Drop");
	}




}
#endif

void Finger_Get_Time()
{
    if(Is_Net_Module_Connected())
    {
        #if NET_TYPE == NET_TYPE_NB_IOT
        {
             BC35_Finger_Get_Time();
        }
        #elif NET_TYPE == NET_TYPE_WIFI
        {
            ESP8285_Finger_Get_Time();
        }
        #endif
    }
}

void Net_Module_Init(int type)
{
	net.type = type;
	#if NET_TYPE == NET_TYPE_NB_IOT
	{
		BC35_Init();
		net.uart_port = BC35G_UART_PORT;
	}
	#elif NET_TYPE == NET_TYPE_WIFI
	{
		ESP8285_Init();
		net.uart_port = ESP8285_UART_PORT;
	}
	#endif

}

void Net_Module_Handle(void)
{

	net.state = Get_Net_Module_State();

	#if NET_TYPE == NET_TYPE_NB_IOT
	{
		BC35G_Handle();
		BC35_Info_Update();
	}
	#elif NET_TYPE == NET_TYPE_WIFI
	{
		ESP8285_Handle();
		ESP8285_Info_Update();
	}
	#endif
}

void Net_Module_Time_Deal(void)
{
	#if NET_TYPE == NET_TYPE_NB_IOT
	{
		BC35G_Time_Deal();
	}
	#elif NET_TYPE == NET_TYPE_WIFI
	{
		ESP8285_Time_Deal();
	}
	#endif
}

void Net_Module_Write_Data(char* data,int len)
{
	#if NET_TYPE == NET_TYPE_NB_IOT
	{
		BC35G_Write_Data(data,len);
	}
	#elif NET_TYPE == NET_TYPE_WIFI
	{
		ESP8285_Write_Data(data,len);
	}
	#endif
}

void Net_Module_Read_Data(char* data,int len)
{
	#if NET_TYPE == NET_TYPE_NB_IOT
	{
		BC35G_Read_Data(data,len);
	}
	#elif NET_TYPE == NET_TYPE_WIFI
	{
		ESP8285_Read_Data(data,len);
	}
	#endif		
}

NetState_Value Get_Net_Module_State(void)
{
	NetState_Value ret = Net_Starting;

	#if NET_TYPE == NET_TYPE_NB_IOT
	{
		BC35GState_Value  bc35g_ret = BC35G_Starting;
		bc35g_ret = Get_BC35G_State();
		
		switch ( bc35g_ret )
		{
		    case BC35G_Starting:
		        ret = Net_Starting;
		        break;
		    case BC35G_Setting:
		        ret = Net_Setting;
		        break;
		    case BC35G_Setted_over:
		        ret = Net_Setted_over;
		        break;
		    case BC35G_Connected:
		        ret = Net_Connected;
		        break;
		    case BC35G_Drop:
		        ret = Net_Drop;
		        break;
		    default:

		        break;
		}
		
	}
	#elif NET_TYPE == NET_TYPE_WIFI
	{
		ESP8285_State_Value  esp8285_ret = ESP8285_Starting;
		esp8285_ret = Get_ESP8285_State();
		switch ( esp8285_ret )
		{
		    case ESP8285_Starting:
		        ret = Net_Starting;
		        break;
		    case ESP8285_Setting:
		        ret = Net_Setting;
		        break;
		    case ESP8285_Setted_over:
		        ret = Net_Setted_over;
		        break;
		    case ESP8285_Connected:
		        ret = Net_Connected;
		        break;
		    case ESP8285_Drop:
		        ret = Net_Drop;
		        break;
		    default:

		        break;
		}
	}
	#endif	

	return ret;
}

int Is_Net_Module_Connected(void)
{
	#if NET_TYPE == NET_TYPE_NB_IOT
	{
		return Is_BC35G_Connected();
	}
	#elif NET_TYPE == NET_TYPE_WIFI
	{
		return Is_ESP8285_Connected();
	}
	#endif	
}

UART_port Get_Net_Module_Uart_Port()
{
	return net.uart_port;
}

char* Get_Net_Module_ID()
{
	return net.ID;
}

char* Get_Net_Module_UI_Line1()
{
	return net.line1;
}

char* Get_Net_Module_UI_Line2()
{
	return net.line2;
}

char* Get_Net_Module_UI_Line3()
{
	return net.line3;
}


