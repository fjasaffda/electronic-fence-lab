/*******************************************************************************
* @文件名        : bc35g
* @描述          : bc35g驱动文件
* @作者          : 物联网项目组
* @日期          : 2023/04/01
* @版本          : V2.0.0
--------------------------------------------------------------------------------
本驱动函数使用指导流程：
1、添加本驱动的.c和.h文件
2、在net.c中的Net_Module_Handle任务中添加函数 BC35G_Handle(void)
3、在net.c中的Net_Module_Time_Deal任务中添加函数 BC35G_Time_Deal(void)
4、完成后网络状态可以通过OLED显示屏和串口查看对应的网络状态
------------------------------------------------------------------------------
关联驱动文件：usart.c     描述：需要usart.c提供串口的发送函数
			 gpio.c     描述：需要gpio.c对PWR的GPIO控制端口、以及对LED的控制
	   ocprotocol.c     描述：需要对通讯协议相关信息进行处理
*******************************************************************************/
/* User includes */
#include "usart_api.h"
#include "gpio_api.h"
#include "bc35g.h"
#include "ocprotocol.h"
#include "log.h"
/* Standard includes */
#include "stdio.h"
#include "string.h"
#include "net.h"
#include "task_inter.h"

#if NET_TYPE == NET_TYPE_NB_IOT

int return_num = 0;

extern char octimebuf[60];
extern char viewtimebuf[60];
extern char time_flag;


/* 定义BC35G结构体 */
BC35G_PACKAGE _BC35G;
BC35G_UART_PACKAGE stUart_BC35G;
BC35G_Err_PACKAGE BC35G_Err;

void time_analyse(char* data);

/* 声明内部使用函数 */
static void BC35G_Rec_Data_Handle(BC35G_PACKAGE* bc35g);
static void BC35G_UE_Config(BC35G_PACKAGE* bc35g);
static void BC35G_Config_judge(BC35G_PACKAGE* bc35g);
static void BC35G_ErrorDeal(BC35G_PACKAGE* bc35g);

/*******************************************************************************
* 名    称：BC35G_Time_Deal
* 功    能：BC35G时间处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：必须1ms调用一次
* 调用方法：外部使用
*******************************************************************************/
void BC35G_Time_Deal(void)
{
	if(_BC35G.feed_time)
	{
        _BC35G.feed_time--;
    }

	if(_BC35G.set_time)
	{
        _BC35G.set_time--;
    }
}

