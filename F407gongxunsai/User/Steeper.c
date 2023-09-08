#include "Steeper.h"
#include "FreeRTOS.h"
#include "task.h"
void Steeper_Steep()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOB, GPIO_Pin_0);
}
void Set_Plus(u16 steep)
{
	u16 i;
	for(i = 0 ;i <= steep ;i++)
	{
	if(GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_0))
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_0);
	}
	else{
	GPIO_SetBits(GPIOB, GPIO_Pin_0);
	}
	vTaskDelay(5);
}
}
