/*******************************************************************************
* @文件名        : view_task
* @描述          : 显示任务
* @作者          : 物联网项目组
* @日期          : 2023/04/01
* @版本          : V2.0.0
--------------------------------------------------------------------------------
  功能包括：
  x:0~240
  y:0~320

   -----x------
  |            |
  |            |
  |            |
  y            y
  |            |
  |            |
  |            |
   ------x-----

  分页显示：
  页面1：显示各模块状态
  页面2：调试打印信息，展现过程
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "lcd.h"
#include "gui.h"
#include "stdio.h"
#include "usart_api.h"
#include "rtc_api.h"
#include "task_inter.h"
#include "net.h"
#include "key.h"
#include "view_task.h"
#include "error.h"

#include "icon_farm_fence_on.h"
#include "icon_farm_fence_off.h"

/******************************内部参数宏定义***********************************/
#define   PRINTF_COLOR     BLACK    //打印输出颜色
#define   PRINTF_Y         150      //打印数据Y轴位置
#define   PRINTF_X         0        //打印数据X轴位置
#define   Appoint_Y        55       //模块状态Y轴位置
#define   Appoint_X        0        //模块状态X轴位置
#define   Appoint_COLOR    BLACK    //打印输出颜色

UINT32 g_ViewTskHandle;

/******************************内部变量定义************************************/
uint16_t ColorTab[5] = {RED, GREEN, BLUE, YELLOW, BRED}; //定义颜色数组
char  Str_Code[20];   //显示屏显示缓冲数组

Key_Struct key1 = {1};
Key_Struct key2 = {2};
Key_Struct key3 = {3};
Key_Struct key4 = {4};

/******************************内部函数声明************************************/
static void Screen_Init_Display(void);     //屏幕显示初始化
static void Screen_Appoint_Display(uint8_t line, char *str);  //APP显示初始化

static void QRCodeview(void);   //二维码初始化
static void Net_View(void);     //net状态显示

/*******************************************************************************
* 名    称：keytask(void)
* 功    能：按键处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void keytask(void)
{
    get_key_value(&key1, Key1_Read_Value);
    Key_Handle(&key1);
    get_key_value(&key2, Key2_Read_Value);
    Key_Handle(&key2);
    get_key_value(&key3, Key3_Read_Value);
    Key_Handle(&key3);
    get_key_value(&key4, Key4_Read_Value);
    Key_Handle(&key4);
}

/*******************************************************************************
* 名    称：Screen_Init_Display(void)
* 功    能：初始化屏幕显示函数
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部函数
*******************************************************************************/
static void Screen_Init_Display(void)
{
    LCD_Init();	   //液晶屏初始化
    LCD_Clear(WHITE);//清全屏白色
    LCD_Fill(0, lcddev.height - 20, lcddev.width, lcddev.height, BLUE);
    LCD_Fill(0, 0, lcddev.width, 20, BLUE);
    Gui_StrCenter(0, 2, BLUE, BLUE, " ", 16, 0);
    Gui_StrCenter(0, 27, BLACK, WHITE, "电子围栏场景实训", 16, 1);
    Screen_Appoint_Display(1, "STATUS:");
    Screen_Appoint_Display(2, "ID:");

    Gui_StrCenter(0, lcddev.height - 18, BLUE, BLUE, " ", 16, 0);
    Show_Str(Appoint_X, Appoint_Y, Appoint_COLOR, YELLOW, "模块信息:", 16, 1);

}

/*******************************************************************************
* 名    称：Screen_Appoint_Display(uint8_t line,char *str)
* 功    能：显示屏在某行显示字符串
* 参数说明：line：显示起始行
            str：显示字符串的指针
* 返 回 值：无
* 说    明：无
* 调用方法：内部函数
*******************************************************************************/
static void Screen_Appoint_Display(uint8_t line, char *str)
{
    if(line >= 7)
    {
        line = 7;
    }
    if(line == 0)
    {
        line = 1;
    }
    LCD_Fill(Appoint_X, Appoint_Y + 16 * line, 150, Appoint_Y + 16 * (line + 1), WHITE);
    Show_Str(Appoint_X, Appoint_Y + 16 * line, Appoint_COLOR, YELLOW, (uint8_t *)str, 16, 1);
}