/*******************************************************************************
* 名    称：BC35G_UE_Config
* 功    能：BC35G初始化设置
* 参数说明：无
* 返 回 值：无
* 说    明：模块状态变化说明
            BC35G_Starting   进行上电动作
            BC35G_Setting     开始发送AT指令
            BC35G_Setted_over 发送AT指令完成  
* 调用方法：内部使用
*******************************************************************************/
void BC35G_UE_Config(BC35G_PACKAGE* bc35g)
{   
    BC35GState_Value   state;
    unsigned char setstate;
    BC35GPower_Value   powerstate; 
    unsigned int    settime,feedtime;	
    char connectbuffer[30]={0};
    int bc35_ing = 0;
	static uint8_t cnt = 0;

	/* 将结构体的值赋值给函数局部变量 */
	state = bc35g->State;
	powerstate = bc35g->power_state;
	settime = bc35g->set_time;
	setstate = bc35g->set_state;
	feedtime = bc35g->feed_time;	
	/*
	   第一步：给模块上电，调用BC35G_PWR_H，如果模块故障则可以重新上电
	*/
	if(state == BC35G_Starting) 
	{
        if(powerstate == POWER_OFF)
		{	
		    powerstate = POWER_ING;	 
            BC35G_PWR_H;
            settime = 8000;// 模块上电后等待初始化8s
            feedtime = 10000;//10s 喂狗时间
			DEBUG_PRINT(MODULE_BC35,"%s","1.BC35G power on,please wait 8s...\r\n");      
		 }
         if (stUart_BC35G.ucRecState == BC35UART_REC_OVER) 	 
		 {	
            stUart_BC35G.ucRecState = BC35UART_REC_DEIL ; 			 
            if(strstr((char*)stUart_BC35G.RecInfo,"OK"))//开机成功
            { 
				
                powerstate = POWER_ON;
                bc35_ing = 1;
                state=BC35G_Setting;//识别到BC35启动成功，开始配置
                DEBUG_PRINT(MODULE_BC35,"%s","BC35 is opened...\r\n");
            }
            memset(stUart_BC35G.RecInfo,0,200);
	     } 
		 if(((!settime)&&(powerstate == POWER_ING))||(powerstate == POWER_ON)) // 当settime为0，即8s后进入该判断语句
		 {
			if((BC35G_Err.ErrFunc == RestartPWR)||(powerstate == POWER_ING))// 如果需要重启模块，或者上电超时，则给模块重新上电
			{   
				DEBUG_PRINT(MODULE_BC35,"%s","BC35G ReStart...\r\n");
				UartSend(RESET_NB); 	//重启BC35
                bc35delay(2000);
                UartSend(UE_UART_CHECK); 
				settime = 5000;//等待3s		没有起到等待的作用	
                BC35G_Err.ErrFunc=0;//清除状态
				
				powerstate = POWER_OFF;//清除状态 重新上电
				
			}
			else
			{
                if(bc35_ing == 1)
                {				
                    DEBUG_PRINT(MODULE_BC35,"%s","2.BC35G Starting...\r\n");
                    state=BC35G_Setting; ////识别到BC35启动成功，开始配置
                }
			}		 
		 }
	     /* 改变结构体数值 */
		 bc35g->State = state;
	     bc35g->power_state = powerstate;
	     bc35g->set_time = settime;
	     bc35g->feed_time = feedtime;  	 			
	}
	/*
	   第二步：当模块完成上电后，开始进行AT指令的设置，共需要设置11个AT指令   
               每个指令发送之间间隔一定的等待时间，当settime=0时才开始发送AT指令 
	*/
	if(state != BC35G_Setting) 
        return;
	if(settime)
		return;
		
	DEBUG_PRINT(MODULE_BC35,"\r\nBC35G Setting %d/12...\r\n",setstate);
    NET_LED_ON; // 打开LED	
	switch(setstate)
	{
		 case 0 : 
                UartSend(UE_UART_CHECK); // 检测串口通讯是否通
		        DEBUG_PRINT(MODULE_BC35,"%s",UE_UART_CHECK);        
                settime = 300;//300ms
		        break;
		 
  		 case 1 : 
                UartSend(OPEN_AUTOCONNECT); // 使能自动入网
                DEBUG_PRINT(MODULE_BC35,"%s",OPEN_AUTOCONNECT);
		        settime = 500;//500ms
		        break;
		 
		 case 2 : 
                UartSend(GET_UE_IMEI); // 获取模块IMEI号
                DEBUG_PRINT(MODULE_BC35,"%s",GET_UE_IMEI);
		        settime = 500;//500ms
		        break;
         
		 case 3 :		 
                if(_BC35G.UEIMEI[0] == 0)
                {
                    bc35g->set_state--;//循环执行本条指令
                    bc35g->set_num--;
                    return_num++;
                    break;
                }
                UartSend(AT_CMD_ENABLE_PRINT); // 使能打印错误消息
                DEBUG_PRINT(MODULE_BC35,"%s",AT_CMD_ENABLE_PRINT);
		        settime = 500;//500ms
		        break;
         
		 case 4 : 
                UartSend(AT_CMD_ENABLE_REMIND); // 设置模块收到数据时主动提醒
                DEBUG_PRINT(MODULE_BC35,"%s",AT_CMD_ENABLE_REMIND);
		        settime = 500;//500ms
		        break;	
		
		 case 5 :
                UartSend(CFUN); // CFUN 开启射频
                DEBUG_PRINT(MODULE_BC35,"%s",CFUN);
                settime = 4000;//4000ms
		        break;
         
   	     case 6 : 
                UartSend(AT_GET_SIM_CIMI);// 获取SIM IMSI号，确保SIM物理连接正常
 		        DEBUG_PRINT(MODULE_BC35,"%s",AT_GET_SIM_CIMI);
                settime = 800;//800ms
                break;
   	     case 7 : 
                UartSend(GET_UE_CSQ); // 查询信号值
                DEBUG_PRINT(MODULE_BC35,"%s",GET_UE_CSQ);
                settime = 1000;//500ms
                break;
         case 8 :
                UartSend(CGATT_ATTACH); // 附着网络到基站
                DEBUG_PRINT(MODULE_BC35,"%s",CGATT_ATTACH);
         		settime = 1000;//1000ms
		        break;
         case 9:
                UartSend(AT_CMD_ENABLE_NET_SEARCH); // 注册ESP网络
                DEBUG_PRINT(MODULE_BC35,"%s",AT_CMD_ENABLE_NET_SEARCH);
		        settime = 1000;//1000ms
		        break;      
         case 10 :
			    if(_BC35G.net_state[2] != '1')
                {
                    bc35g->set_state--;//循环执行本条指令
					cnt++;
					if(cnt > 10)	//循环查询网络注册状态
					{
						return_num++;	//3个循环后未查询到注册信息，则重启
						cnt = 0;
					}
                    break;
                }
                sprintf(connectbuffer,"AT+NCDP=%s,%s\r\n",OC_IP,OC_IP_PORT); 
                UartSend(connectbuffer); // 连接到OC服务器平台
                DEBUG_PRINT(MODULE_BC35,"%d,%s",sizeof(connectbuffer)-1,connectbuffer);               
		        settime = 6000;//6000ms
		        break;	
      
         case 11 :
                UartSend(GET_UE_IP); // 获得模块返回IP地址
                DEBUG_PRINT(MODULE_BC35,"%s",GET_UE_IP);
		        settime = 500;//500ms
		        break;	
 
		 case 12 :
                UartSend(GET_UE_NET_STATE); // 检测模块联网状态
                DEBUG_PRINT(MODULE_BC35,"%s",GET_UE_NET_STATE);
		        settime = 500;//500ms														
		        state = BC35G_Setted_over;
		        break;	
         
	   default :break;
	}	
     /* 改变结构体数值 */
	 bc35g->State = state;
	 bc35g->set_time = settime;
	 bc35g->feed_time = feedtime; 			
}

