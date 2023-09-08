
#include "PID.h"
/**
 * @description: ��ʼ��PID�ṹ�����
 * @param {PID} pid ����PID�ṹ�����
 * @param {float} KP
 * @param {float} KD
 * @param {float} KI
 * @param {float} limit
 * @return {*}
 */
void PID_Initialize(struct PID *pid, float KP, float KD, float KI, int Traget, float limit_high, float limit_low)
{
    pid->Target = Traget;
    pid->KP = KP;
    pid->KI = KI;
    pid->KD = KD;
    pid->limit_high = limit_high;
    pid->limit_low = limit_low;
    pid->Cumulation_Error = 0;
    pid->Last_Error = 0;
    pid->Previous_Error = 0;
}

//! λ��ʽPID����
/**
 * @description:λ��ʽPID����
 * @param {PID} pid ����PID�ṹ�����
 * @param {float} Target Ŀ��ֵ
 * @param {float} Current ��ǰֵ
 * @return {*}
 */
int PID_Realize(struct PID *pid, float Current)
{
    
    
    float iError, Realize;                                                                                     // ʵ�����
    iError = pid->Target - Current;                                                                            // ���㵱ǰ���
    pid->Cumulation_Error += iError;                                                                           // ������
    pid->Cumulation_Error = pid->Cumulation_Error > pid->limit_high ? pid->limit_high : pid->Cumulation_Error; // �����޷�//����
    pid->Cumulation_Error = pid->Cumulation_Error < pid->limit_low ? pid->limit_low : pid->Cumulation_Error;   // �����޷�//����
    Realize = pid->KP * iError + pid->Cumulation_Error * pid->KI + pid->KD * (iError - pid->Last_Error);
    pid->Last_Error = iError; // �����ϴ����
    return (int)Realize;           // ����ʵ��ֵ
}
/**
 * @description: ����ʽPID�㷨����
 * @param {PID} *pid
 * @param {float} Target
 * @param {float} Current
 * @return {*}
 */
int PID_Increase(struct PID *pid, int Current)
{

    float iError, // ��ǰ���
        Increase; // ���ó���ʵ������

    iError = pid->Target - Current; // ���㵱ǰ���

    Increase = pid->KP * (iError - pid->Last_Error) + pid->KI * iError + pid->KD * (iError - 2 * pid->Last_Error + pid->Previous_Error); // ΢��D

    pid->Previous_Error = pid->Last_Error; // ����ǰ�����
    pid->Last_Error = iError;              // �����ϴ����

    return Increase; // ��������
}
/**
 * @description: ����޷�����
 * @param {int} Out_PID PID���ֵ
 * @param {int} Max ���ֵ
 * @param {int} Min ��Сֵ
 * @return {*}
 */
int Limited_Out(int Out_PID, int Max, int Min)
{
    if (Out_PID >= Max)
    {
        Out_PID = Max;
    }
    if (Out_PID <= Min)
    {
        Out_PID = Min;
    }
    return Out_PID;
}
/**
 * @description: ����PID
 * @param {int} *Out_PID_x
 * @param {int} *Out_PID_y
 * @return {*}
 */
void Series_PID(int *Out_PID_x, int *Out_PID_y)
{
    // int x, y;
    // x = PID_Realize(&extral, x1);
    // y = PID_Realize(&extral, y1);
    // vTaskDelay(100);
    // *Out_PID_x = 1500 + PID_Realize(&inner, x1);
    // *Out_PID_y = 1500 - PID_Realize(&inner, y1);
    // set_computer_value(SEND_TARGET_CMD, CURVES_CH1, &target_speed, 1);
}
/*
����ʽ��λ��ʽ����
1����ʽ�㷨����Ҫ���ۼӣ�������������ȷ�������������ƫ�����ֵ�йأ��������Կ��� �������Ӱ���С����λ��ʽ�㷨Ҫ�õ���ȥƫ����ۼ�ֵ�����ײ����ϴ���ۼ���

        2����ʽ�㷨�ó����ǿ������������������ڷ��ſ����У�ֻ������ſ��ȵı仯���֣����� Ӱ��С����Ҫʱ����ͨ���߼��ж����ƻ��ֹ�����������������Ӱ��ϵͳ�Ĺ����� ��λ��ʽ�����ֱ�Ӷ�Ӧ������������˶�ϵͳӰ��ϴ�

        3����ʽPID����������ǿ��������������޻������ã���˸÷���������ִ�л��������ֲ����Ķ����粽������ȣ���λ��ʽPID������ִ�л����������ֲ����Ķ������Һ�ŷ�����

        4�ڽ���PID����ʱ��λ��ʽPID��Ҫ�л����޷�������޷���������ʽPIDֻ������޷�

        λ��ʽPID��ȱ�㣺
            �ŵ㣺
��λ��ʽPID��һ�ַǵ���ʽ�㷨����ֱ�ӿ���ִ�л�������ƽ��С������u(k) ��ֵ��ִ�л�����ʵ��λ�ã���С����ǰ�Ƕȣ���һһ��Ӧ�ģ������ִ�л����������ֲ����Ķ����п��Ժܺ�Ӧ��

        ȱ�㣺
��ÿ����������ȥ��״̬�йأ�����ʱҪ��e(k) �����ۼӣ����㹤������

        ����ʽPID��ȱ�㣺
        �ŵ㣺
������ʱӰ��С����Ҫʱ�����߼��жϵķ���ȥ���������ݡ�
���ֶ� / �Զ��л�ʱ���С������ʵ�����Ŷ��л��������������ʱ�����ܱ���ԭֵ��
����ʽ�в���Ҫ�ۼӡ�����������u(k) ��ȷ���������3�εĲ���ֵ�йء�

        ȱ�㣺
�ٻ��ֽض�ЧӦ������̬��

�������Ӱ����еı��ض���������ʽ��̫�ã�
*/
