#ifndef __log_H
#define __log_H
#ifdef __cplusplus
 extern "C" {
#endif

 #include "stm32l4xx_hal.h"
 #include <stdio.h>
 
#define MODULE_TFT       "[TFT-"
#define MODULE_KEY       "[KEY-"
#define MODULE_E2PROOM   "[W25qxx-"
#define MODULE_RFID      "[RC522-"
#define MODULE_TEMP      "[ds18b20-"
#define MODULE_BC35      "[BC35-"
#define MODULE_OC        "[OC-"
#define MODULE_WIFI      "[WIFI-"
#define MODULE_AS608     "[AS608-"

#define ENABLE_DEBUG_SYS       1
#define ENABLE_DEBUG_DRIVER    1
#define ENBALE_DEBUG_BC35G     1

extern uint8_t debugflag;

#define USE_DEBUG_PRINTF
#ifdef USE_DEBUG_PRINTF
#define DEBUG_PRINT(module,fmt,args...)   do{if(debugflag==1)\
                                             printf(module"]"fmt,args);} while(0)
#else
#define DEBUG_PRINT(fmt...)
#endif

//#define USE_LOG_PRINTF
#ifdef USE_LOG_PRINTF
#define LOG_PRINT(module,fmt,args...)   do{if(debugflag==1)\
                                             printf(module"time:%d]"fmt,HAL_GetTick(),args);} while(0)
#else
#define LOG_PRINT(fmt...)
#endif											 
#ifdef __cplusplus
}
#endif
                                       
#endif /*__ usart_api_H */
