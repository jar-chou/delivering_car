#include "software_timer_tasks.h"
#include "sys.h"
#include <string.h>
#include "encoder.h"

TimerHandle_t Turn_Angle_Handle = NULL;                       //+发送数据到上位机定时器句柄
TimerHandle_t sendto_Upper_Handle = NULL;                     //+发送数据到上位机定时器句柄
TimerHandle_t line_walking_Handle = NULL;                     //+巡线PID定时器句柄
TimerHandle_t analyse_data_Handle = NULL;                     //+解析数据定时器句柄
TimerHandle_t Achieve_Distance_For_Front_Laser_Handle = NULL; // 根据激光来跑直线
TimerHandle_t Achieve_Distance_For_Right_Laser_Handle = NULL;
TimerHandle_t Car_Running_Handle = NULL;
TimerHandle_t Go_Forward_Base_On_Encoder_Handle = NULL;
TimerHandle_t Pan_Left_Base_On_Encoder_Handle = NULL;
TimerHandle_t Keep_The_Car_Have_Special_Right_Distance_Handle = NULL;

// define variables
struct distance
{
    u16 F[5];
    int R[5];
    u16 F_OUT;
    int R_OUT;
} Distance;
int32_t Y_Speed = 0;
int32_t X_Speed = 0;
int32_t angle_speed = 0;
int32_t position_of_car[3] = {0};
u8 already_turned = 0, Y_have_achieved = 0, X_have_achieved = 0; // 是否达到定时器目的的信号量
u8 dataFromLinux[5] = {0};                                       //{'1', '1', '2', '2'}; // the data get from linux
float VOFA_Data[4];
struct angle Angle; // the angle of the car
extern struct Buff VL53_USARTX_Buff;
/**********************************************************************
 * @ 函数名  ： Timer_Task
 * @ 功能说明： Timer_Task任务主体
 * @ 参数    ：
 * @ 返回值  ： 无
 ********************************************************************/

/**
 * @description: this function is the software callback function that keep the car have special right distance
 * @return {*}
 */
void Keep_The_Car_Have_Special_Right_Distance(TimerHandle_t xTimer)
{
    int32_t distance = VOFA_Data[2];
    int32_t absolute_error = fabs(distance - X_Base_On_Laser_PID.Target);
    if ((absolute_error > 100) || (absolute_error < 10)) // 如果距离超过100cm，就不要再跑了
    {
        taskENTER_CRITICAL();
        X_Speed = 0;
        taskEXIT_CRITICAL();
        return;
    }
    int32_t speed = (int32_t)PID_Realize(&X_Base_On_Laser_PID, distance);
    speed = speed > 450 ? 450 : speed;
    speed = speed < -450 ? -450 : speed;
    taskENTER_CRITICAL();
    X_Speed = speed;
    taskEXIT_CRITICAL();
}
/**
 * @description: this function is the software callback function that achieve pan left base on encoder num
 * @return {*}
 */
void Pan_Left_Base_On_Encoder(TimerHandle_t xTimer)
{
    static int i = 0;
    int32_t distance = position_of_car[1];
    int32_t speed = (int32_t)PID_Realize(&X_Speed_PID, distance);
    speed = speed > 500 ? 500 : speed;
    speed = speed < -500 ? -500 : speed;
    taskENTER_CRITICAL();
    X_Speed = speed;
    taskEXIT_CRITICAL();
    if (fabs(distance - X_Speed_PID.Target) < 15)
    {
        i++;
        if (i > 4)
        {
            i = 0;
            xTimerStop(Pan_Left_Base_On_Encoder_Handle, 0);
            X_have_achieved = 1;
        }
    }
}

/**
 * @description: this function is the software callback function that achieve go forward base on encoder num
 * @return {*}
 */
void Go_Forward_Base_On_Encoder(TimerHandle_t xTimer)
{
    static int i = 0;
    int32_t distance = position_of_car[0];
    int32_t speed = (int32_t)PID_Realize(&Y_Speed_PID, distance);
    speed = speed > 1300 ? 1300 : speed;
    speed = speed < -1300 ? -1300 : speed;
    taskENTER_CRITICAL();
    Y_Speed = speed;
    taskEXIT_CRITICAL();
    if (fabs(distance - Y_Speed_PID.Target) < 20)
    {
        i++;
        if (i > 4)
        {
            i = 0;
            xTimerStop(Go_Forward_Base_On_Encoder_Handle, 0);
            Y_have_achieved = 1;
        }
    }
}

