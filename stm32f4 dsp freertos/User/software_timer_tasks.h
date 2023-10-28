#ifndef SOFTWARE_TIMER_TASKS_H
#define SOFTWARE_TIMER_TASKS_H


#include <stdint.h>
// include the head files of FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
//#include "event_groups.h"
#include "queue.h"

// include the head files of the project

#include "PID.h"
#include "buffer.h"
#include "Driver.h"
#include "PWM.h"
#include <math.h>
#include <stdlib.h>
#include "VL53.h"
#include "usart.h"

// there are some define about the secord param of the function "startStraight_Line_For_Laser" and "startStraight_Line_Base_On_Encoder"
#define forward 1
#define pan 0


// declare variables
extern struct PID Coord, Turn_Angle_PID, X_Speed_PID, Y_Speed_PID, X_Base_On_Laser_PID, Y_Base_On_Laser_PID;
extern float VOFA_Data[4];
extern uint8_t dataFromLinux[5];//{'1', '1', '2', '2'}; // the data get from linux
extern u8 already_turned, Y_have_achieved, X_have_achieved; // 是否达到定时器目的的信号量
extern int32_t position_of_car[3];

extern TimerHandle_t Turn_Angle_Handle;                       //+发送数据到上位机定时器句柄
extern TimerHandle_t sendto_Upper_Handle;                     //+发送数据到上位机定时器句柄
extern TimerHandle_t line_walking_Handle;                     //+巡线PID定时器句柄
extern TimerHandle_t analyse_data_Handle;                     //+解析数据定时器句柄
extern TimerHandle_t Achieve_Distance_For_Front_Laser_Handle; // 根据激光来跑直线
extern TimerHandle_t Achieve_Distance_For_Right_Laser_Handle;
extern TimerHandle_t Car_Running_Handle;
extern TimerHandle_t Go_Forward_Base_On_Encoder_Handle;
extern TimerHandle_t Pan_Left_Base_On_Encoder_Handle;
extern TimerHandle_t Keep_The_Car_Have_Special_Right_Distance_Handle;


// declare timer callback functions
void Keep_The_Car_Have_Special_Right_Distance(TimerHandle_t xTimer);
void Pan_Left_Base_On_Encoder(TimerHandle_t xTimer);
void Go_Forward_Base_On_Encoder(TimerHandle_t xTimer);
void Achieve_Distance_For_Right_Laser(TimerHandle_t xTimer);
void Achieve_Distance_Front_Head_Laser(TimerHandle_t xTimer);
void Car_Running(TimerHandle_t xTimer);
void Turn_Angle(TimerHandle_t xTimer);
void sendto_Upper(TimerHandle_t xTimer);
void line_walking(TimerHandle_t xTimer);
void analyse_data(void);

// declare those functions which open the software timer
void startStraight_Line_For_Laser(float target, int which_laser);
void startStraight_Line_Base_On_Encoder(float target, int forwardOrPan);
void startgostraight(float target_angle);
void start_trun(int i);

#endif // SOFTWARE_TIMER_TASKS_H