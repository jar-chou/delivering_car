#include "my_tasks.h"

#include "software_timer_tasks.h"

#ifdef __CC_ARM
#include "OLED.h"
#elif __clang__
#include "oled_draw.h"
#include "oled_buffer.h"
#endif

#include "stdio.h"
#include "PWM.h"
#include "software_usart.h"

#include "sys.h"

/******************************* 宏定义 ************************************/
#define LEFT_ANGLE -90.
#define orginial_ANGLE 0.
#define LEFT_AREA_DISTANCE 175
#define RIGHT_AREA_DISTANCE 185
#define MIDDLE_AREA_DISTANCE 180

// the declaration of extern variable
extern struct angle Angle;  // the angle of the car
extern int32_t angle_speed; // this declare is used to show the angle speed of the car at oled
// the definition of local variable

// the definition of global variable
TaskHandle_t OLED_SHOW_Handle = NULL;     //+OLDE显示句柄
TaskHandle_t AppTaskCreate_Handle = NULL; //+创建任务句柄
TaskHandle_t Task__ONE_Handle = NULL;     //+任务1句柄
TaskHandle_t Task__TWO_Handle = NULL;     //+任务2句柄
TaskHandle_t Get_Start_Handle = NULL;     //+任务3句柄
TaskHandle_t Task__FOUR_Handle = NULL;    //+任务4句柄
TaskHandle_t Task__FIVE_Handle = NULL;    //+任务4句柄

// const variable
const u8 voice[5][6] = {{0xaa, 0x07, 0x02, 0x00, 0x01, 0xb4},
                        {0xaa, 0x07, 0x02, 0x00, 0x02, 0xb5},
                        {0xaa, 0x07, 0x02, 0x00, 0x03, 0xb6},
                        {0xaa, 0x07, 0x02, 0x00, 0x04, 0xb7},
                        {0xaa, 0x07, 0x02, 0x00, 0x05, 0xb8}};

// the following code is the definition of function

/**
 * the following code is about the task that we create
 */

// extern struct PID Coord, Turn_Angle_PID, X_Speed_PID, Y_Speed_PID, X_Base_On_Laser_PID, Y_Base_On_Laser_PID;
/**
 * @description: this task is is used to show necessary information on the OLED
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
void OLED_SHOW(void *pvParameters)
{

    while (1)
    {
#ifdef __clang__
        ClearScreenBuffer(0x00);
#endif

        char buff[20];
        float current_angle = (float)Angle.z / 32768 * 180;
        sprintf(buff, "Angle_Z:%.2f", current_angle);
#ifdef __CC_ARM
        OLED_ShowString(1, 1, buff);
#elif __clang__
        DrawString(0, 0, buff);
#endif

        sprintf(buff, "Dis:%5d", position_of_car[0]);
#ifdef __CC_ARM
        OLED_ShowString(2, 1, buff);
#elif __clang__
        DrawString(1, 0, buff);
#endif

        sprintf(buff, "R:%4d,f:%4d", (int)VOFA_Data[2], (int)VOFA_Data[3]);
#ifdef __CC_ARM
        OLED_ShowString(3, 1, buff);
#elif __clang__
        DrawString(2, 0, buff);
#endif

        sprintf(buff, "a_a=%.0f e:%3d", Turn_Angle_PID.Target, angle_speed);
#ifdef __CC_ARM
        OLED_ShowString(4, 1, buff);
#elif __clang__
        DrawString(3, 0, buff);
        UpdateScreenDisplay();
#endif

        vTaskDelay(70);
    }
}

/**
 * @description: this task is used to control loudspeaker
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
void Get_Start(void *parameter)
{
    while (1)
    {

        //     USART_Send(voice[0], 6); // 播报打开仓库门
        //     vTaskDelay(5000);        /* 延时500个tick */
        //     USART_Send(voice[1], 6); // 播报到一号仓库
        //     vTaskDelay(5000);        /* 延时500个tick */
        //     USART_Send(voice[2], 6); // 播报到达二号仓库
        //     vTaskDelay(5000);        /* 延时500个tick */

        // startgostraight(0);
        // vTaskDelay(1000);
        // startStraight_Line_For_Laser(200, pan);
        // while(!X_have_achieved)
        //     vTaskDelay(20);
        // while(1)
        //     vTaskDelay(1000);

        // while (1)
        // {
        //     if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == 1)
        //     {
        //         vTaskDelay(50);
        //         if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == 1)
        //         {
        //             break;
        //         }
        //     }
        //     vTaskDelay(50);
        // }
        // 重置陀螺仪偏航角
        // sendcmd(YAWCMD);
        vTaskDelay(200);
        // 打开两个舵机
        SetCompare1(TIM8, 1820, 1);
        SetCompare1(TIM8, 500, 2);

        USART_Send(voice[0], 6); // 播报打开仓库门

        vTaskDelay(2000);
        // 关闭两个舵机
        SetCompare1(TIM8, 920, 1);
        SetCompare1(TIM8, 1520, 2);
        vTaskDelay(600);
        if (check_whetherCharArrayInRange(dataFromLinux, 4, '1', '2')) //! switch the task that base on qrcode
            vTaskResume(Task__ONE_Handle);
        else
            vTaskResume(Task__FIVE_Handle);
        vTaskDelete(Get_Start_Handle);
    }
}