/**
 * @description: this function is the software callback function that achieve reach the target distance for right laser
 * @return {*}
 */
void Achieve_Distance_For_Right_Laser(TimerHandle_t xTimer)
{
    static int i = 0;
    float distance = VOFA_Data[2];
    int32_t speed = -(int32_t)PID_Realize(&X_Base_On_Laser_PID, distance);
    speed = speed > 550 ? 550 : speed;
    speed = speed < -550 ? -550 : speed;

    taskENTER_CRITICAL();
    X_Speed = speed;
    taskEXIT_CRITICAL();

    if (fabs(distance - X_Base_On_Laser_PID.Target) < 7)
    {
        i++;
        if (i > 4)
        {
            i = 0;
            xTimerStop(Achieve_Distance_For_Right_Laser_Handle, 0);
            X_have_achieved = 1;
        }
    }
}

/**
 * @description: this function is the software callback function that achieve reach the target distance for front laser
 * @return {*}
 */
void Achieve_Distance_Front_Head_Laser(TimerHandle_t xTimer)
{
    static int i = 0;
    float distance = VOFA_Data[3];
    int32_t speed = -(int32_t)PID_Realize(&Y_Base_On_Laser_PID, distance);
    speed = speed > 200 ? 200 : speed;
    speed = speed < -200 ? -200 : speed;

    taskENTER_CRITICAL();
    Y_Speed = speed;
    taskEXIT_CRITICAL();

    if (fabs(distance - Y_Base_On_Laser_PID.Target) < 10)
    {
        i++;
        if (i > 2)
        {
            i = 0;
            xTimerStop(Achieve_Distance_For_Front_Laser_Handle, 0);
            Y_have_achieved = 1;
        }
    }
}

/**
 * @description: this function is the software callback function that calculates the ccr register value of the wheel
 * @return {*}
 */
void Car_Running(TimerHandle_t xTimer)
{
    taskENTER_CRITICAL(); // enter critical zone and operate global variable

    // get the value of global variable
    int32_t Local_Y_Speed = Y_Speed;
    int32_t Local_X_Speed = X_Speed;
    int32_t Local_angle_speed = angle_speed;

    // clear the value of global variable
    Y_Speed = 0;
    X_Speed = 0;
    angle_speed = 0;

    taskEXIT_CRITICAL(); // exit critical zone

    int32_t CCR_wheel[4] = {0};
    // calculate the ccr register value of the wheel
    CCR_wheel[0] = Local_Y_Speed;
    CCR_wheel[1] = Local_Y_Speed;
    CCR_wheel[2] = Local_Y_Speed;
    CCR_wheel[3] = Local_Y_Speed;

    CCR_wheel[0] += Local_X_Speed;
    CCR_wheel[1] += Local_X_Speed;
    CCR_wheel[2] -= Local_X_Speed;
    CCR_wheel[3] -= Local_X_Speed;

    CCR_wheel[0] -= Local_angle_speed;
    CCR_wheel[1] += Local_angle_speed;
    CCR_wheel[2] += Local_angle_speed;
    CCR_wheel[3] -= Local_angle_speed;

    // change the direction of the wheel
    for (size_t i = 0; i < 4; i++)
    {
        if (CCR_wheel[i] > 0) // forward
        {
            Advance(i + 2); // it is because the wheel is from 2 to 5
        }
        else // backward
        {
            Back(i + 2);                  // it is because the wheel is from 2 to 5
            CCR_wheel[i] = -CCR_wheel[i]; // change the value to positive, because the function "SetCompare1" need a positive value
        }
    }

    // set the ccr register value of the wheel
    SetCompare1(TIM1, CCR_wheel[0], 1);
    SetCompare1(TIM1, CCR_wheel[1], 2);
    SetCompare1(TIM1, CCR_wheel[2], 3);
    SetCompare1(TIM1, CCR_wheel[3], 4);
}

/**
 * @description: this function is the software callback function that achieve turn angle
 * @return {*}
 */
void Turn_Angle(TimerHandle_t xTimer)
{
    static u8 i = 0;
    float angle, PIDOUT;
    angle = (float)Angle.z / 32768 * 180;
    PIDOUT = PID_Realize(&Turn_Angle_PID, angle);
    angle_speed = (int32_t)PIDOUT;
    if (fabs(angle - Turn_Angle_PID.Target) < 1)
    {
        i++;
        if (i > 4)
        {
            i = 0;
            xTimerStop(Turn_Angle_Handle, 0);
            already_turned = 1;
        }
    }
    // Allocation_PID((int)PIDOUT);
}