/*******************************************************************************
* 名    称：BC35_Setting_reqinfo_handle
* 功    能：设置过程中接收的信息处理
* 参数说明：reqinfo：接收的信息
			bc35g：bc35结构数据
* 返 回 值：无
* 说    明：无
* 调用方法：内部使用
*******************************************************************************/
void BC35_Setting_reqinfo_handle(char * reqinfo,BC35G_PACKAGE* bc35g)
{   
        DEBUG_PRINT(MODULE_BC35,"%s\r\n",reqinfo);
        if(strstr((char*)reqinfo,"OK"))
        {         
            if(strstr((char*)reqinfo,"+CGSN"))
            { 
                sscanf((char *)reqinfo,"\r\n+CGSN:%s\r\n",bc35g->UEIMEI);
                qrcodeflag=0;
            }
			else if(strstr((char*)reqinfo,"+CEREG"))
            { 
                sscanf((char *)reqinfo,"\r\n+CEREG:%s\r\n",bc35g->net_state);      			
            }
            else if(strstr((char*)reqinfo,"+CSQ:"))
            {
                sscanf((char *)reqinfo,"\r\n+CSQ:%d,",&bc35g->UECSQ);
                if((bc35g->UECSQ >31) || ( bc35g->UECSQ == 0 ))
                {
					DEBUG_PRINT(MODULE_BC35,"CSQ=%d,signal error..\r\n",bc35g->UECSQ);
					DEBUG_PRINT(MODULE_BC35,"%s","Check CSQ\r\n");
					bc35g->set_state--;
                }
            }
            

            else if(strstr((char*)reqinfo,"+CGPADDR"))
            {
                if(*(reqinfo+12) == ',')
                {
                    DEBUG_PRINT(MODULE_BC35,"Get ip %s\r\n",reqinfo);
                    bc35g->set_state++;//执行下一条指令
				    bc35g->set_num++;
                    return;
                }
                else
                {
                    return_num++;
                    bc35g->set_time=2000;
                    bc35g->feed_time=3000;
                    bc35g->set_state=4;
                    return_num++;
                    DEBUG_PRINT(MODULE_BC35,"%s","No Get ip\r\n");
                }
            }
			DEBUG_PRINT(MODULE_BC35,"BC35G Set set_num=%d---->OK\r\n",bc35g->set_num);                   
			bc35g->set_state++;//执行下一条指令
			bc35g->set_num++;      
        }
        else if(strstr((char*)reqinfo,"ERROR"))//在设定过程中，模块返回ERROR，对应处理
        {
             unsigned  int error_code = 0; 
             return_num++;
             sscanf((char *)reqinfo,"\r\n+CME ERROR:%d,",&error_code);	  
			 DEBUG_PRINT(MODULE_BC35,"\r\nError Code = %d \r\n",error_code);
          
			 if(error_code == 523)
			 {
			     DEBUG_PRINT(MODULE_BC35,"%s","UART frame error\r\n");	
				 g_ptl_SendTack.TackState=SendTack_Wait;// 任务状态改为空闲						
			 } 
             if(bc35g->set_state == 6)
             {
                 bc35g->set_time=2000;
                 bc35g->feed_time=3000;
                 DEBUG_PRINT(MODULE_BC35, "%s", "Check SIM Card\r\n");
             }
        }
        if(return_num >= 3)
        {
           bc35g->State=BC35G_Starting;      // 重新开始配置BC35G模块
           bc35g->set_state=0;
           bc35g->set_time=2000;
           bc35g->feed_time=5000;
           bc35g->power_state=POWER_ON;
           BC35G_Err.ErrFunc = RestartPWR;
           return_num = 0;
        }
}

