/*******************************************************************************
* @文件名        : view_task
* @描述          : 显示任务
* @作者          : 物联网项目组
* @日期          : 2024/05/01
* @版本          : V1.0.0
--------------------------------------------------------------------------------
  功能包括：
  1、显示大气压、温湿度、光照强度、二氧化碳
  2、显示灯、风扇的输出状态
  3、显示联网模块状态
  屏幕坐标：x:0~239  y:0~319
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "lcd.h"
#include "gui.h"
#include "usart_api.h"
#include "rtc_api.h"
#include "task_inter.h"
#include "net.h"
#include "key.h"
#include "view_task.h"
#include "error.h"
#include "env_sensor.h"
#include "env_control.h"

/******************************内部参数宏定义***********************************/
#define   SECTION_COLOR    BLACK
#define   SECTION_BG       YELLOW

#define   ROW_SENSOR_1     66
#define   ROW_SENSOR_2     83
#define   ROW_SENSOR_3     100
#define   ROW_SENSOR_4     117
#define   ROW_SENSOR_5     134
#define   ROW_DEVICE_1     172
#define   ROW_DEVICE_2     192
#define   ROW_NET_1        236
#define   ROW_NET_2        256
#define   ROW_NET_3        276

#define   ROW_MAX          10

UINT32 g_ViewTskHandle;

/******************************内部变量定义************************************/
char  Str_Code[24];   //显示屏显示缓冲数组

Key_Struct key1 = {1};
Key_Struct key2 = {2};
Key_Struct key3 = {3};
Key_Struct key4 = {4};

static char LastLine[ROW_MAX][24];   //上一次显示的内容，避免重复刷新造成闪烁

/******************************内部函数声明************************************/
static void Screen_Init_Display(void);     //屏幕显示初始化

/*******************************************************************************
* 名    称：Env_Fmt_One_Decimal
* 功    能：把浮点数格式化成 1 位小数的字符串
* 参数说明：out：输出缓冲   size：缓冲长度   val：待转换的数值
* 返 回 值：无
* 说    明：工程使用 MicroLIB，不支持 %f 格式化
* 调用方法：内部函数
*******************************************************************************/
static void Env_Fmt_One_Decimal(char *out, uint16_t size, float val)
{
    int32_t v;
    int32_t a;

    v = (val >= 0.0f) ? (int32_t)(val * 10.0f + 0.5f) : (int32_t)(val * 10.0f - 0.5f);
    a = (v >= 0) ? v : -v;

    snprintf(out, size, "%s%d.%d", (v < 0) ? "-" : "", (int)(a / 10), (int)(a % 10));
}

/*******************************************************************************
* 名    称：Screen_Show_Row
* 功    能：显示屏某一行显示字符串（内容不变时不重复刷新）
* 参数说明：row：行编号   y：Y 轴坐标   str：显示字符串
* 返 回 值：无
* 说    明：无
* 调用方法：内部函数
*******************************************************************************/
static void Screen_Show_Row(uint8_t row, uint16_t y, const char *str)
{
    if(row >= ROW_MAX)
    {
        return;
    }

    if(strcmp(LastLine[row], str) == 0)
    {
        return;
    }

    memset(LastLine[row], 0, sizeof(LastLine[row]));
    strncpy(LastLine[row], str, sizeof(LastLine[row]) - 1);

    LCD_Fill(0, y, lcddev.width - 1, y + 16, WHITE);
    Show_Str(0, y, BLACK, WHITE, (uint8_t *)str, 16, 1);
}

/*******************************************************************************
* 名    称：Screen_Section
* 功    能：显示分组标题
* 参数说明：y：Y 轴坐标   str：标题字符串
* 返 回 值：无
* 说    明：无
* 调用方法：内部函数
*******************************************************************************/
static void Screen_Section(uint16_t y, char *str)
{
    LCD_Fill(0, y, lcddev.width - 1, y + 16, SECTION_BG);
    Show_Str(0, y, SECTION_COLOR, SECTION_BG, (uint8_t *)str, 16, 1);
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
    LCD_Init();	                       //液晶屏初始化
    LCD_Clear(WHITE);                  //清全屏白色
    LCD_Fill(0, lcddev.height - 20, lcddev.width, lcddev.height, BLUE);
    LCD_Fill(0, 0, lcddev.width, 20, BLUE);
    Gui_StrCenter(0, 22, BLACK, WHITE, "环境监测场景实训", 16, 1);

    Screen_Section(48, "传感器数据:");
    Screen_Section(152, "设备状态:");
    Screen_Section(216, "模块信息:");

    memset(LastLine, 0, sizeof(LastLine));
}

