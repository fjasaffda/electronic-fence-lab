
#ifndef __pwm_api_H
#define __pwm_api_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
    

void pwm_open(uint8_t duty);
void pwm_close(void);
void pwm_set(uint8_t duty);

void pwm_error_handle(void);

#ifdef __cplusplus
}
#endif
#endif /*__pwm_api_H */
