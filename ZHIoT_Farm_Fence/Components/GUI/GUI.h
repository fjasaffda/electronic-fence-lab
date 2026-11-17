#ifndef __GUI_H__
#define __GUI_H__

#include "stm32l4xx_hal.h"

#define QR_STR_HEAD       "https://iot.zj-huawei.com/wx/#/device?nodeId="   //二维码生成数据

/*****************************外部函数声明**************************************/
extern void show_printflog(void);
extern void RingBuff_Init(void);
extern void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color);
extern void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
extern void LCD_DrawFillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
extern void Draw_Triangel(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2,
                          uint16_t y2);
extern void Fill_Triangel(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2,
                          uint16_t y2);
extern void LCD_ShowChar(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t num,
                         uint8_t size, uint8_t mode);
extern void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size);
extern void LCD_ShowString(uint16_t x, uint16_t y, uint8_t size, uint8_t *p, uint8_t mode);
extern void GUI_DrawFont16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,
                           uint8_t mode);
extern void GUI_DrawFont24(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,
                           uint8_t mode);
extern void GUI_DrawFont32(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,
                           uint8_t mode);
extern void Show_Str(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str, uint8_t size,
                     uint8_t mode);
extern void Gui_Drawbmp16(uint16_t x, uint16_t y, const unsigned char *p);
extern void gui_circle(int xc, int yc, uint16_t c, int r, int fill);
extern void Gui_StrCenter(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str,
                          uint8_t size, uint8_t mode);

extern void Screen_Init_Display(void);
extern void Gui_Appoint_Display(uint8_t line, char *str);
extern void Gui_Printf_Display(uint8_t *str);
extern void QR_CODE_Display(uint8_t  *qrcode_data);
extern void QR_CODE_Display_Carid(uint8_t  *qrcode_data);
#endif

