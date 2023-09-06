#ifndef software_usart__
#define software_usart__
#include "stm32f10x.h"
#define BAUDRATE 104
//!  3 -> 256000  8->115200
void Software_USART_TXD(u8 Data);
void USART_Send(u8 *buf, u8 len);
void Software_USART_IOConfig(void);
void TIM7_Int_Init(u16 arr, u16 psc);
#endif