/**
 * @description: this task is including the main logic of the program
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
void Task__ONE(void *parameter)
{

    // char qrcode=0x07;
    while (1)
    {

        startStraight_Line_Base_On_Encoder(13000, forward); // 根据编码器向前走到t型路口
        startgostraight(0);                                 // 保证走直线
        while (!Y_have_achieved)                            // 检测到达位置
            vTaskDelay(20);

        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1); // 小车停止移动
        xTimerStop(Car_Running_Handle, 1);                // 小车停止移动
        xTimerStop(line_walking_Handle, 1);               // 停止走直线
        start_trun(1);                                    // 左转
        while (!already_turned)                           // 等待转弯完成
            vTaskDelay(20);

        // here we need to switch the task that first go to the red area in the right hand side or in the left hand side

        if (dataFromLinux[1] == '2') //! the code in here is unfinished,we need to judge the value of the dataFromLinux[0] to decide which task we should switch to
            vTaskResume(Task__TWO_Handle);
        else if (dataFromLinux[1] == '1')
            vTaskResume(Task__FOUR_Handle);

        vTaskDelete(Task__ONE_Handle);
        while (1)
        {
            vTaskDelay(1000);
        }
    }
}

/**
 * @description: if the car don't got the qrcode data from linux upper previously,this task will be resumed,this task will make the car stop in the purple area to wait for the qrcode data from linux upper
 *              after the car got the qrcode data from linux upper,this task will decide which area the car should go to firstly refering to qrcode data,left or right?
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
void Task__FIVE(void *parameter)
{
    // char qrcode=0x07;
    while (1)
    {

        startStraight_Line_Base_On_Encoder(9500, forward); // 根据编码器向前走到t型路口
        startgostraight(0);                                // 保证走直线
        while (!Y_have_achieved)                           // 检测到达位置
            vTaskDelay(20);

        while (!check_whetherCharArrayInRange(dataFromLinux, 4, '1', '2')) // 用rgb颜色识别检测到达红色区域
            vTaskDelay(20);
        startStraight_Line_Base_On_Encoder(2750, forward); // 根据编码器向前走到t型路口
        startgostraight(0);                                // 保证走直线
        while (!Y_have_achieved)                           // 检测到达位置
            vTaskDelay(20);

        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1); // 小车停止移动
        xTimerStop(Car_Running_Handle, 1);                // 小车停止移动
        xTimerStop(line_walking_Handle, 1);               // 停止走直线
        start_trun(1);                                    // 左转
        while (!already_turned)                           // 等待转弯完成
            vTaskDelay(20);

        // here we need to switch the task that first go to the red area in the right hand side or in the left hand side

        if (dataFromLinux[1] == '2') //! the code in here is unfinished,we need to judge the value of the dataFromLinux[0] to decide which task we should switch to
            vTaskResume(Task__TWO_Handle);
        else if (dataFromLinux[1] == '1')
            vTaskResume(Task__FOUR_Handle);

        vTaskDelete(Task__FIVE_Handle);
        while (1)
        {
            vTaskDelay(1000);
        }
    }
}

#if 0
/**
 * @description: this task is used to control the car,make it go to the red area in the right hand side firstly,then go to the red area in the left hand side
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
void Task__TWO(void *parameter)
{
    while (1)
    {

        startStraight_Line_For_Laser(140, pan); //! the distance need to be changed,it is because only the center of road do not have barrier
        startgostraight(LEFT_ANGLE);            // 保证车的方向不变
        while (!X_have_achieved)
        {
            vTaskDelay(20);
        }

        startStraight_Line_Base_On_Encoder(-6500, forward); // 向后走到右边红色区域
        startgostraight(LEFT_ANGLE);                        // 保证走直线
        while (!Y_have_achieved)                            // 用rgb颜色识别检测到达红色区域
            vTaskDelay(20);

        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1); // 停止前后走

        startStraight_Line_For_Laser(140, pan); //! the distance need to be changed,it is because only the center of road do not have barrier

        while (!X_have_achieved)
        {
            vTaskDelay(20);
        }

        // 打开仓库，取出物品，
        if (dataFromLinux[0] == '1') // 打开一号仓库
        {
            SetCompare1(TIM8, 1820, 1);
        }
        else if (dataFromLinux[0] == '2') // 打开二号仓库
        {
            SetCompare1(TIM8, 500, 2);
        }
        // 播报到达二号收货点
        USART_Send(voice[2], 6);
        vTaskDelay(2000); // 等待取出货物
        SetCompare1(TIM8, 920, 1);
        SetCompare1(TIM8, 1520, 2);
        vTaskDelay(600);

        xTimerStop(Car_Running_Handle, 1);                // 小车停止移动
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1); // 停止前后走
        xTimerStop(Turn_Angle_Handle, 1);
        xTimerStop(Achieve_Distance_For_Front_Laser_Handle, 1);
        xTimerStop(Achieve_Distance_For_Right_Laser_Handle, 1);
        xTimerStop(Pan_Left_Base_On_Encoder_Handle, 1);
        // 向前走到左边红色区域
        startStraight_Line_Base_On_Encoder(13000, forward); // go forward to the red area in the left hand side
        startgostraight(LEFT_ANGLE);                        // 保证车的方向不变
        vTaskDelay(1500);
        startStraight_Line_Base_On_Encoder(-350, pan); //! code is unfinished,the first param need to be changed
        while (!Y_have_achieved)                       // 检测到达位置
            vTaskDelay(20);

        startStraight_Line_For_Laser(175, pan);          // 根据右边激光测距调整距离
        startStraight_Line_For_Laser(150, forward);      // 根据前面激光测距调整距离
        while ((!X_have_achieved) || (!Y_have_achieved)) // 检测到达位置
            vTaskDelay(20);

        // 打开仓库，取出物品，
        if (dataFromLinux[2] == '1') // 打开一号仓库
        {
            SetCompare1(TIM8, 1820, 1);
        }
        else if (dataFromLinux[2] == '2') // 打开二号仓库
        {
            SetCompare1(TIM8, 500, 2);
        }
        // 播报到达一号收货点
        USART_Send(voice[3], 6);
        vTaskDelay(2000); // 等待取出货物
        SetCompare1(TIM8, 920, 1);
        SetCompare1(TIM8, 1520, 2);
        vTaskDelay(600);

        startgostraight(LEFT_ANGLE);            // 保证走直线
        startStraight_Line_For_Laser(210, pan); // 根据右边激光测距调整距离
        while (!X_have_achieved)                // 检测到达位置
            vTaskDelay(20);

        startStraight_Line_Base_On_Encoder(-6300, forward); // 向后走到t字路口

        startgostraight(LEFT_ANGLE); // 保证走直线
        while (!Y_have_achieved)     // 检测到达位置
            vTaskDelay(20);

        xTimerStop(line_walking_Handle, 1); // 停止走直线
        xTimerStop(Car_Running_Handle, 1);  // 小车停止移动

        // 开始右转
        start_trun(0);          // 右转
        while (!already_turned) // 等待转弯完成
            vTaskDelay(20);

        // to sure the car is in the center of the road
        startgostraight(orginial_ANGLE);                 // 保证走直线
        startStraight_Line_For_Laser(1170, pan);         // 根据右边激光测距调整距离
        startStraight_Line_For_Laser(240, forward);      // 根据前面激光测距调整距离
        while ((!X_have_achieved) || (!Y_have_achieved)) // 检测到达位置
            vTaskDelay(20);

        startStraight_Line_Base_On_Encoder(-12500, forward); // 向后走到右边黄色区域
        startgostraight(orginial_ANGLE);                     // 保证走直线
        vTaskDelay(1650);
        startStraight_Line_Base_On_Encoder(-400, pan); // 向左移动小车调整距离 //! code is unfinished,the first param need to be changed
        while (!Y_have_achieved)                       // 检测到达位置
            vTaskDelay(20);
        startStraight_Line_For_Laser(1200, pan); // 根据右边激光测距调整距离
        while (!X_have_achieved)                 // 检测到达位置
            vTaskDelay(20);
        // 小车停止移动
        xTimerStop(Car_Running_Handle, 1);
        xTimerStop(line_walking_Handle, 1);

        PULL_High();
        while (1) // 任务完成，挂机
        {
            vTaskDelay(1000);
        }
    }
}

/**
 * @description: this task is used to control the car,make it go to the red area in the left hand side firstly,then go to the red area in the right hand side
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
void Task__FOUR(void *parameter)
{
    while (1)
    {

        startStraight_Line_For_Laser(220, pan); //! the distance need to be changed,it is because only the center of road do not have barrier
        startgostraight(LEFT_ANGLE);            // 保证车的方向不变
        while (!X_have_achieved)
        {
            vTaskDelay(20);
        }
        xTimerStop(line_walking_Handle, 1);               // 停止走直线
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1); // 小车停止移动

        startStraight_Line_Base_On_Encoder(5500, forward); // 向前走到左边红色区域
        startgostraight(LEFT_ANGLE);                       // 保证走直线

        while (!Y_have_achieved) // 检测到达左边红色区域
            vTaskDelay(10);

        startStraight_Line_For_Laser(175, pan);          // 根据右边激光测距调整距离
        startStraight_Line_For_Laser(150, forward);      // 根据前面激光测距调整距离
        while ((!X_have_achieved) || (!Y_have_achieved)) // 检测到达位置
            vTaskDelay(10);

        // 打开仓库，取出物品，
        if (dataFromLinux[0] == '1') // 打开一号仓库
        {
            SetCompare1(TIM8, 1820, 1);
        }
        else if (dataFromLinux[0] == '2') // 打开二号仓库
        {
            SetCompare1(TIM8, 500, 2);
        }
        // 播报到达一号收货点
        USART_Send(voice[1], 6);
        vTaskDelay(2000); // 等待取出货物
        SetCompare1(TIM8, 920, 1);
        SetCompare1(TIM8, 1520, 2);
        vTaskDelay(600);

        startgostraight(LEFT_ANGLE);            // 保证走直线
        startStraight_Line_For_Laser(250, pan); // 根据右边激光测距调整距离
        while (!X_have_achieved)                // 检测到达位置
            vTaskDelay(20);

        startStraight_Line_Base_On_Encoder(-13000, forward); // 向后走到左边红色区域
        vTaskDelay(1500);                                    // 延时1.5s，否则会检测到左边红色区域
        startStraight_Line_Base_On_Encoder(350, pan);        // 根据右边激光测距调整距离
        while (!Y_have_achieved)                             // 用rgb颜色识别检测到达右边红色区域
            vTaskDelay(20);                                  // 延时20ms

        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1); // 停止前后走
        PULL_High();

        startStraight_Line_For_Laser(175, pan); // 根据右边激光测距调整距离
        while (!X_have_achieved)                // 检测到达位置
            vTaskDelay(20);

        // 打开仓库，取出物品，
        if (dataFromLinux[2] == '1') // 打开一号仓库
        {
            SetCompare1(TIM8, 1820, 1);
        }
        else if (dataFromLinux[2] == '2') // 打开二号仓库
        {
            SetCompare1(TIM8, 500, 2);
        }
        // 播报到达二号收货点
        USART_Send(voice[4], 6);
        vTaskDelay(2000); // 等待取出货物
        SetCompare1(TIM8, 920, 1);
        SetCompare1(TIM8, 1520, 2);
        vTaskDelay(600);

        startStraight_Line_Base_On_Encoder(6100, forward); //! 向前走到十字t字路口 code is unfinished,the first param need to be changed
        startgostraight(LEFT_ANGLE);                       // 保证走直线
        while (!Y_have_achieved)                           // 检测到达位置
            vTaskDelay(20);

        xTimerStop(line_walking_Handle, 1); // 停止走直线
        // 开始右转
        start_trun(0);          // 右转
        while (!already_turned) // 等待转弯完成
            vTaskDelay(20);

        // to sure the car is in the center of the road
        startgostraight(orginial_ANGLE);                 // 保证走直线
        startStraight_Line_For_Laser(1170, pan);         // 根据右边激光测距调整距离
        startStraight_Line_For_Laser(240, forward);      // 根据前面激光测距调整距离
        while ((!X_have_achieved) || (!Y_have_achieved)) // 检测到达位置
            vTaskDelay(20);

        startStraight_Line_Base_On_Encoder(-12500, forward); // 向后走到右边黄色区域
        startgostraight(orginial_ANGLE);                     // 保证走直线
        vTaskDelay(1650);
        startStraight_Line_Base_On_Encoder(-400, pan); // 向左移动小车调整距离 //! code is unfinished,the first param need to be changed
        while (!Y_have_achieved)                       // 检测到达位置
            vTaskDelay(20);
        startStraight_Line_For_Laser(1200, pan); // 根据右边激光测距调整距离
        while (!X_have_achieved)                 // 检测到达位置
            vTaskDelay(20);
        // 小车停止移动
        xTimerStop(Car_Running_Handle, 1);
        xTimerStop(line_walking_Handle, 1);
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);

        PULL_High(); // 挂机 it is necessary to add the code that make the direction pin of the motor high
        while (1)
        {
            vTaskDelay(1000);
        }
    }
}
#else

/**
 * @description: this task is used to control the car,make it go to the red area in the right hand side firstly,then go to the red area in the left hand side
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
void Task__TWO(void *parameter)
{
    while (1)
    {

        startStraight_Line_For_Laser(MIDDLE_AREA_DISTANCE, pan); //! the distance need to be changed,it is because only the center of road do not have barrier
        startgostraight(LEFT_ANGLE);                             // 保证车的方向不变
        while (!X_have_achieved)
        {
            vTaskDelay(20);
        }

        startStraight_Line_Base_On_Encoder(-6500, forward); // 向后走到右边红色区域
        startgostraight(LEFT_ANGLE);                        // 保证走直线
        while (!Y_have_achieved)                            // 用rgb颜色识别检测到达红色区域
            vTaskDelay(20);

        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1); // 停止前后走

        startStraight_Line_For_Laser(RIGHT_AREA_DISTANCE, pan); //! the distance need to be changed,it is because only the center of road do not have barrier

        while (!X_have_achieved)
        {
            vTaskDelay(20);
        }

        // 打开仓库，取出物品，
        if (dataFromLinux[0] == '1') // 打开一号仓库
        {
            SetCompare1(TIM8, 1820, 1);
        }
        else if (dataFromLinux[0] == '2') // 打开二号仓库
        {
            SetCompare1(TIM8, 500, 2);
        }
        // 播报到达二号收货点
        USART_Send(voice[2], 6);
        vTaskDelay(2000); // 等待取出货物
        SetCompare1(TIM8, 920, 1);
        SetCompare1(TIM8, 1520, 2);
        vTaskDelay(600);

        xTimerStop(Car_Running_Handle, 1);                // 小车停止移动
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1); // 停止前后走
        xTimerStop(Turn_Angle_Handle, 1);
        xTimerStop(Achieve_Distance_For_Front_Laser_Handle, 1);
        xTimerStop(Achieve_Distance_For_Right_Laser_Handle, 1);
        xTimerStop(Pan_Left_Base_On_Encoder_Handle, 1);
        // 向前走到左边红色区域
        startStraight_Line_Base_On_Encoder(13000, forward); // go forward to the red area in the left hand side
        startgostraight(LEFT_ANGLE);                        // 保证车的方向不变
        vTaskDelay(1500);
        while (!Y_have_achieved) // 检测到达位置
            vTaskDelay(20);

        startStraight_Line_For_Laser(LEFT_AREA_DISTANCE, pan); // 根据右边激光测距调整距离
        startStraight_Line_For_Laser(150, forward);            // 根据前面激光测距调整距离
        while ((!X_have_achieved) || (!Y_have_achieved))       // 检测到达位置
            vTaskDelay(20);

        // 打开仓库，取出物品，
        if (dataFromLinux[2] == '1') // 打开一号仓库
        {
            SetCompare1(TIM8, 1820, 1);
        }
        else if (dataFromLinux[2] == '2') // 打开二号仓库
        {
            SetCompare1(TIM8, 500, 2);
        }
        // 播报到达一号收货点
        USART_Send(voice[3], 6);
        vTaskDelay(2000); // 等待取出货物
        SetCompare1(TIM8, 920, 1);
        SetCompare1(TIM8, 1520, 2);
        vTaskDelay(600);

        startgostraight(LEFT_ANGLE);                           // 保证走直线
        startStraight_Line_For_Laser(LEFT_AREA_DISTANCE, pan); // 根据右边激光测距调整距离
        while (!X_have_achieved)                               // 检测到达位置
            vTaskDelay(20);

        startStraight_Line_Base_On_Encoder(-6300, forward); // 向后走到t字路口

        startgostraight(LEFT_ANGLE); // 保证走直线
        while (!Y_have_achieved)     // 检测到达位置
            vTaskDelay(20);

        xTimerStop(line_walking_Handle, 1); // 停止走直线
        xTimerStop(Car_Running_Handle, 1);  // 小车停止移动

        // 开始右转
        start_trun(0);          // 右转
        while (!already_turned) // 等待转弯完成
            vTaskDelay(20);

        // to sure the car is in the center of the road
        startgostraight(orginial_ANGLE);                 // 保证走直线
        startStraight_Line_For_Laser(1195, pan);         // 根据右边激光测距调整距离
        startStraight_Line_For_Laser(240, forward);      // 根据前面激光测距调整距离
        while ((!X_have_achieved) || (!Y_have_achieved)) // 检测到达位置
            vTaskDelay(20);

        startStraight_Line_Base_On_Encoder(-12800, forward); // 向后走到右边黄色区域
        startgostraight(orginial_ANGLE);                     // 保证走直线
        vTaskDelay(1650);
        while (!Y_have_achieved) // 检测到达位置
            vTaskDelay(20);
        startStraight_Line_For_Laser(1200, pan); // 根据右边激光测距调整距离
        while (!X_have_achieved)                 // 检测到达位置
            vTaskDelay(20);
        // 小车停止移动
        xTimerStop(Car_Running_Handle, 1);
        xTimerStop(line_walking_Handle, 1);

        PULL_High();
        while (1) // 任务完成，挂机
        {
            vTaskDelay(1000);
        }
    }
}

/**
 * @description: this task is used to control the car,make it go to the red area in the left hand side firstly,then go to the red area in the right hand side
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
void Task__FOUR(void *parameter)
{
    while (1)
    {

        startStraight_Line_For_Laser(MIDDLE_AREA_DISTANCE, pan); //! the distance need to be changed,it is because only the center of road do not have barrier
        startgostraight(LEFT_ANGLE);                             // 保证车的方向不变
        while (!X_have_achieved)
        {
            vTaskDelay(20);
        }
        xTimerStop(line_walking_Handle, 1);               // 停止走直线
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1); // 小车停止移动

        startStraight_Line_Base_On_Encoder(6200, forward); // 向前走到左边红色区域
        startgostraight(LEFT_ANGLE);                       // 保证走直线

        while (!Y_have_achieved) // 检测到达左边红色区域
            vTaskDelay(10);

        startStraight_Line_For_Laser(LEFT_AREA_DISTANCE, pan); // 根据右边激光测距调整距离
        startStraight_Line_For_Laser(150, forward);            // 根据前面激光测距调整距离
        while ((!X_have_achieved) || (!Y_have_achieved))       // 检测到达位置
            vTaskDelay(10);

        // 打开仓库，取出物品，
        if (dataFromLinux[0] == '1') // 打开一号仓库
        {
            SetCompare1(TIM8, 1820, 1);
        }
        else if (dataFromLinux[0] == '2') // 打开二号仓库
        {
            SetCompare1(TIM8, 500, 2);
        }
        // 播报到达一号收货点
        USART_Send(voice[1], 6);
        vTaskDelay(2000); // 等待取出货物
        SetCompare1(TIM8, 920, 1);
        SetCompare1(TIM8, 1520, 2);
        vTaskDelay(600);

        startgostraight(LEFT_ANGLE); // 保证走直线

        startStraight_Line_Base_On_Encoder(-13000, forward); // 向后走到右边红色区域
        vTaskDelay(1500);                                    // 延时1.5s，否则会检测到左边红色区域
        while (!Y_have_achieved)                             // 用rgb颜色识别检测到达右边红色区域
            vTaskDelay(20);                                  // 延时20ms

        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1); // 停止前后走
        PULL_High();

        startStraight_Line_For_Laser(RIGHT_AREA_DISTANCE, pan); // 根据右边激光测距调整距离
        while (!X_have_achieved)                                // 检测到达位置
            vTaskDelay(20);

        // 打开仓库，取出物品，
        if (dataFromLinux[2] == '1') // 打开一号仓库
        {
            SetCompare1(TIM8, 1820, 1);
        }
        else if (dataFromLinux[2] == '2') // 打开二号仓库
        {
            SetCompare1(TIM8, 500, 2);
        }
        // 播报到达二号收货点
        USART_Send(voice[4], 6);
        vTaskDelay(2000); // 等待取出货物
        SetCompare1(TIM8, 920, 1);
        SetCompare1(TIM8, 1520, 2);
        vTaskDelay(600);

        startStraight_Line_Base_On_Encoder(6100, forward); //! 向前走到十字t字路口 code is unfinished,the first param need to be changed
        startgostraight(LEFT_ANGLE);                       // 保证走直线
        while (!Y_have_achieved)                           // 检测到达位置
            vTaskDelay(20);

        xTimerStop(line_walking_Handle, 1); // 停止走直线
        // 开始右转
        start_trun(0);          // 右转
        while (!already_turned) // 等待转弯完成
            vTaskDelay(20);

        // to sure the car is in the center of the road
        startgostraight(orginial_ANGLE);                 // 保证走直线
        startStraight_Line_For_Laser(1195, pan);         // 根据右边激光测距调整距离
        startStraight_Line_For_Laser(240, forward);      // 根据前面激光测距调整距离
        while ((!X_have_achieved) || (!Y_have_achieved)) // 检测到达位置
            vTaskDelay(20);

        startStraight_Line_Base_On_Encoder(-12800, forward); // 向后走到右边黄色区域
        startgostraight(orginial_ANGLE);                     // 保证走直线
        vTaskDelay(1650);
        while (!Y_have_achieved) // 检测到达位置
            vTaskDelay(20);
        startStraight_Line_For_Laser(1200, pan); // 根据右边激光测距调整距离
        while (!X_have_achieved)                 // 检测到达位置
            vTaskDelay(20);
        // 小车停止移动
        xTimerStop(Car_Running_Handle, 1);
        xTimerStop(line_walking_Handle, 1);
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);

        PULL_High(); // 挂机 it is necessary to add the code that make the direction pin of the motor high
        while (1)
        {
            vTaskDelay(1000);
        }
    }
}

#endif