
#ifndef __key_H
#define __key_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "stdint.h"
#include "gpio.h"

#define K1      1u
#define K2      2u
#define K3      3u
#define K4      4u

#define Key1_Read_Value     HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)//gpio_pin_read(GPIO_A,KEY1_Pin)
#define Key2_Read_Value     HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)
#define Key3_Read_Value     HAL_GPIO_ReadPin(KEY3_GPIO_Port,KEY3_Pin)
#define Key4_Read_Value     HAL_GPIO_ReadPin(KEY4_GPIO_Port,KEY4_Pin)

typedef enum 
{
    key_idle=0,
    key_trigger_start,
    key_trigger_keep,
    key_trigger_end,
}Key_State;

typedef struct
{
    uint8_t keynum;
    uint8_t keyvalue;
    uint8_t keycount;
    Key_State keystate;
}Key_Struct;

void KEY_Init(void);
void get_key_value(Key_Struct * keynum,uint8_t keyvalue);
void Key_Handle(Key_Struct * keynum);

void key1_start_handle(void);
void key1_keep_handle(void);
void key1_end_handle(void);
void key2_start_handle(void);
void key2_keep_handle(void);
void key2_end_handle(void);
void key3_start_handle(void);
void key3_keep_handle(void);
void key3_end_handle(void);
void key4_start_handle(void);
void key4_keep_handle(void);
void key4_end_handle(void);

#ifdef __cplusplus
}
#endif
#endif /*__key_H */

