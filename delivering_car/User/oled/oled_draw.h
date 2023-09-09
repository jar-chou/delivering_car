#ifndef OLED_DRAW_H
#define OLED_DRAW_H
#include "stdint.h"

void DrawChar(int x, int y, unsigned char c);
void DrawString(int x, int y,char *str);
void DrawNum(unsigned char x,unsigned char y,unsigned int num,unsigned char len);
void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t *cn);



#endif

