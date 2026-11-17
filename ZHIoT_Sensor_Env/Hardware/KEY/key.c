/*******************************************************************************
* @文件名        : key
* @描述          : 按键处理函数
* @作者          : 物联网项目组
* @日期          : 2023/04/01
* @版本          : V2.0.0
--------------------------------------------------------------------------------
  按键存在抖动，当识别到为有效按键触发后：
  1、先执行一次单次触发按键功能，keyx_start_handle
  2、随后按键一直保存触发状态，则执行按键持续功能，keyx_keep_handle
  3、当按键返回正常状态后，则执行按键结束功能。keyx_end_handle
*******************************************************************************/

#include "key.h"
#include "gpio_api.h"

#define KEY_IDLE_VALUE          1u
#define KEY_TRIGGER_VALUE       0u

#define KEY_VALID_COUNT         1    

__weak void key1_start_handle(void)
{
}
__weak void key1_keep_handle(void)
{
}
__weak void key1_end_handle(void)
{
}
__weak void key2_start_handle(void)
{
}
__weak void key2_keep_handle(void)
{
}
__weak void key2_end_handle(void)
{
}
__weak void key3_start_handle(void)
{
}
__weak void key3_keep_handle(void)
{
}
__weak void key3_end_handle(void)
{
}
__weak void key4_start_handle(void)
{
}
__weak void key4_keep_handle(void)
{
}
__weak void key4_end_handle(void)
{
}

/*******************************************************************************
* 名    称：key_start_handle
* 功    能：按键触发处理函数
* 参数说明：keynum：按键编号
* 返 回 值：无
* 说    明：无
*******************************************************************************/
void key_start_handle(uint8_t keynum)
{
    switch (keynum)
    {
        case K1:key1_start_handle();break;
        case K2:key2_start_handle();break;
        case K3:key3_start_handle();break;
        case K4:key4_start_handle();break;
        default : break;
    }
}

/*******************************************************************************
* 名    称：key_keep_handle
* 功    能：按键保持触发处理函数
* 参数说明：keynum：按键编号
* 返 回 值：无
* 说    明：无
*******************************************************************************/
void key_keep_handle(uint8_t keynum)
{
    switch (keynum)
    {
        case K1:key1_keep_handle();break;
        case K2:key2_keep_handle();break;
        case K3:key3_keep_handle();break;
        case K4:key4_keep_handle();break;
        default : break;
    }
}

/*******************************************************************************
* 名    称：key_end_handle
* 功    能：按键触发结束处理函数
* 参数说明：keynum：按键编号
* 返 回 值：无
* 说    明：无
*******************************************************************************/
void key_end_handle(uint8_t keynum)
{
    switch (keynum)
    {
        case K1:key1_end_handle();break;
        case K2:key2_end_handle();break;
        case K3:key3_end_handle();break;
        case K4:key4_end_handle();break;
        default : break;
    }
}

/*******************************************************************************
* 名    称：KEY_Init
* 功    能：按键初始化
* 参数说明：无
* 返 回 值：无
* 说    明：无
*******************************************************************************/
void KEY_Init(void)   
{
    gpio_module_open(KEYS);
}

/*******************************************************************************
* 名    称：get_key_value
* 功    能：获取按键值
* 参数说明：keynum:按键编号结构体
			keyvalue：按键值
* 返 回 值：无
* 说    明：该函数的调用频率，代表按键的灵敏度
*******************************************************************************/
void get_key_value(Key_Struct * keynum,uint8_t keyvalue)
{
    if(keyvalue == KEY_TRIGGER_VALUE)
    {
        if(keynum->keycount ++ > KEY_VALID_COUNT)
        {
            keynum->keyvalue = KEY_TRIGGER_VALUE;
            keynum->keycount =0;
        }
    }
    else 
    {
        keynum->keyvalue = KEY_IDLE_VALUE;
        keynum->keycount =0;        
    }

}

/*******************************************************************************
* 名    称：Key_Handle
* 功    能：按键状态处理
* 参数说明：keynum:按键编号结构体
* 返 回 值：无
* 说    明：该函数默认按键值经过消抖后的稳定值
*******************************************************************************/
void Key_Handle(Key_Struct * keynum)
{
    switch (keynum->keystate)
    {
        case key_idle: 
            if(keynum->keyvalue == KEY_TRIGGER_VALUE) 
            {
                keynum->keystate = key_trigger_start;               
            }break;
        case key_trigger_start:
            key_start_handle(keynum->keynum);
            if(keynum->keyvalue == KEY_TRIGGER_VALUE)
            {                
                keynum->keystate = key_trigger_keep;
            }
            else if(keynum->keyvalue == KEY_IDLE_VALUE)
            {
                keynum->keystate = key_trigger_end;
            }break;
        case key_trigger_keep:
             key_keep_handle(keynum->keynum);            
            if(keynum->keyvalue == KEY_IDLE_VALUE)
            {
                keynum->keystate = key_trigger_end;
            }break;            
        case key_trigger_end:
            key_end_handle(keynum->keynum);
            keynum->keystate = key_idle;
            break;    
    }
}


