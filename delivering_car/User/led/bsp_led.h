

#ifndef __LED_H
#define	__LED_H


#include "stm32f10x.h"

#define keystart Buzzer_ONE

#define check_keystart GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_8)

void LED_GPIO_Config(void);
void KEY_ONE(void);
void Buzzer_TWO(void);
void Buzzer_ONE(void);
#endif /* __LED_H */