/**
 * @description: this function is the software callback function that send data to upper computer
 * @return {*}
 */
void sendto_Upper(TimerHandle_t xTimer)
{

    VOFA_Data[0] = (float)Angle.z / 32768 * 180;
    VOFA_Send_float(VOFA_Data, 4); //! 发送数据给VOFA
}

/**
 * @description: this function is the software callback function that keep the car walking straightly
 * @return {*}
 */
void line_walking(TimerHandle_t xTimer)
{
    float angle;
    //? PID开始巡线
    angle = (float)Angle.z / 32768 * 180;
    angle_speed = PID_Realize(&Coord, angle);
    return;
}

/**
 * @description: this function is the software callback function that analyse data
 * @return {*}
 */
void analyse_data(void)
{
    u8 check_sum;
    const u8 TOFSENSE[] = {0x55, 0x53};

    Read_buff_Void(&U2_buffer, TOFSENSE, 2, Angle.data, 9, 8, 0);
    check_sum = (0x55 + 0x53 + Angle.data[5] + Angle.data[4] + Angle.data[7] + Angle.data[6]);
    if (Angle.data[8] == check_sum)
    {
        Angle.z = -((Angle.data[5] << 8) + Angle.data[4]);
        printf("%.2f\r\n", (float)Angle.z / 32768 * 180);
    }

    const u8 VL53_Agreement_RX[] = {0x50, 0x03, 0x02};
    const u8 RIGHTLASER[] = {0x57, 0x00, 0xFF, 0x01};
    u8 TOF_Data[7] = {0};
    VL53_Send_Agrement();
    if (have_enough_data(&U3_buffer, 3, 1, 16))
    {
        while (have_enough_data(&U3_buffer, 3, 1, 16))
            Read_buff_Void(&U3_buffer, VL53_Agreement_RX, 3, &Distance.F_OUT, 1, 16, 1);
    }

    const float alpha = 0.1;        // low pass filter
    if (have_enough_data(&VL53_USARTX_Buff, 4, 9, 8))
    {
        while (have_enough_data(&VL53_USARTX_Buff, 4, 9, 8))
            Read_buff_Void(&VL53_USARTX_Buff, RIGHTLASER, 4, TOF_Data, 7, 8, 1);
        float distance = (int32_t)(TOF_Data[4] << 8 | TOF_Data[5] << 16 | TOF_Data[6] << 24) / 256;
        Distance.R_OUT = (1 - alpha) * Distance.R_OUT + alpha * distance;       // low pass filter
        USART_SendData(VL53_USARTX, 0x0F);
    }

    static int32_t How_many_revolutions_of_the_motor[4] = {0, 0, 0, 0};

    How_many_revolutions_of_the_motor[0] = Read_Encoder(2);
    How_many_revolutions_of_the_motor[1] = Read_Encoder(3);
    How_many_revolutions_of_the_motor[2] = Read_Encoder(4);
    How_many_revolutions_of_the_motor[3] = Read_Encoder(5);

    u8 get_howmanyencoder = 0;
    for (u8 n = 0; n < 4; n++)
    {
        if (abs(How_many_revolutions_of_the_motor[n]) > 5)
            get_howmanyencoder++;
    }

    int32_t The_distance_that_has_gone_forward = 0;
    if (get_howmanyencoder != 0)
        The_distance_that_has_gone_forward = (How_many_revolutions_of_the_motor[0] + How_many_revolutions_of_the_motor[1] + How_many_revolutions_of_the_motor[2] + How_many_revolutions_of_the_motor[3]) / get_howmanyencoder;
    int32_t The_distance_that_has_gone_right_head_side = (How_many_revolutions_of_the_motor[1] - How_many_revolutions_of_the_motor[2]) / 2;

    if (!check_whetherCharArrayInRange(dataFromLinux, 4, '1', '2'))
    {
        // get data from upper computer
        const static u8 upper_head[] = {0x1C, 0x2C, 0x3C};
        u8 local_dataFromLinux_buff[4] = {0, 0};
        static u8 index = 0;
        static u8 local_dataFromLinux[4] = {0, 0};
        while (have_enough_data(&U4_buffer, 3, 4, 8)) // judge whether the data is enough
        {
            Read_buff_Void(&U4_buffer, (u8 *)upper_head, 3, local_dataFromLinux_buff, 4, 8, 1); // read the data from buffer

            if (!memcmp(local_dataFromLinux_buff, local_dataFromLinux, 4)) // judge whether the data is same as the last data
            {
                index++;
                if (index >= 1) // if the data is same as the last data, then judge whether the data is same as the last data for 4 times
                {
                    index = 0;
                    //
                    memcpy(dataFromLinux, local_dataFromLinux, 4); // copy the data from local variable to global variable
                    printf("%s\n", dataFromLinux);
                }
            }
            else
            {
                index = 0;
                memcpy(local_dataFromLinux, local_dataFromLinux_buff, 4); // copy the data from local variable to global variable
            }
        }
    }

    taskENTER_CRITICAL(); // 进入基本临界区

    VOFA_Data[2] = Distance.R_OUT; // 右边激光测距得到的距离
    VOFA_Data[3] = Distance.F_OUT; // 前面激光测距得到的距离
    // Distance.F_OUT = 0;
    // Distance.R_OUT = 0;
    position_of_car[0] += The_distance_that_has_gone_forward;         // the distance that car have gone forward
    position_of_car[1] += The_distance_that_has_gone_right_head_side; // the distance that car have gone right hand side
    // Read_RGB();

    taskEXIT_CRITICAL(); // 退出基本临界区
}