/*******************************************************************************
* 名    称：BC35_Lastset_reqinfo_handle
* 功    能：最后一步回复的状态处理
* 参数说明：reqinfo：接收的信息
			bc35g：bc35结构数据
* 返 回 值：无
* 说    明：无
* 调用方法：内部使用
*******************************************************************************/
void BC35_Lastset_reqinfo_handle(char * reqinfo,BC35G_PACKAGE* bc35g)
{
		  if(strstr((char*)reqinfo,"+NMSTATUS:")) // 网络状态反馈
		  {	
              //获取模组组网返回状态 并进行对应处理
			  if(strstr((char*)reqinfo,"INIITIALISED"))
			  {
                  DEBUG_PRINT(MODULE_BC35,"%s","UE iniitialised\r\n");
				  bc35g->State = BC35G_Setting;
                  return_num++;
				  bc35g->set_state=5;						   	 
			  }
			  else if(strstr((char*)reqinfo,"REGISTERING"))
			  {
				  DEBUG_PRINT(MODULE_BC35,"%s","UE registering...\r\n");
				  bc35g->State = BC35G_Setting;
                  return_num++;
				  bc35g->set_state=5;						   			 
			  }
			  else if(strstr((char*)reqinfo,"MO_DATA_ENABLED"))
			  {
                  DEBUG_PRINT(MODULE_BC35,"%s","UE connect OK\r\n");	
				  bc35g->State = BC35G_Connected;
                  NET_LED_ON;
				  bc35g->First_Connected_Flag = 1;	              
			  }
			  else if(strstr((char*)reqinfo,"REJECTED_BY_SERVER"))
			  {
                  DEBUG_PRINT(MODULE_BC35,"%s","UE rejected by server\r\n");	
				  bc35g->State = BC35G_Setting;
                  return_num = 3;
                  NET_LED_OFF;				 		  				  
			  }
              else
              {
                  DEBUG_PRINT(MODULE_BC35,"%s","Other Status\r\n");	
				  bc35g->State = BC35G_Setting;
				  bc35g->set_state=5;
                  return_num++;
              }
		  }
		  else if(strstr((char*)reqinfo,"ERROR")) 
		  {  	
//              DEBUG_PRINT(MODULE_BC35,"\r\n%s",reqinfo);
              return_num++;	  
		  }
          if(return_num >= 3)
          {
             bc35g->State=BC35G_Starting;      // 重新开始配置BC35G模块
             bc35g->set_state=0;
             bc35g->set_time=2000;
             bc35g->feed_time=5000;
             bc35g->power_state=POWER_ON;
             BC35G_Err.ErrFunc = RestartPWR;
             return_num = 0;
          }
}

