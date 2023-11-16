#include "software_timer_tasks.h"
#include "sys.h"
#include <string.h>
#include "encoder.h"
#include "my_math.h"
#include "mecanum.h"

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
    u16 F_OUT;
    u16 R_OUT;
} Distance;

#define maximum_Y_accelerate_rate 100
int16_t maximum_Y_speed = 1900;

mecanum_car_speed_t car_speed = {.y_speed = 0, .x_speed = 0, .w_speed = 0};
int32_t position_of_car[3] = {0};
int32_t speed_of_car[3] = {0};
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
    if ((absolute_error > 100) || (absolute_error < 10)) // 如果距离超过100mm，就不要再跑了
    {
        taskENTER_CRITICAL();
        car_speed.x_speed = 0;
        taskEXIT_CRITICAL();
        return;
    }
    int32_t speed = (int32_t)PID_Realize(&X_Base_On_Laser_PID, distance);
    // limit the range of Y_Speed
    VAL_LIMIT(speed, -450, 450);

    // assign the value of pid output to global variable
    taskENTER_CRITICAL();
    car_speed.x_speed = speed;
    taskEXIT_CRITICAL();
}

/**
 * @description: this function is the software callback function that achieve pan left base on encoder num
 * @return {*}
 */
void Pan_Left_Base_On_Encoder(TimerHandle_t xTimer)
{
    static int i = 0, timeout = 0;
    int32_t distance = position_of_car[1];
    int32_t speed = -(int32_t)PID_Realize(&X_Speed_PID, distance);

    // limit the range of X_Speed
    VAL_LIMIT(speed, -500, 500);

    // assign the value of pid output to global variable
    taskENTER_CRITICAL();
    car_speed.x_speed = speed;
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
    static int i = 0, timeout = 0;
    int32_t distance = position_of_car[0];
    int32_t speed = -(int32_t)PID_Realize(&Y_Speed_PID, distance);
    static int32_t last_speed = 0;

    // limit the range of Y_Speed
    VAL_LIMIT(speed, -maximum_Y_speed, maximum_Y_speed);

    // limit the accelerate rate of Y_Speed
    if (speed > 0)
    {
        if (speed - last_speed > maximum_Y_accelerate_rate)
            speed = last_speed + maximum_Y_accelerate_rate;
    }
    else
    {
        if (speed - last_speed < -maximum_Y_accelerate_rate)
            speed = last_speed - maximum_Y_accelerate_rate;
    }
    last_speed = speed;

    // assign the value of pid output to global variable
    taskENTER_CRITICAL();
    car_speed.y_speed = speed;
    taskEXIT_CRITICAL();
    if (fabs(distance - Y_Speed_PID.Target) < 20)
    {
        i++;
        if (i > 2)
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
    static int i = 0, timeout = 0;
    float distance = VOFA_Data[2];
    int32_t speed = (int32_t)PID_Realize(&X_Base_On_Laser_PID, distance);

    // limit the range of X_Speed
    VAL_LIMIT(speed, -240, 240);

    // assign the value of pid output to global variable
    taskENTER_CRITICAL();
    car_speed.x_speed = speed;
    taskEXIT_CRITICAL();

    if (fabs(distance - X_Base_On_Laser_PID.Target) < 2.1)
    {
        i++;
        if (i > 15)
        {
            i = 0;
            xTimerStop(Achieve_Distance_For_Right_Laser_Handle, 0);
            X_have_achieved = 1;
        }
    }
    if (speed_of_car[2] == 0)
    {
        timeout++;
        if (timeout > 50)
        {
            timeout = 0;
            xTimerStop(Achieve_Distance_For_Right_Laser_Handle, 0);
            X_have_achieved = 1;
        }
    }
    else
        timeout = 0;
}

/**
 * @description: this function is the software callback function that achieve reach the target distance for front laser
 * @return {*}
 */
void Achieve_Distance_Front_Head_Laser(TimerHandle_t xTimer)
{
    static int i = 0, timeout = 0;
    float distance = VOFA_Data[3];
    int32_t speed = (int32_t)PID_Realize(&Y_Base_On_Laser_PID, distance);

    // limit the range of Y_Speed
    VAL_LIMIT(speed, -200, 200);

    // assign the value of pid output to global variable
    taskENTER_CRITICAL();
    car_speed.y_speed = speed;
    taskEXIT_CRITICAL();

    if (fabs(distance - Y_Base_On_Laser_PID.Target) < 10)
    {
        i++;
        if (i > 5)
        {
            i = 0;
            xTimerStop(Achieve_Distance_For_Front_Laser_Handle, 0);
            Y_have_achieved = 1;
        }
    }
    if (speed_of_car[0] == 0)
    {
        timeout++;
        if (timeout > 50)
        {
            timeout = 0;
            xTimerStop(Achieve_Distance_For_Front_Laser_Handle, 0);
            Y_have_achieved = 1;
        }
    }
    else
        timeout = 0;
}

/**
 * @description: this function is the software callback function that achieve turn angle
 * @return {*}
 */
void Turn_Angle(TimerHandle_t xTimer)
{
    static u8 i = 0, timeout = 0;
    float angle;
    int PIDOUT;
    // get the angle of the car, and calculate the pid output
    angle = (float)Angle.z / 32768 * 180;
    PIDOUT = -PID_Realize_angle(&Turn_Angle_PID, angle);

    // limit the range of pid output
    VAL_LIMIT(PIDOUT, -1300, 1300);

    // assign the value of pid output to global variable
    taskENTER_CRITICAL();
    car_speed.w_speed = (int32_t)PIDOUT;
    taskEXIT_CRITICAL();

    // if the car have turned, then stop this timer
    if ((fabs(angle - Turn_Angle_PID.Target) < 1) || (fabs(angle - Turn_Angle_PID.Target) > 359))
    {
        i++;
        if (i > 4)
        {
            i = 0;
            xTimerStop(Turn_Angle_Handle, 0);
            already_turned = 1;
        }
    }
    if (speed_of_car[2] == 0)
    {
        timeout++;
        if (timeout > 50)
        {
            timeout = 0;
            xTimerStop(Turn_Angle_Handle, 0);
            already_turned = 1;
        }
    }
    else
        timeout = 0;
}

/**
 * @description: this function is the software callback function that keep the car walking straightly
 * @return {*}
 */
void line_walking(TimerHandle_t xTimer)
{
    float angle;
    int PIDOUT;
    //? PID开始巡线
    angle = (float)Angle.z / 32768 * 180;
    // angle_speed = -PID_Realize(&Coord, angle);

    // !code in here need to test
    PIDOUT = -PID_Realize_angle(&Coord, angle);

    // limit the range of pid output
    VAL_LIMIT(PIDOUT, -500, 500);

    // assign the value of pid output to global variable
    taskENTER_CRITICAL();
    car_speed.w_speed = PIDOUT;
    taskEXIT_CRITICAL();
    return;
}

/**
 * @description: this function is the software callback function that calculates the ccr register value of the wheel
 * @return {*}
 */
void Car_Running(TimerHandle_t xTimer)
{
    taskENTER_CRITICAL(); // enter critical zone and operate global variable

    // get the value of global variable
    mecanum_car_speed_t local_car_speed = {.y_speed = car_speed.y_speed, .x_speed = car_speed.x_speed, .w_speed = car_speed.w_speed};

    // clear the value of global variable
    car_speed.y_speed = 0;
    car_speed.x_speed = 0;
    car_speed.w_speed = 0;

    taskEXIT_CRITICAL(); // exit critical zone

    int32_t CCR_wheel[4] = {0};
    // calculate the ccr register value of the wheel
    mecanum_calculate(&local_car_speed, CCR_wheel);

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
 * @description: this function is the software callback function that send data to upper computer
 * @return {*}
 */
void sendto_Upper(TimerHandle_t xTimer)
{

    VOFA_Data[0] = (float)Angle.z / 32768 * 180;
    VOFA_Send_float(VOFA_Data, 4); //! 发送数据给VOFA
}

/**
 * @description: this function is the software callback function that analyse data
 * @return {*}
 */
void analyse_data(void)
{
    u8 check_sum;
    const u8 TOFSENSE[] = {0x55, 0x53};
    VL53_Send_Agrement();
    VL53_Send_Agrement();
    Read_buff_Void(&U2_buffer, TOFSENSE, 2, Angle.data, 9, 8, 0);
    check_sum = (0x55 + 0x53 + Angle.data[5] + Angle.data[4] + Angle.data[7] + Angle.data[6]);
    if (Angle.data[8] == check_sum)
    {
        Angle.z = -((Angle.data[5] << 8) + Angle.data[4]);
        // printf("%.2f\r\n", (float)Angle.z / 32768 * 180);
    }

    // get the motor encoder value
    static int32_t How_many_revolutions_of_the_motor[4] = {0, 0, 0, 0};
    How_many_revolutions_of_the_motor[0] = Read_Encoder(2);
    How_many_revolutions_of_the_motor[1] = Read_Encoder(3);
    How_many_revolutions_of_the_motor[2] = Read_Encoder(4);
    How_many_revolutions_of_the_motor[3] = Read_Encoder(5);

    u8 get_howmanyencoder = 0;
    for (u8 n = 0; n < 4; n++)
    {
        if (abs(How_many_revolutions_of_the_motor[n]) > 1)
            get_howmanyencoder++;
    }

    // calculate the distance that car have gone based on the encoder value
    int32_t The_distance_that_has_gone_forward = 0;
    if (get_howmanyencoder != 0)
        The_distance_that_has_gone_forward = (How_many_revolutions_of_the_motor[0] + How_many_revolutions_of_the_motor[1] + How_many_revolutions_of_the_motor[2] + How_many_revolutions_of_the_motor[3]) / get_howmanyencoder;
    int32_t The_distance_that_has_gone_right_head_side = (How_many_revolutions_of_the_motor[1] - How_many_revolutions_of_the_motor[2]) / 2;
    int32_t The_angle_that_has_turned = (-How_many_revolutions_of_the_motor[0] + How_many_revolutions_of_the_motor[1]) / 2;
    // get data that represents the qrcode from upper computer
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

    // get the distance that car have gone based on the laser value
    const u8 VL53_Agreement_RX[] = {0x50, 0x03, 0x02};
    const u8 RIGHTLASER[] = {0x57, 0x00, 0xFF, 0x01};
    u8 TOF_Data[7] = {0};

    if (have_enough_data(&U3_buffer, 3, 1, 16))
    {
        while (have_enough_data(&U3_buffer, 3, 1, 16))
            Read_buff_Void(&U3_buffer, VL53_Agreement_RX, 3, &Distance.F_OUT, 1, 16, 1);
    }

    if (have_enough_data(&VL53_USARTX_Buff, 3, 1, 16))
    {
        u16 rout = 0;
        while (have_enough_data(&VL53_USARTX_Buff, 3, 1, 16))
        {
            Read_buff_Void(&VL53_USARTX_Buff, VL53_Agreement_RX, 3, &rout, 1, 16, 1);
            if (rout & 0x00FF) //! to judge whether got the full data
                Distance.R_OUT = rout;
        }
        printf("R:%d\r\n", Distance.R_OUT);
    }

    taskENTER_CRITICAL(); // 进入基本临界区

    VOFA_Data[2] = Distance.R_OUT; // 右边激光测距得到的距离
    VOFA_Data[3] = Distance.F_OUT; // 前面激光测距得到的距离
    // Distance.F_OUT = 0;
    // Distance.R_OUT = 0;
    position_of_car[0] += The_distance_that_has_gone_forward;         // the distance that car have gone forward
    position_of_car[1] += The_distance_that_has_gone_right_head_side; // the distance that car have gone right hand side
    position_of_car[2] += The_angle_that_has_turned;                  // the angle that car have turned

    speed_of_car[0] = The_distance_that_has_gone_forward;
    speed_of_car[1] = The_distance_that_has_gone_right_head_side;
    speed_of_car[2] = The_angle_that_has_turned;

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
 *                  cn:根据激光来跑直线
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
        X_Base_On_Laser_PID.Last_Error = target - VOFA_Data[2];
        X_have_achieved = 0;
        xTimerStart(Achieve_Distance_For_Right_Laser_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }
    else if (which_laser == 1) // 根据前面的激光距离来调整距离
    {
        Y_Base_On_Laser_PID.Target = target;
        Y_Base_On_Laser_PID.Cumulation_Error = 0;
        Y_Base_On_Laser_PID.Last_Error = target - VOFA_Data[3];
        Y_have_achieved = 0;
        xTimerStart(Achieve_Distance_For_Front_Laser_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }
}

/**
 * @description:    Follow the encoder and go straight
 *                  cn:根据编码器来跑直线
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
        X_Speed_PID.Last_Error = target - position_of_car[1];
        X_have_achieved = 0;
        position_of_car[1] = 0;
        xTimerStart(Pan_Left_Base_On_Encoder_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }
    else if (forwardOrPan == 1) // 根据编码器向前走
    {
        Y_Speed_PID.Target = target;
        Y_Speed_PID.Cumulation_Error = 0;
        Y_Speed_PID.Last_Error = target - position_of_car[0];
        Y_have_achieved = 0;
        position_of_car[0] = 0;
        xTimerStart(Go_Forward_Base_On_Encoder_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }
    return;
}

/**
 * @description: start to Keep the direction Angle of the car
 *              cn:开始保持车的方向角
 * @param {float} target_angle : the angle that you want to keep
 * @return {*}
 */
void startgostraight(float target_angle)
{
    Coord.Target = target_angle;
    Coord.Cumulation_Error = 0;
    Coord.Last_Error = target_angle - (float)Angle.z / 32768 * 180;
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