/**
 * the software timer callback function is end in here
 */

/**
 * the following code is about open the software timer
 */
// there are some define about the secord param of the function "startStraight_Line_For_Laser" and "startStraight_Line_Base_On_Encoder"
// #define forward 1
// #define pan 0
/**
 * @description:    Follow the laser and go straight
 * @param {float} target        what is the distance of laser you want?
 * @param {int} which_laser     which is the laser that you want to refer to? When the value is equal to 1, it indicates the laser on the front of the reference
 * @return {*}
 */
void startStraight_Line_For_Laser(float target, int which_laser)
{

    if (which_laser == 0)
    {
        X_Base_On_Laser_PID.Target = target;
        X_Base_On_Laser_PID.Cumulation_Error = 0;
        X_have_achieved = 0;
        xTimerStart(Achieve_Distance_For_Right_Laser_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }
    else if (which_laser == 1) // 根据前面的激光距离来调整距离
    {
        Y_Base_On_Laser_PID.Target = target;
        Y_Base_On_Laser_PID.Cumulation_Error = 0;
        Y_have_achieved = 0;
        xTimerStart(Achieve_Distance_For_Front_Laser_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }
}

/**
 * @description:    Follow the encoder and go straight
 * @param {float} target        what is the distance of encoder you want?
 * @param {int} forwardOrPan    which is the encoder that you want to refer to? When the value is equal to 1, it indicates the encoder on the front of the reference
 * @return {*}
 */
void startStraight_Line_Base_On_Encoder(float target, int forwardOrPan)
{
    if (forwardOrPan == 0)
    {
        X_Speed_PID.Target = target;
        X_Speed_PID.Cumulation_Error = 0;
        X_have_achieved = 0;
        position_of_car[1] = 0;
        xTimerStart(Pan_Left_Base_On_Encoder_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }
    else if (forwardOrPan == 1) // 根据编码器向前走
    {
        Y_Speed_PID.Target = target;
        Y_Speed_PID.Cumulation_Error = 0;
        Y_have_achieved = 0;
        position_of_car[0] = 0;
        xTimerStart(Go_Forward_Base_On_Encoder_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }
    return;
}

/**
 * @description: start to Keep the direction Angle of the car
 * @param {float} target_angle  :the angle that you want to keep
 * @return {*}
 */
void startgostraight(float target_angle)
{
    Coord.Target = target_angle;
    xTimerStart(line_walking_Handle, 1);
}

/**
 * @description: start to turn left or turn right
 * @param {int} i car only turn left when the value of its only param "i" is equal to 1,turn right when it's equal to zero
 * @return {*}
 */
void start_trun(int i)
{
    float angle;
    angle = (float)Angle.z / 32768 * 180;
    if (i == 1)
        Turn_Angle_PID.Target = angle - 90;
    else if (i == 0)
        Turn_Angle_PID.Target = angle + 90;
    already_turned = 0;
    xTimerStart(Turn_Angle_Handle, 0);
    xTimerStart(Car_Running_Handle, 0);
}
