#ifndef WIRLESS__
#define WIRLESS__
#include "stm32f10x.h"


// extern struct Buff BUFF;
// extern struct Buff USART1_buffer;

void ESP8266_Enable(void);
u8 ESP_Check(u8 Timeout);
void Wriless_Serial_port_Initial(u8 a);
u8 Check_Connect(u8 sign, int time);
void RSR_ESP(void);
void ESP_Init(u8 a, int time);
u8 ESP_Check_OK(u8 Timeout);
#endif

