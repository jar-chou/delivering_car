
#include "Delay.h"

/**
 * @description: 定时器做延时函数初始化
 * @param CLOCK:选择时钟源，注：只能初始化TIM2-7
 * @param TIM_PERIOD：重装载计数值
 * @param TIM_PRESCALER：预分频系数，建议选72（72定时器每计数一次为1us）
 * @return {*}
 */
//*TIMX_Delay_Init(RCC_APB1Periph_TIM6, 65530, 72, TIM6)
void TIMX_Delay_Init(uint32_t ClOCK, uint16_t TIM_PERIOD, uint16_t TIM_PRESCALER, TIM_TypeDef *TIMX)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    RCC_APB1PeriphClockCmd(ClOCK, ENABLE);                          /// 使能TIM3时钟
    TIM_TimeBaseInitStructure.TIM_Period = TIM_PERIOD;              // 自动重装载值
    TIM_TimeBaseInitStructure.TIM_Prescaler = TIM_PRESCALER - 1;    // 定时器分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseInit(TIMX, &TIM_TimeBaseInitStructure);             // 初始化TIM3
    TIM_Cmd(TIMX, DISABLE);                                         // 关闭定时器
}
/**
 * @description:微妙延时函数
 * @param TIMX：选择定时器
 * @param xus：延时多小微妙
 * @return {*}
 */
void Delayus(u16 xus)
{

    TIM_Cmd(TIM6, ENABLE); // 启动定时器
    while (TIM6->CNT < xus)
        ;                   // 获取定时器寄存器CNT的值
    TIM_Cmd(TIM6, DISABLE); // 关闭定时器
    TIM6->CNT = 0;
}
/**
 * @description: 毫秒延时函数
 * @param TIMX：选择定时器
 * @param xms：延时多小毫秒
 */
void Delayms(u16 xms)
{
    int i;
    for (i = 0; i < xms; i++)
    {
        Delayus(1000);
    }
}
