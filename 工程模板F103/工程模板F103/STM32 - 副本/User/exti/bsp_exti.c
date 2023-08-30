/*
 * @Author: your name
 * @Date: 2023-01-06 15:06:07
 * @LastEditTime: 2023-01-14 21:23:20
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @Description: In User Settings Edit
 * @FilePath: \RVMDK��uv5��i:\electroni_file\STM32\����ģ�壨freertos�� - ����\User\exti\bsp_exti.c
 */

#include "bsp_exti.h"

#if KEY_ENABLE
/**
 * @description: ��ʼ������
 * @param {*}
 * @return {*}
 */
void Key_GPIO_Config(uint32_t KET_GPION_CLOCK, uint16_t GPIO_PIN, GPIO_TypeDef *GPIOX)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /*���������˿ڵ�ʱ��*/
    RCC_APB2PeriphClockCmd(KET_GPION_CLOCK, ENABLE);
    // ѡ�񰴼�������
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN;
    // ���ð���������Ϊ��������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    // ʹ�ýṹ���ʼ������
    GPIO_Init(GPIOX, &GPIO_InitStructure);
}

/**
 * @brief  ����Ƕ�������жϿ�����NVIC
 * @param  NVIC_IRQChANNEL:�ж�ͨ����PREEMPTIONPRIORIT
 * @param  PREEMPTIONPRIORIT����ռ���ȼ�
 * @param  SUBPRIORIT�������ȼ�
 * @retval ��
 */
static void NVIC_KEY_Configuration(enum IRQn NVIC_IRQChANNEL, uint8_t PREEMPTIONPRIORIT, uint8_t SUBPRIORIT)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    /* ����NVICΪ���ȼ���1 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    /* �����ж�Դ������1 */
    NVIC_InitStructure.NVIC_IRQChannel = NVIC_IRQChANNEL;
    /* ������ռ���ȼ� */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PREEMPTIONPRIORIT;
    /* ���������ȼ� */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = SUBPRIORIT;
    /* ʹ���ж�ͨ�� */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief  ���� IOΪEXTI�жϿڣ��������ж����ȼ�
 * @param  ��
 * @retval ��
 */
void EXTI_Key_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    /*��������GPIO�ڵ�ʱ��*/
    RCC_APB2PeriphClockCmd(KEY1_INT_GPIO_CLK, ENABLE);

    /* ���� NVIC �ж�*/
    // NVIC_KEY_Configuration();

    /*--------------------------KEY1����-----------------------------*/
    /* ѡ�񰴼��õ���GPIO */
    GPIO_InitStructure.GPIO_Pin = KEY1_INT_GPIO_PIN;
    /* ����Ϊ�������� */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(KEY1_INT_GPIO_PORT, &GPIO_InitStructure);

    /* ѡ��EXTI���ź�Դ */
    GPIO_EXTILineConfig(KEY1_INT_EXTI_PORTSOURCE, KEY1_INT_EXTI_PINSOURCE);
    EXTI_InitStructure.EXTI_Line = KEY1_INT_EXTI_LINE;

    /* EXTIΪ�ж�ģʽ */
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    /* �������ж� */
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    /* ʹ���ж� */
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}
#endif
/*********************************************END OF FILE**********************/