uint8_t lunar_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

void time_zone_change(int *year,int *month,int *day,int *hour,int offset_hour)
{

    int year_tmp = *year; 
    int month_tmp = *month; 
    int day_tmp = *day; 
    int hour_tmp = *hour; 

     if((*hour + offset_hour) >= 24)
     {
        hour_tmp =  (*hour + offset_hour) - 24;
         
        if((*month == 1) || (*month == 3) || (*month == 5) || (*month == 7) ||
                       (*month == 8) || (*month == 10) || (*month == 12))
        {
            if(*day == 31)
            {
                 if(*month == 12)
                 {
                    year_tmp += 1; 
                 }
                 month_tmp = (month_tmp+1)%12;
                 day_tmp = 1;
            }
            else
            {
                
                 day_tmp = day_tmp+1;
            }
                
        }
        else if(*month == 2)
        {
            if(lunar_year(*year))
            {
                if(*day == 29)
                {
                     month_tmp = 3;
                     day_tmp = 1;
                }
                else
                {
                    
                     day_tmp = day_tmp+1;
                }
            }
            else
            {
                if(*day == 28)
                {
                     month_tmp = 3;
                     day_tmp = 1;
                }
                else
                {
                    
                     day_tmp = day_tmp+1;
                }
                 
            }
        }
        else
        {
            if(*day == 30)
            {
                 month_tmp = (month_tmp+1)%12;
                 day_tmp = 1;
            }
            else
            {
                
                 day_tmp = day_tmp+1;
            }
        }
         
     }
     else
     {
        hour_tmp =  (*hour + offset_hour);
     }
    
    *year = year_tmp; 
    *month = month_tmp; 
    *day = day_tmp; 
    *hour = hour_tmp;
}

void time_analyse(char* data)
{
    int month = {0};
    int day = {0};
    //char time[10] = {0};
    int year = {0};
    char time_offset[5] = {0};
    char data_out[40] = {0};
    char time_out[40] = {0};
    int hour;
    int minute;
    int second;

    sscanf(data,"%d/%d/%d,%d:%d:%d+%s",&year,&month,&day,&hour,&minute,
                                        &second,time_offset);
    

    if(year > 50)
    {
        year = 1900 + year;
    }
    if(year <= 50)
    {
        year = 2000 + year;
    }

    
    time_zone_change(&year,&month,&day,&hour,8);

    snprintf(data_out,sizeof(data_out),"%d.%02d.%02d %02d:%02d:%02d",year,month,day,
                                            hour,minute,second);

    sscanf(data_out,"%d.%02d.%02d %02d:%02d:%02d",&year,&month,&day,&hour,&minute,&second);
    snprintf(time_out,sizeof(time_out),"%02d:%02d:%02d",hour,minute,second);
    
    memset(octimebuf,0,sizeof(octimebuf));
    strncpy(octimebuf,data_out,sizeof(octimebuf));
//    printf("octimebuf = %s\r\n",octimebuf);
    
    memset(viewtimebuf,0,sizeof(viewtimebuf));
    strncpy(viewtimebuf,time_out,sizeof(viewtimebuf));
//    printf("viewtimebuf = %s\r\n",viewtimebuf);
    
    time_flag = 1;

}




