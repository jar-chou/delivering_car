#include "usart.h"
#include "buffer.h"
 extern struct Buff U3_buffer, U2_buffer;
/*
way��1.����USARTX_Config()������ʼ������
*/
#if USART1_ENABLE // �궨���Ƿ�ʹ��������룬��ͷ�ļ��޸�
/******************/
/***USART1��ʼ��***/
/*****************/
static void NVIC_USART1_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Ƕ�������жϿ�������ѡ�� */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* ����USARTΪ�ж�Դ */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    /* �������ȼ�*/
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    /* �����ȼ� */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    /* ʹ���ж� */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    /* ��ʼ������NVIC */
    NVIC_Init(&NVIC_InitStructure);
}

void USART1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // �򿪴���GPIO��ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // �򿪴��������ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // ��USART Tx��GPIO����Ϊ���츴��ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // ��USART Rx��GPIO����Ϊ��������ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // ���ô��ڵĹ�������
    // ���ò�����
    USART_InitStructure.USART_BaudRate = 115200;
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
    USART_Init(USART1, &USART_InitStructure);

    // �����ж����ȼ�����
    NVIC_USART1_Configuration();

    //	// ʹ�ܴ��ڽ����ж�
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // ʹ�ܴ���
    USART_Cmd(USART1, ENABLE);
}
//!                           �жϺ���

void USART1_IRQHandler()
{
	//u8 dr;
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == 1)
    {
        //dr = USART_ReceiveData(USART1);
      
    }
    USART_ClearFlag(USART1, USART_FLAG_RXNE);
}

#endif

#if 1
/******************/
/***USART2��ʼ��***/
/*****************/
static void NVIC_USART2_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Ƕ�������жϿ�������ѡ�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* ����USARTΪ�ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	/* �������ȼ�*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	/* �����ȼ� */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	/* ʹ���ж� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* ��ʼ������NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

void USART2_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// �򿪴���GPIO��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// �򿪴��������ʱ��

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// ���ô��ڵĹ�������
	// ���ò�����
	USART_InitStructure.USART_BaudRate = 115200;
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
	USART_Init(USART2, &USART_InitStructure);

	// �����ж����ȼ�����
	NVIC_USART2_Configuration();

	//	// ʹ�ܴ��ڽ����ж�
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	// ʹ�ܴ���
	USART_Cmd(USART2, ENABLE);
}
u8 buff_usart2;
void USART2_IRQHandler()
{
	if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == 1)
	{
		buff_usart2 = USART_ReceiveData(USART2);
    Write_BUFF(&buff_usart2, &U2_buffer);
    }
	USART_ClearFlag(USART2, USART_FLAG_RXNE);
}

#endif
#if 0
/******************/
/***USART3��ʼ��***/
/*****************/
static void
NVIC_USART3_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Ƕ�������жϿ�������ѡ�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* ����USARTΪ�ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART1_IRQ;
	/* �������ȼ�*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	/* �����ȼ� */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	/* ʹ���ж� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* ��ʼ������NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

void USART3_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// �򿪴���GPIO��ʱ��
	DEBUG_USART3_GPIO_APBxClkCmd(DEBUG_USART1_GPIO_CLK, ENABLE);

	// �򿪴��������ʱ��
	DEBUG_USART3_APBxClkCmd(DEBUG_USART1_CLK, ENABLE);

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART1_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART1_TX_GPIO_PORT, &GPIO_InitStructure);

	// ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART1_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_USART1_RX_GPIO_PORT, &GPIO_InitStructure);

	// ���ô��ڵĹ�������
	// ���ò�����
	USART_InitStructure.USART_BaudRate = DEBUG_USART1_BAUDRATE;
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
	USART_Init(DEBUG_USART1, &USART_InitStructure);

	// �����ж����ȼ�����
	NVIC_USART3_Configuration();

	//	// ʹ�ܴ��ڽ����ж�
	//	USART_ITConfig(DEBUG_USARTx, USART_IT_RXNE, ENABLE);

	// ʹ�ܴ���
	USART_Cmd(DEBUG_USART1, ENABLE);
}
#endif
#if 0
/******************/
/***UART4��ʼ��***/
/*****************/
static void NVIC_USART4_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Ƕ�������жϿ�������ѡ�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* ����USARTΪ�ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART1_IRQ;
	/* �������ȼ�*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	/* �����ȼ� */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	/* ʹ���ж� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* ��ʼ������NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

void USART4_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// �򿪴���GPIO��ʱ��
	DEBUG_UART4_GPIO_APBxClkCmd(DEBUG_USART1_GPIO_CLK, ENABLE);

	// �򿪴��������ʱ��
	DEBUG_UART4_APBxClkCmd(DEBUG_USART1_CLK, ENABLE);

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART1_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART1_TX_GPIO_PORT, &GPIO_InitStructure);

	// ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART1_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_USART1_RX_GPIO_PORT, &GPIO_InitStructure);

	// ���ô��ڵĹ�������
	// ���ò�����
	USART_InitStructure.USART_BaudRate = DEBUG_USART1_BAUDRATE;
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
	USART_Init(DEBUG_USART1, &USART_InitStructure);

	// �����ж����ȼ�����
	NVIC_USART4_Configuration();

	//	// ʹ�ܴ��ڽ����ж�
	//	USART_ITConfig(DEBUG_USARTx, USART_IT_RXNE, ENABLE);

	// ʹ�ܴ���
	USART_Cmd(DEBUG_USART1, ENABLE);
}
#endif
#if 0
/******************/
/***UART5��ʼ��***/
/*****************/
static void NVIC_USART5_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Ƕ�������жϿ�������ѡ�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* ����USARTΪ�ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART1_IRQ;
	/* �������ȼ�*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	/* �����ȼ� */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	/* ʹ���ж� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* ��ʼ������NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

void USART5_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// �򿪴���GPIO��ʱ��
	DEBUG_UART5_GPIO_APBxClkCmd(DEBUG_USART1_GPIO_CLK, ENABLE);

	// �򿪴��������ʱ��
	DEBUG_UART5_APBxClkCmd(DEBUG_USART1_CLK, ENABLE);

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART1_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART1_TX_GPIO_PORT, &GPIO_InitStructure);

	// ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART1_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_USART1_RX_GPIO_PORT, &GPIO_InitStructure);

	// ���ô��ڵĹ�������
	// ���ò�����
	USART_InitStructure.USART_BaudRate = DEBUG_USART1_BAUDRATE;
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
	USART_Init(DEBUG_USART1, &USART_InitStructure);

	// �����ж����ȼ�����
	NVIC_USART5_Configuration();

	//	// ʹ�ܴ��ڽ����ж�
	//	USART_ITConfig(DEBUG_USARTx, USART_IT_RXNE, ENABLE);

	// ʹ�ܴ���
	USART_Cmd(DEBUG_USART1, ENABLE);
}
#endif
union AH
{
    float fdata;
    char ldata[4];
} FloatLongType;
void VOFA_Send_float(float *Data, u8 b)
{
    u8 i;
    const u8 tail[4] = {0x00, 0x00, 0x80, 0x7f};
    while (b--)
    {
        FloatLongType.fdata = *Data;
        for (i = 0; i < 4; i++)
        {
            Usart_SendByte(USART1, FloatLongType.ldata[i]);
        }
        Data++;
     }

        for (i = 0; i < 4; i++)
    {
        Usart_SendByte(USART1, tail[i]);
    }
}
/*****************  ����һ���ֽ� **********************/
void Usart_SendByte(USART_TypeDef *pUSARTx, uint8_t ch)
{
    /* ����һ���ֽ����ݵ�USART */
    USART_SendData(pUSARTx, ch);

    /* �ȴ������ݼĴ���Ϊ�� */
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
        ;
}

