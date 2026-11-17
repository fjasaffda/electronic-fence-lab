#ifndef __ERROR_H__
#define __ERROR_H__


#define ERR_MQ_MAX_NUM           		4

typedef enum
{
    ERROR_NONE = 0,
    ERROR_HOME_SERVO = 1,
    ERROR_FARM_USART = 2,
    ERROR_LIGHT_I2C = 3,
    ERROR_TEMP_I2C = 4,
    ERROR_CO2_I2C = 5,

    ERROR_LIGHT_DATA = 7,
    ERROR_TEMP_DATA = 8,
    ERROR_CO2_DATA = 9,
}ERROR_MSG_VALUE; 


void Error_Msg_Init(void);
void Error_Msg_Send(ERROR_MSG_VALUE msg);
void Error_Msg_Handle(void);
void Error_Msg_View(void);

#endif
