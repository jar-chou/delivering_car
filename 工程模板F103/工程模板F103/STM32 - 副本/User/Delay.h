
#ifndef __TIME____

#define __TIME____

#include "stm32f10x.h"

void TIMX_Delay_Init(uint32_t ClOCK, uint16_t TIM_PERIOD, uint16_t TIM_PRESCALER, TIM_TypeDef *TIMX);

void Delayus(u16 xus);
void Delayms(u16 xms);

#endif
