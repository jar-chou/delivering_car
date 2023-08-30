#include "VL53.h"
#include "buffer.h"
#define EN_VL53 0
#if EN_VL53
static void NVIC_USARTX_VL53_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Ƕ�������жϿ�������ѡ�� */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* ����USARTΪ�ж�Դ */
    NVIC_InitStructure.NVIC_IRQChannel = VL53_USARTX_IRQn;
    /* �������ȼ�*/
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    /* �����ȼ� */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    /* ʹ���ж� */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    /* ��ʼ������NVIC */
    NVIC_Init(&NVIC_InitStructure);
}

void USARTX_VL53_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // �򿪴���GPIO��ʱ��
    RCC_APB2PeriphClockCmd(VL53_GPIO_CLOCK, ENABLE);

    // �򿪴��������ʱ��
    RCC_APB1PeriphClockCmd(VL53_USARTX_CLOCK, ENABLE);

    // ��USART Tx��GPIO����Ϊ���츴��ģʽ
    GPIO_InitStructure.GPIO_Pin = VL53_TX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(VL53_GPIOX, &GPIO_InitStructure);

    // ��USART Rx��GPIO����Ϊ��������ģʽ
    GPIO_InitStructure.GPIO_Pin = VL53_RX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(VL53_GPIOX, &GPIO_InitStructure);

    // ���ô��ڵĹ�������
    // ���ò�����
    USART_InitStructure.USART_BaudRate = VL53_BAUDRATE;
    // ���� �������ֳ�
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    // ����ֹͣλ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    // ����У��λ
    USART_InitStructure.USART_Parity = USART_Parity_No;
    // ����Ӳ��������
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    // ���ù���ģʽ���շ�һ��
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    // ��ɴ��ڵĳ�ʼ������
    USART_Init(VL53_USARTX, &USART_InitStructure);

    // �����ж����ȼ�����
    NVIC_USARTX_VL53_Configuration();

    // ʹ�ܴ��ڽ����ж�
    USART_ITConfig(VL53_USARTX, USART_IT_RXNE, ENABLE);

    // ʹ�ܴ���
    USART_Cmd(VL53_USARTX, ENABLE);
}

struct Buff VL53_USARTX_Buff;

void VL53_USARTX_IRQHandler()
{
    u8 Data;
    if (USART_GetFlagStatus(VL53_USARTX, USART_FLAG_RXNE) == 1)
    {
        Data = VL53_USARTX->DR;
        Write_BUFF(&Data, &VL53_USARTX_Buff); // �Ѵ��ڽ��յ������ݲ�������λ�����
    }
    USART_ClearFlag(VL53_USARTX, USART_FLAG_RXNE);
}
/**
 * @description: ��ʼ�������ࣨ�ܺ�����
 * @return {*}
 */
void VL53_Initial()
{
    USARTX_VL53_Config();
    Iinitial_BUFF(&VL53_USARTX_Buff);
}
/**
 * @description: ��ȡBuff���ݣ�������Ϊ��������
 * @param {u16} *distance ����һ��u16��ָ����վ�������
 * @return {*}
 */
void VL53_Read_Data(u16 *distance)
{
    const u8 VL53_Agreement_RX[] = {0x50, 0x03, 0x02};
    u8 H_bit, L_bit, i = 5;
    while (i--)
    {
        if (*(u8 *)Read_BUFF(&VL53_USARTX_Buff) == VL53_Agreement_RX[0])
        {
            if (*(u8 *)Read_BUFF(&VL53_USARTX_Buff) == VL53_Agreement_RX[1])
            {
                if (*(u8 *)Read_BUFF(&VL53_USARTX_Buff) == VL53_Agreement_RX[2])
                {
                    H_bit = *(u8 *)Read_BUFF(&VL53_USARTX_Buff);
                    L_bit = *(u8 *)Read_BUFF(&VL53_USARTX_Buff);
                    *distance = (H_bit << 8) + L_bit;
                    break;
                }
            }
        }
    }
}
/**
 * @description: ���Ͷ�ȡ��������
 * @return {*}
 */
void VL53_Send_Agrement()
{
    u8 i;
    const u8 Agreement[] = {0x50, 0x03, 0x00, 0x34, 0x00, 0X01, 0XC8, 0X45};
    for (i = 0; i < 8; i++)
    {
        USART_SendData(VL53_USARTX, Agreement[i]);
        while (USART_GetFlagStatus(VL53_USARTX, USART_FLAG_TXE) == RESET)
            ;
    }
}
#endif