/****************** ����8λ������ ************************/
void Usart_SendArray(USART_TypeDef *pUSARTx, uint8_t *array, uint16_t num)
{
    uint8_t i;

    for (i = 0; i < num; i++)
    {
        /* ����һ���ֽ����ݵ�USART */
        Usart_SendByte(pUSARTx, array[i]);
    }
    /* �ȴ�������� */
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TC) == RESET)
        ;
}

/*****************  �����ַ��� **********************/
void Usart_SendString(USART_TypeDef *pUSARTx, char *str)
{
    unsigned int k = 0;
    do
    {
        Usart_SendByte(pUSARTx, *(str + k));
        k++;
    } while (*(str + k) != '\0');

    /* �ȴ�������� */
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TC) == RESET)
    {
    }
}

/*****************  ����һ��16λ�� **********************/
void Usart_SendHalfWord(USART_TypeDef *pUSARTx, uint16_t ch)
{
    uint8_t temp_h, temp_l;

    /* ȡ���߰�λ */
    temp_h = (ch & 0XFF00) >> 8;
    /* ȡ���Ͱ�λ */
    temp_l = ch & 0XFF;

    /* ���͸߰�λ */
    USART_SendData(pUSARTx, temp_h);
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
        ;

    /* ���͵Ͱ�λ */
    USART_SendData(pUSARTx, temp_l);
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
        ;
}

/// �ض���c�⺯��printf�����ڣ��ض�����ʹ��printf����
int fputc(int ch, FILE *f)
{
    /* ����һ���ֽ����ݵ����� */
    USART_SendData(DEBUG_USART1, (uint8_t)ch);

    /* �ȴ�������� */
    while (USART_GetFlagStatus(DEBUG_USART1, USART_FLAG_TXE) == RESET)
        ;

    return (ch);
}

/// �ض���c�⺯��scanf�����ڣ���д����ʹ��scanf��getchar�Ⱥ���
int fgetc(FILE *f)
{
    /* �ȴ������������� */
    while (USART_GetFlagStatus(DEBUG_USART1, USART_FLAG_RXNE) == RESET)
        ;

    return (int)USART_ReceiveData(DEBUG_USART1);
}