/*******************************************************************************
* 名    称：BC35_connect_reqinfo_handle
* 功    能：连接成功是接收数据处理
* 参数说明：reqinfo：接收的信息
			bc35g：bc35结构数据
* 返 回 值：无
* 说    明：无
* 调用方法：内部使用
*******************************************************************************/
void BC35_connect_reqinfo_handle(char * reqinfo,BC35G_PACKAGE* bc35g)
{   
         if(strstr((char*)reqinfo,"+NNMI"))// 主动接收到平台下发的命令
		 {
             //add 偏移
             ptl_Receive_OCData_Handle((char *)(reqinfo+10));
		 }
         else if(strstr((char*)reqinfo,"+CEREG:4"))// 网络掉线
         {
             _BC35G.State = BC35G_Drop; 
             DEBUG_PRINT(MODULE_BC35,"%s","Net is Drop!\r\n");
             
         }
         
         else if(strstr((char*)reqinfo,"+CCLK"))// BC35 获取时间
		 {
             char* time_index = strstr((char*)reqinfo,"+CCLK:")+ 6;
             time_analyse(time_index);

		 }
         
         else if(strstr((char*)reqinfo,"+CEREG:1"))// 网络恢复
         {
             _BC35G.State = BC35G_Connected; 
             _BC35G.feed_time = 15000;
             DEBUG_PRINT(MODULE_BC35,"%s","Net is resumes Conneceted!");
         } 
		 else if(strstr((char*)reqinfo,"ERROR")) 
		 {  				 		  
		      		  
		 }           
          else if(strstr((char*)reqinfo,"+CSQ:"))
         {  
             sscanf((char *)reqinfo,"\r\n+CSQ:%d,",&bc35g->UECSQ);
             if(bc35g->UECSQ >31)
             {
                DEBUG_PRINT(MODULE_BC35,"CSQ=%d,signal error..\r\n",bc35g->UECSQ); 
             }
         }		  
         if(strcmp((char*)reqinfo,"\r\nOK\r\n")==0)// 发送数据成功
		 {
             ptl_SendSuccess_Deal(&g_ptl_SendTack);
             bc35g->feed_time=15000; // 通讯正常时喂狗15s
		 }         
}

/*******************************************************************************
* 名    称：BC35_Printf_NUESTATS
* 功    能：BC35 打印网络状态
* 参数说明：*p_rec_buf：接收的buf
* 返 回 值：无
* 说    明：无
*******************************************************************************/
void BC35_Printf_NUESTATS(char *p_rec_buf)
{
    char *p = NULL;
	
    p = strstr(p_rec_buf, "Signal power:");
    if(p != NULL)
    {
        printf("%s", p);
        return;
    }
    p = strstr(p_rec_buf, "Total power:");
    if(p != NULL)
    {
        printf("%s", p);
        return;
    }

    p = strstr(p_rec_buf, "TX power:");
    if(p != NULL)
    {
        printf("%s", p);
        return;
    }

    p = strstr(p_rec_buf, "Cell ID:");
    if(p != NULL)
    {
        printf("%s", p);
        return;
    }
    p = strstr(p_rec_buf, "ECL");
    if(p != NULL)
    {
        printf("%s", p);
        return;
    }
    p = strstr(p_rec_buf, "SNR");
    if(p != NULL)
    {
        printf("%s", p);
        return;
    }
    p = strstr(p_rec_buf, "EARFCN");
    if(p != NULL)
    {
        printf("%s", p);
        return;
    }

    p = strstr(p_rec_buf, "PCI");
    if(p != NULL)
    {
        printf("%s", p);
        return;
    }
    p = strstr(p_rec_buf, "RSRQ");
    if(p != NULL)
    {
        printf("%s", p);
        return;
    }
}