/*******************************************************************************
* 名    称：QRCodeview(void)
* 功    能：二维码显示函数
* 参数说明：无
* 返 回 值：无
* 说    明：当得到IMEI号，生成二维码
* 调用方法：内部函数
*******************************************************************************/
static void QRCodeview(void)
{
    if(qrcodeflag == 1)
    {
        return;
    }

    char strbuffer[sizeof(QR_STR_HEAD) + 20] = {0};

    qrcodeflag = 1;
    strcat(strbuffer, QR_STR_HEAD);
    strcat(strbuffer, (char *)Get_Net_Module_ID());
    QR_CODE_Display((uint8_t *)strbuffer);
}

static void Fence_View()
{
	if(Scene.state == 1)
    {
        Scene.req_state = 1; //其他线程已经接收到state的变化，并已做出应答响应
        if(FarmView.clr_screen_status == 1)
        {
            FarmView.clr_screen_status = 0;
            LCD_Fill(0, 166, 239, 300, WHITE);        //刷新显示
        }

        LCD_Fill(PRINTF_X, PRINTF_Y, PRINTF_X + 80, PRINTF_Y + 16, WHITE);
        Show_Str(PRINTF_X, PRINTF_Y, PRINTF_COLOR, YELLOW, "状态显示:", 16, 1);
        
        if(FarmView.EnclosureInput == 1)
        {
            LCD_draw_bmp(96, 196, 48, gImage_icon_farm_fence_on);
        }
        else
        {
            LCD_draw_bmp(96, 196, 48, gImage_icon_farm_fence_off);

        }


        if(FarmView.EnclosureInput == 1)
        {
            snprintf(Str_Code, sizeof(Str_Code), "有人闯入");
            Screen_Appoint_Display(4, Str_Code);
        }
        else if(FarmView.EnclosureInput == 0)
        {
            snprintf(Str_Code, sizeof(Str_Code), "正常");
            Screen_Appoint_Display(4, Str_Code);
        }
    }
    else
    {
        Scene.req_state = 0;
    }
}
/*******************************************************************************
* 名    称：Net_View(void)
* 功    能：net状态显示函数
* 参数说明：无
* 返 回 值：无
* 说    明：显示网络状态、信号值、IEMI号
* 调用方法：内部函数
*******************************************************************************/
static void Net_View(void)
{
    static  uint32_t tickstart;
	LOS_TaskLock();
    if((HAL_GetTick() - tickstart) >= 2000)
    {
        tickstart = HAL_GetTick();
        
        Screen_Appoint_Display(1, (char*)FarmView.Line1);
        Screen_Appoint_Display(2, (char*)FarmView.Line2);
        Screen_Appoint_Display(3, (char*)FarmView.Line3);


    }
	LOS_TaskUnlock();
}

/*******************************************************************************
* 名    称：task_view_init(void)
* 功    能：显示初始化函数
* 参数说明：无
* 返 回 值：无
* 说    明：外部调用接口函数
* 调用方法：外部函数
*******************************************************************************/
void task_view_init(void)
{
    /*显示屏初始化界面*/
    Screen_Init_Display();
    KEY_Init();
    Scene.state = 1; //初始化状态为1
}

/*******************************************************************************
* 名    称： view_task(VOID)
* 功    能：界面处理任务函数
* 参数说明：无
* 返 回 值：无
* 说    明：界面任务处理函数
* 调用方法：内部函数
*******************************************************************************/
VOID view_task(VOID)
{
    UINT32 uwRet = LOS_OK;
    while(1)
    {
        Fence_View();

        Net_View();
		Error_Msg_View();
        keytask();
        uwRet = LOS_TaskDelay(10);
        if(uwRet != LOS_OK)
        {
            return;
        }
    }
}

/*******************************************************************************
* 名    称：creat_view_task(VOID)
* 功    能：创建view_task任务
* 参数说明：无
* 返 回 值：uwRet：LOS_OK 表示创建任务成功；否则表示失败
* 说    明：外部调用
* 调用方法：在main函数中调用
*******************************************************************************/

UINT32 creat_view_task(VOID)
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 1;
    task_init_param.pcName = "view_task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)view_task;
    task_init_param.uwStackSize = 0xA00;
    uwRet = LOS_TaskCreate(&g_ViewTskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}


