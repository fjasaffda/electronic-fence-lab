
#ifndef __can_api_H
#define __can_api_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
    


void adc1_open(void);
void adc1_close(void);
void adc1_getvalue(float * Voltage);


void adc_error_handle(void);

#ifdef __cplusplus
}
#endif
#endif /*__ can_api_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


