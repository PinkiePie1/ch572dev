#ifndef __MINIGUI_H_
#define __MINIGUI_H_
//上面这些define是为了防止头文件被重复引用

#include "CH57x_common.h"
#include "fonts.h"
#include <stdarg.h>
#include <stdio.h>

#define BLACK 0xFF//黑色
#define WHITE 0x00//白色

void paint_SetImageCache(uint8_t *imagePtr);
void drawLine(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint8_t color);
void drawRect(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint8_t color);
void fillRect(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint8_t color);
void fastFill(uint16_t x, uint16_t y, uint16_t xblock, uint16_t yblock, uint8_t color);
void FastImg(uint16_t xStart, uint16_t xEnd,const char *imgDat);
void fastDrawChar(uint16_t xStart, uint16_t yStart, char chara, const uint8_t *font);
void fastDrawString(uint16_t xStart, uint16_t yStart,char *stringToPrint, const uint8_t *font);
void drawChar(uint16_t xStart, uint16_t yStart, char charToPrint, const char *font, uint8_t color);
void drawStr(uint16_t xStart, uint16_t yStart,char *stringToPrint, const char *font, uint8_t color);
void fastRect(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint8_t color);
void EPD_Printf(uint16_t xStart, uint16_t yStart, const char *font, uint8_t color, const char *format, ...);

#endif
