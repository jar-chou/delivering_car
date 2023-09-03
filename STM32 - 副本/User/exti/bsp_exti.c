/*
 * @Author: your name
 * @Date: 2023-01-06 15:06:07
 * @LastEditTime: 2023-01-14 21:23:20
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @Description: In User Settings Edit
 * @FilePath: \RVMDK（uv5）i:\electroni_file\STM32\工程模板（freertos） - 副本\User\exti\bsp_exti.c
 */

#include "bsp_exti.h"

#if KEY_ENABLE
/**
 * @description: 初始化按键
 * @param {*}
 * @return {*}
 */
void Key_GPIO_Config(uint32_t KET_GPION_CLOCK, uint16_t GPIO_PIN, GPIO_TypeDef *GPIOX)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /*开启按键端口的时钟*/
    RCC_APB2PeriphClockCmd(KET_GPION_CLOCK, ENABLE);
    // 选择按键的引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN;
    // 设置按键的引脚为浮空输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    // 使用结构体初始化按键
    GPIO_Init(GPIOX, &GPIO_InitStructure);
}

/**
 * @brief  配置嵌套向量中断控制器NVIC
 * @param  NVIC_IRQChANNEL:中断通道，PREEMPTIONPRIORIT
 * @param  PREEMPTIONPRIORIT：抢占优先级
 * @param  SUBPRIORIT：子优先级
 * @retval 无
 */
static void NVIC_KEY_Configuration(enum IRQn NVIC_IRQChANNEL, uint8_t PREEMPTIONPRIORIT, uint8_t SUBPRIORIT)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    /* 配置NVIC为优先级组1 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    /* 配置中断源：按键1 */
    NVIC_InitStructure.NVIC_IRQChannel = NVIC_IRQChANNEL;
    /* 配置抢占优先级 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PREEMPTIONPRIORIT;
    /* 配置子优先级 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = SUBPRIORIT;
    /* 使能中断通道 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief  配置 IO为EXTI中断口，并设置中断优先级
 * @param  无
 * @retval 无
 */
void EXTI_Key_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    /*开启按键GPIO口的时钟*/
    RCC_APB2PeriphClockCmd(KEY1_INT_GPIO_CLK, ENABLE);

    /* 配置 NVIC 中断*/
    // NVIC_KEY_Configuration();

    /*--------------------------KEY1配置-----------------------------*/
    /* 选择按键用到的GPIO */
    GPIO_InitStructure.GPIO_Pin = KEY1_INT_GPIO_PIN;
    /* 配置为浮空输入 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(KEY1_INT_GPIO_PORT, &GPIO_InitStructure);

    /* 选择EXTI的信号源 */
    GPIO_EXTILineConfig(KEY1_INT_EXTI_PORTSOURCE, KEY1_INT_EXTI_PINSOURCE);
    EXTI_InitStructure.EXTI_Line = KEY1_INT_EXTI_LINE;

    /* EXTI为中断模式 */
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    /* 上升沿中断 */
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    /* 使能中断 */
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}
#endif
/*********************************************END OF FILE**********************/
