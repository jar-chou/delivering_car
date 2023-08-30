
#include "Delay.h"

/**
 * @description: ��ʱ������ʱ������ʼ��
 * @param CLOCK:ѡ��ʱ��Դ��ע��ֻ�ܳ�ʼ��TIM2-7
 * @param TIM_PERIOD����װ�ؼ���ֵ
 * @param TIM_PRESCALER��Ԥ��Ƶϵ��������ѡ72��72��ʱ��ÿ����һ��Ϊ1us��
 * @return {*}
 */
//*TIMX_Delay_Init(RCC_APB1Periph_TIM6, 65530, 72, TIM6)
void TIMX_Delay_Init(uint32_t ClOCK, uint16_t TIM_PERIOD, uint16_t TIM_PRESCALER, TIM_TypeDef *TIMX)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    RCC_APB1PeriphClockCmd(ClOCK, ENABLE);                          /// ʹ��TIM3ʱ��
    TIM_TimeBaseInitStructure.TIM_Period = TIM_PERIOD;              // �Զ���װ��ֵ
    TIM_TimeBaseInitStructure.TIM_Prescaler = TIM_PRESCALER - 1;    // ��ʱ����Ƶ
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // ���ϼ���ģʽ
    TIM_TimeBaseInit(TIMX, &TIM_TimeBaseInitStructure);             // ��ʼ��TIM3
    TIM_Cmd(TIMX, DISABLE);                                         // �رն�ʱ��
}
/**
 * @description:΢����ʱ����
 * @param TIMX��ѡ��ʱ��
 * @param xus����ʱ��С΢��
 * @return {*}
 */
void Delayus(u16 xus)
{

    TIM_Cmd(TIM6, ENABLE); // ������ʱ��
    while (TIM6->CNT < xus)
        ;                   // ��ȡ��ʱ���Ĵ���CNT��ֵ
    TIM_Cmd(TIM6, DISABLE); // �رն�ʱ��
    TIM6->CNT = 0;
}
/**
 * @description: ������ʱ����
 * @param TIMX��ѡ��ʱ��
 * @param xms����ʱ��С����
 */
void Delayms(u16 xms)
{
    int i;
    for (i = 0; i < xms; i++)
    {
        Delayus(1000);
    }
}
