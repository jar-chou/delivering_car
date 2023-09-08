#ifndef Encoder__
#define Encoder__

#include "stm32f4xx.h"

void TIMX_Delay_Init(uint32_t ClOCK, uint16_t TIM_PERIOD, uint16_t TIM_PRESCALER, TIM_TypeDef *TIMX);
void Encoder_Init_TIM2(void);
void Encoder_Init(void);
int Read_Encoder(u8 TIMX);
#endif
