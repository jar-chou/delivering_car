/*
 * @Author: error: git config user.name && git config user.email & please set dead value or install git
 * @Date: 2023-01-11 11:51:47
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2023-01-14 00:25:01
 * @FilePath: \RVMDK（uv5）i:\electroni_file\STM32\工程模板（freertos） - 副本\User\kalman.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef Kalman_H
#define Kalman_H

#include "stm32f10x.h"

struct Kalman
{
	float LastP;//上次估算协方差 初始化值为0.02
    float Now_P;//当前估算协方差 初始化值为0
    float out;//卡尔曼滤波器输出 初始化值为0
    float Kg;//卡尔曼增益 初始化值为0
    float Q;//过程噪声协方差 初始化值为0.001
    float R;//观测噪声协方差 初始化值为0.543
};
void Kalman_Init(struct Kalman *KFP);
float KalmanFilter(struct Kalman Kalman, float input);
#endif