/*******************************************************************************
* 名    称：BC35G_Rec_Data_Handle
* 功    能：BC35 接收数据处理
* 参数说明：*bc35g：bc35数据结构体
* 返 回 值：无
* 说    明：无
*******************************************************************************/
void BC35G_Rec_Data_Handle(BC35G_PACKAGE* bc35g)
{
    if (stUart_BC35G.ucRecState == BC35UART_REC_OVER) 	 
    {	
     stUart_BC35G.ucRecState = BC35UART_REC_DEIL ;
                 
     BC35_connect_reqinfo_handle(stUart_BC35G.RecInfo,bc35g);
	 BC35_Printf_NUESTATS(stUart_BC35G.RecInfo);
     memset(stUart_BC35G.RecInfo,0,200);      
    }      
}

/*******************************************************************************
* 名    称：BC35G_Config_judge
* 功    能：对BC35G初始化时返回的数据进行判断
* 参数说明：*bc35g:bc35数据结构体
* 返 回 值：无
* 说    明：无
* 调用方法：内部使用
*******************************************************************************/
void BC35G_Config_judge(BC35G_PACKAGE* bc35g)
{   
    unsigned char state = 0,setstate = 0;
    unsigned int feedtime = 0;	
    
	state = bc35g->State;	
    
	if(state == BC35G_Connected)	// 如果BC35G成功联网后则不进行该函数处理
		return;
	/* 将结构体的值赋值给函数局部变量 */
	feedtime = bc35g->feed_time;
	setstate = bc35g->set_state;

	/* 对BC35G模块设置AT指令时返回的数据进行判断处理 */
	if(state == BC35G_Setting)
	{ 
        /* 
           第一种情况为：
           feed_time=0,即模块第一条AT指令无返回数据，判定模块未或者串口通讯不通 
        */        
	    if((!feedtime)&&(setstate<1))
		{ 			  
			DEBUG_PRINT(MODULE_BC35,"%s","BC35G No data return\r\n");
			
			/* 修改结构体的值 */
		    bc35g->set_state = 0;
			bc35g->feed_time = 2000;
			bc35g->State = BC35G_Starting; // 重新开始初始化

            bc35g->power_state=POWER_OFF;
            BC35G_Err.ErrFunc = RestartPWR;
            g_ptl_SendTack.TackNo = 1;
            g_ptl_SendTack.TackState = SendTack_Wait;
            g_ptl_SendTack.TackName[0] = SendTask1;	

            NET_LED_OFF; // 关闭LED
			return;
	     }
         /* 
             第二种情况为：
             串口有接受到数据，则需要对接受到的数据进行判断
         */ 
	     if (stUart_BC35G.ucRecState == BC35UART_REC_OVER) 	 
		 {	
              stUart_BC35G.ucRecState = BC35UART_REC_DEIL ;
                 
              bc35g->feed_time = 2000;	// 给模块喂狗5s
              NET_LED_OFF; // 关闭LED
              BC35_Setting_reqinfo_handle(stUart_BC35G.RecInfo,bc35g);
              memset(stUart_BC35G.RecInfo,0,200);
              return; 
	     }  
	 }	
     else if(state == BC35G_Setted_over)
     {
         /* 
             如果最后一条AT指令发送后，没有数据返回则重新发送该AT指令
         */          
         if(!feedtime&&(setstate >= 11)) 
		 {		  
			  DEBUG_PRINT(MODULE_BC35,"%s","\r\nAT Commond no return \r\n");
			  /*修改结构体的值*/
			  bc35g->set_state=0;
			  bc35g->set_time=500;				
			  bc35g->feed_time=2000;

              bc35g->power_state=POWER_ON;
              BC35G_Err.ErrFunc = RestartPWR;
              bc35g->State = BC35G_Starting;
              g_ptl_SendTack.TackNo = 1;
              g_ptl_SendTack.TackState = SendTack_Wait;
              g_ptl_SendTack.TackName[0] = SendTask1;	

              NET_LED_OFF;
			  return;
	      }	
         /* 
             当AT指令完成设置后，对模块网络状态进行判断
         */         
  
		  bc35g->feed_time=15000;
          NET_LED_OFF; // 关闭LED        
 		  BC35_Lastset_reqinfo_handle(stUart_BC35G.RecInfo,bc35g);
           memset(stUart_BC35G.RecInfo,0,200);
	 }  		  
}

