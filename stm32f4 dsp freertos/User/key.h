
#ifndef __KEY_H
#define __KEY_H
#include "stm32f4xx.h"

//=======================
//3个板载按键
//E4:(KEY0)按下为低电平
//E3:(KEY1)按下为低电平
//A0:(K_UP)按下为高电平
//=======================

#define RCC_AHB1Periph_KEY0 RCC_AHB1Periph_GPIOE
#define RCC_AHB1Periph_KEY1 RCC_AHB1Periph_GPIOE
#define RCC_AHB1Periph_K_UP RCC_AHB1Periph_GPIOE

#define GPIO_KEY0 GPIOE
#define GPIO_KEY1 GPIOE
#define GPIO_K_UP GPIOE

#define GPIO_Pin_KEY0 GPIO_Pin_15
#define GPIO_Pin_KEY1 GPIO_Pin_10
#define GPIO_Pin_K_UP GPIO_Pin_8

//通过直接操作库函数方式读取IO
#define KEY0 		GPIO_ReadInputDataBit(GPIO_KEY0,GPIO_Pin_KEY0)  //PE4
#define KEY1 		GPIO_ReadInputDataBit(GPIO_KEY0,GPIO_Pin_KEY1) 	//PE3 
#define K_UP 	  GPIO_ReadInputDataBit(GPIO_K_UP,GPIO_Pin_K_UP)	//PA0

#define KEY0_PRES 	1
#define KEY1_PRES	  2
#define K_UP_PRES   3

void KEY_Init(void);	//IO初始化
u8 KEY_Scan(u8);  		//按键扫描函数	

#endif
