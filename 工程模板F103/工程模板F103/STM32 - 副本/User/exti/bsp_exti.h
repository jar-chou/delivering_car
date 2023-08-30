/*
 * @Author: error: git config user.name && git config user.email & please set dead value or install git
 * @Date: 2023-01-07 15:08:34
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-07-28 12:10:03
 * @FilePath: \RVMDK（uv5）i:\electroni_file\STM32\工程模板（freertos） - 副本\User\exti\bsp_exti.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __EXTI_H
#define __EXTI_H

#include "stm32f10x.h"

/**
 * @description: ????
 * @param {*}
 * @return {*}
 */
#define KEY_ENABLE 0
#define KEY1_GPIO_CLK RCC_APB2Periph_GPIOA
#define KEY1_GPIO_PORT GPIOA
#define KEY1_GPIO_PIN GPIO_Pin_0

#define KEY2_GPIO_CLK RCC_APB2Periph_GPIOC
#define KEY2_GPIO_PORT GPIOC
#define KEY2_GPIO_PIN GPIO_Pin_13

#define KEY1_INT_GPIO_PORT GPIOA
#define KEY1_INT_GPIO_CLK (RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO)
#define KEY1_INT_GPIO_PIN GPIO_Pin_0
#define KEY1_INT_EXTI_PORTSOURCE GPIO_PortSourceGPIOA
#define KEY1_INT_EXTI_PINSOURCE GPIO_PinSource0
#define KEY1_INT_EXTI_LINE EXTI_Line0
#define KEY1_INT_EXTI_IRQ EXTI0_IRQn

#define KEY1_IRQHandler EXTI0_IRQHandler

#define KEY2_INT_GPIO_PORT GPIOC
#define KEY2_INT_GPIO_CLK (RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO)
#define KEY2_INT_GPIO_PIN GPIO_Pin_13
#define KEY2_INT_EXTI_PORTSOURCE GPIO_PortSourceGPIOC
#define KEY2_INT_EXTI_PINSOURCE GPIO_PinSource13
#define KEY2_INT_EXTI_LINE EXTI_Line13
#define KEY2_INT_EXTI_IRQ EXTI15_10_IRQn

#define KEY2_IRQHandler EXTI15_10_IRQHandler

/**
 * @description:?????
 * @param {*}
 * @return {*}
 */
void Key_GPIO_Config(uint32_t KET_GPION_CLOCK, uint16_t GPIO_PIN, GPIO_TypeDef *GPIOX);
// uint8_t Key_Scan(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
static void NVIC_KEY_Configuration(enum IRQn NVIC_IRQChANNEL, uint8_t PREEMPTIONPRIORIT, uint8_t SUBPRIORIT);
void IO_TXD(u8 Data);

#endif /* __EXTI_H */