/*******************************************************************************
* 名    称：Sensor_View(void)
* 功    能：传感器数据显示
* 参数说明：无
* 返 回 值：无
* 说    明：500ms 刷新一次
* 调用方法：内部函数
*******************************************************************************/
static void Sensor_View(void)
{
    static uint32_t tickstart = 0;

    if((HAL_GetTick() - tickstart) < 500)
    {
        return;
    }
    tickstart = HAL_GetTick();

    if(g_env_data.sht30_ok)
    {
        char val[12];

        Env_Fmt_One_Decimal(val, sizeof(val), g_env_data.temperature);
        snprintf(Str_Code, sizeof(Str_Code), "温度: %s C", val);
        Screen_Show_Row(0, ROW_SENSOR_1, Str_Code);

        Env_Fmt_One_Decimal(val, sizeof(val), g_env_data.humidity);
        snprintf(Str_Code, sizeof(Str_Code), "湿度: %s %%RH", val);
        Screen_Show_Row(1, ROW_SENSOR_2, Str_Code);
    }
    else
    {
        Screen_Show_Row(0, ROW_SENSOR_1, "温度: ---");
        Screen_Show_Row(1, ROW_SENSOR_2, "湿度: ---");
    }

    if(g_env_data.cps121_ok)
    {
        snprintf(Str_Code, sizeof(Str_Code), "气压: %d hPa", (int)(g_env_data.pressure + 0.5f));
    }
    else
    {
        snprintf(Str_Code, sizeof(Str_Code), "气压: ---");
    }
    Screen_Show_Row(2, ROW_SENSOR_3, Str_Code);

    if(g_env_data.bh1750_ok)
    {
        snprintf(Str_Code, sizeof(Str_Code), "光照: %d lx", (int)g_env_data.light);
    }
    else
    {
        snprintf(Str_Code, sizeof(Str_Code), "光照: ---");
    }
    Screen_Show_Row(3, ROW_SENSOR_4, Str_Code);

    if(g_env_data.sgp30_ok)
    {
        snprintf(Str_Code, sizeof(Str_Code), "CO2: %d ppm", (int)g_env_data.co2);
    }
    else
    {
        snprintf(Str_Code, sizeof(Str_Code), "CO2: ---");
    }
    Screen_Show_Row(4, ROW_SENSOR_5, Str_Code);
}

/*******************************************************************************
* 名    称：Device_View(void)
* 功    能：灯、风扇状态显示
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部函数
*******************************************************************************/
static void Device_View(void)
{
    snprintf(Str_Code, sizeof(Str_Code), "灯: %s  档位:%d",
             Env_Lamp_Get_State() ? "开" : "关", (int)Env_Lamp_Get_Level());
    Screen_Show_Row(5, ROW_DEVICE_1, Str_Code);

    snprintf(Str_Code, sizeof(Str_Code), "风扇: %s",
             Env_Fan_Get_State() ? "开" : "关");
    Screen_Show_Row(6, ROW_DEVICE_2, Str_Code);
}

/*******************************************************************************
* 名    称：Net_View(void)
* 功    能：net状态显示函数
* 参数说明：无
* 返 回 值：无
* 说    明：显示网络状态、ID 号
* 调用方法：内部函数
*******************************************************************************/
static void Net_View(void)
{
    static uint32_t tickstart = 0;

    if((HAL_GetTick() - tickstart) < 2000)
    {
        return;
    }
    tickstart = HAL_GetTick();

    Screen_Show_Row(7, ROW_NET_1, (char *)EnvView.Line1);
    Screen_Show_Row(8, ROW_NET_2, (char *)EnvView.Line2);
    Screen_Show_Row(9, ROW_NET_3, (char *)EnvView.Line3);}

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
        Sensor_View();
        Device_View();
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
