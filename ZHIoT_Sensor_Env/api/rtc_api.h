#ifndef __rtc_api_H
#define __rtc_api_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

void rtc_open(void);
void rtc_close(void);
void rtc_gettime(char *ptimebuffer);
void rtc_settime(char *ptimebuffer);

void rtc_error_handler(void);

#ifdef __cplusplus
}
#endif
#endif /*__rtc_api_H */