/*******************************************************************************
* 名    称：BC35G_ErrorDeal
* 功    能：当BC35G第一次联网成功后，出现断网的情况处理
* 参数说明：*bc35g:bc35数据结构体
* 返 回 值：无
* 说    明：无
* 调用方法：内部使用
*******************************************************************************/
void BC35G_ErrorDeal(BC35G_PACKAGE* bc35g)
{ 
	if(bc35g->First_Connected_Flag == 1) // 初始化联网成功后的异常处理
	{	
	    if(bc35g->State == BC35G_Connected )	
		{
            if((!bc35g->feed_time))
            {				
                bc35g->State=BC35G_Setting;      // 重新开始配置BC35G模块
                bc35g->set_state=0;
                bc35g->set_time=2000;
                bc35g->feed_time=5000;
                bc35g->power_state=POWER_ON;
                BC35G_Err.ErrFunc = RestartPWR;
                bc35g->State = BC35G_Starting;
                
                g_ptl_SendTack.TackNo = 1;
                g_ptl_SendTack.TackState = SendTack_Wait;
                g_ptl_SendTack.TackName[0] = SendTask1;			
                DEBUG_PRINT(MODULE_BC35,"%s","\r\nNet is Error...\r\n");
            }	
        }        
	    else if(bc35g->State == BC35G_Drop) // 网络掉线，查询信号值和网络状态
        {
            //ptl_Heartbeat_Handle函数执行查询的工作
            DEBUG_PRINT(MODULE_BC35,"%s","\r\nNet is Drop...\r\n");
            
            //掉线一段时间后，执行重启
            bc35g->State=BC35G_Starting;      // 重新开始配置BC35G模块
            bc35g->set_state=0;
            bc35g->set_time=2000;
            bc35g->feed_time=5000;
            bc35g->power_state=POWER_ON;
            BC35G_Err.ErrFunc = RestartPWR;
            g_ptl_SendTack.TackNo = 1;
            g_ptl_SendTack.TackState = SendTack_Wait;
            g_ptl_SendTack.TackName[0] = SendTask1;	
        }
	}
}

/*******************************************************************************
* 名    称：BC35G_Handle
* 功    能：BC35数据处理
* 参数说明：无
* 返 回 值：无
* 说    明：初始化模块，数据处理，错误判断
* 调用方法：外部使用
*******************************************************************************/
void BC35G_Handle(void)
{
    /* BC35G模块初始化配置 */
    BC35G_UE_Config(&_BC35G);
    /* BC35G初始化状态判断 */
    BC35G_Config_judge(&_BC35G); 
    /* BC35G网络错误处理 */
    BC35G_ErrorDeal(&_BC35G);
    /* BC35G数据接受处理 */
    BC35G_Rec_Data_Handle(&_BC35G);
}

void BC35G_Write_Data(char* data,int len)
{


	uart_write(uart3,len,(unsigned char *)data);
}

void BC35G_Read_Data(char* data,int len)
{
	if(len>sizeof(stUart_BC35G.RecInfo))
	{

		return;	
	}
	memcpy(stUart_BC35G.RecInfo, data, len);  //串口3接收到的数据复制到BC35数据接收组
	stUart_BC35G.ucRecState = BC35UART_REC_OVER;            //BC35状态--接收数据完成
}

BC35GState_Value Get_BC35G_State()
{
	return _BC35G.State;
}

int Is_BC35G_Connected()
{
	if( _BC35G.State == BC35G_Connected)
    {
        return 1;
    }
    else
    {	
    	return 0;
    }
}

void BC35_Init(void)
{
	gpio_module_open(BC35G_SW);
    uart_open(uart3, 9600); //打开串口3，设置波特率9600
}

void BC35_Finger_Get_Time(void)
{
    _BC35G_Get_Time();
}

void _BC35G_Get_Time(void)
{
        char time_buff[] = "AT+CCLK?\r\n";
        BC35G_Write_Data(time_buff,strlen(time_buff));
}

#endif

