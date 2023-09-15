/*
 *                        _oo0oo_
 *                       o8888888o
 *                       88" . "88
 *                       (| -_- |)
 *                       0\  =  /0
 *                     ___/`---'\___
 *                   .' \\|     |// '.
 *                  / \\|||  :  |||// \
 *                 / _||||| -:- |||||- \
 *                |   | \\\  - /// |   |
 *                | \_|  ''\---/''  |_/ |
 *                \  .-\__  '-'  ___/-. /
 *              ___'. .'  /--.--\  `. .'___
 *           ."" '<  `.___\_<|>_/___.' >' "".
 *          | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *          \  \ `_.   \_ __\ /__ _/   .-` /  /
 *      =====`-.____`.___ \_____/___.-`___.-'=====
 *                        `=---='
 * 
 * 
 *      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 
 *            佛祖保佑     永不宕机     永无BUG
 * 
 * @Author: zhaojianchao and jar-chou 2722642511@qq.com
 * @Date: 2023-09-06 13:02:19
 * @LastEditors: jar-chou 2722642511@qq.com
 * @LastEditTime: 2023-09-15 22:16:26
 * @FilePath: \delivering_car\User\main.c
 * @Description: 
 */




/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"
/* 开发板硬件bsp头文件 */
#include "stdio.h"
#include "bsp_led.h"
#include "usart.h"
#include "bsp_key.h"
#include "bsp_exti.h"
#include "Delay.h"
#include "Sysclk_output.h"
#include "PWM.h"
#include "Driver.h"
#include "software_usart.h"
#include "Software_iic.h"
#include "JY62.h"
#include "buffer.h"
#include "PID.h"
#include "limits.h"
#include "encoder.h"
#include "mpu6050.h"
#include "OLED.h"
#include "VL53.h"
#include "sys.h"
#include <math.h>
#include <stdbool.h>



/**************************** 任务句柄 ********************************/
/*
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */

QueueHandle_t Task_Number_Handle = NULL;

static TimerHandle_t Turn_Angle_Handle = NULL;     			//+发送数据到上位机定时器句柄
static TimerHandle_t sendto_Upper_Handle = NULL;   			//+发送数据到上位机定时器句柄
static TimerHandle_t line_walking_Handle = NULL;   			//+巡线PID定时器句柄
static TimerHandle_t analyse_data_Handle = NULL;   			//+解析数据定时器句柄
static TimerHandle_t Achieve_Distance_For_Front_Laser_Handle = NULL;	//根据激光来跑直线
static TimerHandle_t Achieve_Distance_For_Right_Laser_Handle = NULL;
static TimerHandle_t Car_Running_Handle = NULL;
static TimerHandle_t Go_Forward_Base_On_Encoder_Handle = NULL;
static TimerHandle_t Pan_Left_Base_On_Encoder_Handle = NULL;

static TaskHandle_t OLED_SHOW_Handle = NULL;       			//+OLDE显示句柄
static TaskHandle_t AppTaskCreate_Handle = NULL;   			//+创建任务句柄
static TaskHandle_t Task__ONE_Handle = NULL;       			//+任务1句柄
static TaskHandle_t Task__TWO_Handle = NULL;       			//+任务2句柄
static TaskHandle_t Task__THREE_Handle = NULL;     			//+任务3句柄
static TaskHandle_t Task__FOUR_Handle = NULL;      			//+任务4句柄
static EventGroupHandle_t Group_One_Handle = NULL; 			//+事件组句柄

/******************************* Global variable declaration ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */
#define Initial_Speed 0
#define Turn_Speed 0
extern const char ACCCMD[3];
extern const char YAWCMD[3];
int32_t Y_Speed = Initial_Speed;
int32_t X_Speed = 0;
int32_t angle_speed = 0;
float VOFA_Data[4];
volatile extern struct Buff VL53_USARTX_Buff;
struct PID Coord, Turn_Angle_PID, X_Speed_PID, Y_Speed_PID, X_Base_On_Laser_PID, Y_Base_On_Laser_PID;
u8 already_turned = 0, Y_have_achieved = 0, X_have_achieved = 0;   //是否达到定时器目的的信号量
int32_t CCR_wheel[4]={0,0,0,0};
int32_t position_of_car[3]={0,0,0};
u8 dataFromLinux[4] = {0, 0};       //the data get from linux
u8 voice[3][6] = {0xaa,0x07,0x02,0x00,0x01,0xb4,0xaa,0x07,0x02,0x00,0x02,0xb5,0xaa,0x07,0x02,0x00,0x03,0xb6};
struct distance
{
    u16 F[5];
    int R[5];
    u16 F_OUT;
    int R_OUT;
} Distance;

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void analyse_data(void);
static void line_walking(void);
static void AppTaskCreate(void);           	/* 用于创建任务 */
static void Task__TWO(void *pvParameters); 	/* Test_Task任务实现 */
static void OLED_SHOW(void *pvParameters); 	/* Test_Task任务实现 */
static void Task__ONE(void *pvParameters); 	/* Test_Task任务实现 */
static void BSP_Init(void);                	/* 用于初始化板载相关资源 */
static void sendto_Upper(void);            	/* 用于初始化板载相关资源 */
static void Task__THREE(void *pvParameters);             	//
static void Task__FOUR(void *pvParameters);              	//
static void Turn_Angle(void);              	//
static void Achieve_Distance_Front_Head_Laser(void);
static void Achieve_Distance_For_Right_Laser(void);
static void Car_Running(void);
static void Go_Forward_Base_On_Encoder(void);
static void Pan_Left_Base_On_Encoder(void);
void Allocation_PID(int PIDOUT);


/*****************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化
            第二步：创建APP应用任务
            第三步：启动FreeRTOS，开始多任务调度
  ****************************************************************/
int main(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为pdPASS */

    /* 开发板硬件初始化 */
    BSP_Init();
    /* 创建AppTaskCreate任务 */
    xReturn = xTaskCreate((TaskFunction_t)AppTaskCreate,          /* 任务入口函数 */
                          (const char *)"AppTaskCreate",          /* 任务名字 */
                          (uint16_t)256,                          /* 任务栈大小 */
                          (void *)NULL,                           /* 任务入口函数参数 */
                          (UBaseType_t)1,                         /* 任务的优先级 */
                          (TaskHandle_t *)&AppTaskCreate_Handle); /* 任务控制块指针 */
    /* 启动任务调度 */
    if (pdPASS == xReturn)
        vTaskStartScheduler(); /* 启动任务，开启调度 */
    else
        return -1;

    while (1)
        ; /* 正常不会执行到这里 */
}

/***********************************************************************
 * @ 函数名  ： AppTaskCreate
 * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
 * @ 参数    ： 无
 * @ 返回值  ： 无
 **********************************************************************/
static void AppTaskCreate(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为pdPASS */

    taskENTER_CRITICAL(); // 进入临界区

    /* 创建Test_Task任务 */
    xReturn = xTaskCreate((TaskFunction_t)Task__TWO,          /* 任务入口函数 */
                          (const char *)"Task__TWO",          /* 任务名字 */
                          (uint16_t)512,                      /* 任务栈大小 */
                          (void *)NULL,                       /* 任务入口函数参数 */
                          (UBaseType_t)3,                    /* 任务的优先级 */
                          (TaskHandle_t *)&Task__TWO_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("Task__TWO任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task__ONE,          /* 任务入口函数 */
                          (const char *)"Task__ONE",          /* 任务名字 */
                          (uint16_t)512,                      /* 任务栈大小 */
                          (void *)NULL,                       /* 任务入口函数参数 */
                          (UBaseType_t)3,                     /* 任务的优先级 */
                          (TaskHandle_t *)&Task__ONE_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("Task__ONE任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)OLED_SHOW,          /* 任务入口函数 */
                          (const char *)"OLED_SHOW",          /* 任务名字 */
                          (uint16_t)512,                      /* 任务栈大小 */
                          (void *)NULL,                       /* 任务入口函数参数 */
                          (UBaseType_t)2,                     /* 任务的优先级 */
                          (TaskHandle_t *)&OLED_SHOW_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("OLED_SHOW任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task__THREE,          /* 任务入口函数 */
                          (const char *)"Task__THREE",          /* 任务名字 */
                          (uint16_t)256,                        /* 任务栈大小 */
                          (void *)NULL,                         /* 任务入口函数参数 */
                          (UBaseType_t)3,                       /* 任务的优先级 */
                          (TaskHandle_t *)&Task__THREE_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("Task__THREE任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task__FOUR,          /* 任务入口函数 */
                          (const char *)"Task__FOUR",          /* 任务名字 */
                          (uint16_t)256,                       /* 任务栈大小 */
                          (void *)NULL,                        /* 任务入口函数参数 */
                          (UBaseType_t)3,                      /* 任务的优先级 */
                          (TaskHandle_t *)&Task__FOUR_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("Task__FOUR任务创建成功\r\n");
    line_walking_Handle = xTimerCreate((const char *)"line_walking",
                                       (TickType_t)20,                         /* 定时器周期 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* 周期模式 */
                                       (void *)1,                              /* 为每个计时器分配一个索引的唯一ID */
                                       (TimerCallbackFunction_t)line_walking); //! 回调函数名
    analyse_data_Handle = xTimerCreate((const char *)"analyse_data",
                                       (TickType_t)20,                         /* 定时器周期 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* 周期模式 */
                                       (void *)4,                              /* 为每个计时器分配一个索引的唯一ID */
                                       (TimerCallbackFunction_t)analyse_data); //! 回调函数名

    sendto_Upper_Handle = xTimerCreate((const char *)"sendto_Upper",
                                       (TickType_t)40,                         /* 定时器周期 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* 周期模式 */
                                       (void *)4,                              /* 为每个计时器分配一个索引的唯一ID */
                                       (TimerCallbackFunction_t)sendto_Upper); //! 回调函数名
    Turn_Angle_Handle = xTimerCreate((const char *)"Turn_Angle",
                                     (TickType_t)40,                       /* 定时器周期 1000(tick) */
                                     (UBaseType_t)pdTRUE,                  /* 周期模式 */
                                     (void *)4,                            /* 为每个计时器分配一个索引的唯一ID */
                                     (TimerCallbackFunction_t)Turn_Angle); //! 回调函数名
	Achieve_Distance_For_Front_Laser_Handle = xTimerCreate((const char *)"Achieve_Distance_Front_Head_Laser",
                                    (TickType_t)20,                      /* 定时器周期 1000(tick) */
                                    (UBaseType_t)pdTRUE,                 /* 周期模式 */
                                    (void *)4,                           /* 为每个计时器分配一个索引的唯一ID */
                                    (TimerCallbackFunction_t)Achieve_Distance_Front_Head_Laser); //! 回调函数名
    Achieve_Distance_For_Right_Laser_Handle = xTimerCreate((const char *)"Achieve_Distance_For_Right_Laser",
                                    (TickType_t)20,                      /* 定时器周期 1000(tick) */
                                    (UBaseType_t)pdTRUE,                 /* 周期模式 */
                                    (void *)4,                           /* 为每个计时器分配一个索引的唯一ID */
                                    (TimerCallbackFunction_t)Achieve_Distance_For_Right_Laser); //! 回调函数名
    Car_Running_Handle = xTimerCreate((const char *)"Car_Running",
                                    (TickType_t)20,                      /* 定时器周期 1000(tick) */
                                    (UBaseType_t)pdTRUE,                 /* 周期模式 */
                                    (void *)4,                           /* 为每个计时器分配一个索引的唯一ID */
                                    (TimerCallbackFunction_t)Car_Running); //! 回调函数名
    Go_Forward_Base_On_Encoder_Handle = xTimerCreate((const char *)"Go_Forward_Base_On_Encoder",
                                    (TickType_t)20,                      /* 定时器周期 1000(tick) */
                                    (UBaseType_t)pdTRUE,                 /* 周期模式 */
                                    (void *)4,                           /* 为每个计时器分配一个索引的唯一ID */
                                    (TimerCallbackFunction_t)Go_Forward_Base_On_Encoder); //! 回调函数名
    Pan_Left_Base_On_Encoder_Handle = xTimerCreate((const char *)"Pan_Left_Base_On_Encoder",
                                    (TickType_t)20,                      /* 定时器周期 1000(tick) */
                                    (UBaseType_t)pdTRUE,                 /* 周期模式 */
                                    (void *)4,                           /* 为每个计时器分配一个索引的唯一ID */
                                    (TimerCallbackFunction_t)Pan_Left_Base_On_Encoder); //! 回调函数名

    xTimerStop(line_walking_Handle, 1);
    xTimerStop(sendto_Upper_Handle, 1);
    xTimerStart(analyse_data_Handle, 1);
    xTimerStop(Turn_Angle_Handle, 1);
	xTimerStop(Achieve_Distance_For_Front_Laser_Handle, 1);
    xTimerStop(Achieve_Distance_For_Right_Laser_Handle, 1);
    xTimerStop(Car_Running_Handle, 1);
    xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);
    xTimerStop(Pan_Left_Base_On_Encoder_Handle, 1);


    //xTimerStart(sendto_Upper_Handle, 0); //! 发送数据到上位机定时器

    Task_Number_Handle = xQueueCreate(1, 1); // 开始解析数据
    Group_One_Handle = xEventGroupCreate();
    Group_One_Handle = Group_One_Handle;
    
    // 挂机任务，等待选择任务
    vTaskSuspend(Task__THREE_Handle);
    vTaskSuspend(Task__TWO_Handle);
    vTaskSuspend(Task__FOUR_Handle);
    vTaskDelete(AppTaskCreate_Handle); // 删除AppTaskCreate任务

    taskEXIT_CRITICAL(); // 退出临界区
}

/**********************************************************************
 * @ 函数名  ： Timer_Task
 * @ 功能说明： Timer_Task任务主体
 * @ 参数    ：
 * @ 返回值  ： 无
 ********************************************************************/

/**
 * @description: this function is the software callback function that achieve pan left base on encoder num
 * @return {*}
 */
static void Pan_Left_Base_On_Encoder(void)
{
    static int i = 0;
    int32_t distance = position_of_car[1];
    int32_t speed = -(int32_t)PID_Realize(&X_Speed_PID, distance);
	speed = speed > 400 ? 400 : speed;
	speed = speed < -400 ? -400 : speed;
    taskENTER_CRITICAL();
    X_Speed = speed;
    taskEXIT_CRITICAL();
    if (fabs(distance - X_Speed_PID.Target)<15)
    {
        i++;
        if(i > 4)
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
static void Go_Forward_Base_On_Encoder(void)
{
    static int i = 0;
    int32_t distance = position_of_car[0];
    int32_t speed = (int32_t)PID_Realize(&Y_Speed_PID, distance);
	speed = speed > 1000 ? 1000 : speed;
	speed = speed < -1000 ? -1000 : speed;
    taskENTER_CRITICAL();
    Y_Speed = speed;
    taskEXIT_CRITICAL();
    if (fabs(distance - Y_Speed_PID.Target)<100)
    {
        i++;
        if(i > 4)
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
static void Achieve_Distance_For_Right_Laser(void)
{
    static int i = 0;
    float distance = VOFA_Data[2];
    int32_t speed = -(int32_t)PID_Realize(&X_Base_On_Laser_PID, distance);
    speed = speed > 400 ? 400 : speed;
	speed = speed < -400 ? -400 : speed;

    taskENTER_CRITICAL();
    X_Speed = speed;
    taskEXIT_CRITICAL();

    if (fabs(distance - X_Base_On_Laser_PID.Target)<10)
    {
        i++;
        if(i > 4)
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
static void Achieve_Distance_Front_Head_Laser(void)
{
    static int i = 0;
    float distance = VOFA_Data[3];
    int32_t speed = -(int32_t)PID_Realize(&Y_Base_On_Laser_PID, distance);
	speed = speed > 300 ? 300 : speed;
	speed = speed < -300 ? -300 : speed;

    taskENTER_CRITICAL();
    Y_Speed = speed;
    taskEXIT_CRITICAL();

    if (fabs(distance - Y_Base_On_Laser_PID.Target)<10)
    {
        i++;
        if(i > 4)
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
static void Car_Running(void)
{
    taskENTER_CRITICAL();   //enter critical zone and operate global variable

    //get the value of global variable
    int32_t Local_Y_Speed = Y_Speed;
    int32_t Local_X_Speed = X_Speed;
    int32_t Local_angle_speed = angle_speed;

    //clear the value of global variable
    Y_Speed = 0;
    X_Speed = 0;
    angle_speed = 0;
    
    taskEXIT_CRITICAL();    //exit critical zone
    
    //calculate the ccr register value of the wheel
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


    //change the direction of the wheel
   for (size_t i = 0; i < 4; i++)
   {
       if (CCR_wheel[i]>0)  //forward
       {
           Advance(i+2);    //it is because the wheel is from 2 to 5
       }
       else //backward
       {
           Back(i+2);       //it is because the wheel is from 2 to 5
           CCR_wheel[i] = -CCR_wheel[i];    //change the value to positive, because the function "SetCompare1" need a positive value
       }
   }
    
    //set the ccr register value of the wheel
    SetCompare1(TIM1, CCR_wheel[0], 1);
    SetCompare1(TIM1, CCR_wheel[1], 2);
    SetCompare1(TIM1, CCR_wheel[2], 3);
    SetCompare1(TIM1, CCR_wheel[3], 4);

}

/**
 * @description: this function is the software callback function that achieve turn angle
 * @return {*}
*/
static void Turn_Angle(void)
{
    static u8 i = 0;
    float Angle, PIDOUT;
    Angle = (float)stcAngle.Angle[2] / 32768 * 180;
    PIDOUT = PID_Realize(&Turn_Angle_PID, Angle);
    angle_speed = (int32_t)PIDOUT;
    if (fabs(Angle - Turn_Angle_PID.Target) < 1)
    {
        i++;
        if(i > 4)
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
static void sendto_Upper(void)
{

    VOFA_Send_float(VOFA_Data, 3); //! 发送数据给VOFA
    VOFA_Data[0] = (float)stcAngle.Angle[2] / 32768 * 180;
}

/**
 * @description: this function is the software callback function that keep the car walking straightly
 * @return {*}
*/
static void line_walking(void)
{
    float Angle;
    //?PID开始巡线
    Angle = (float)stcAngle.Angle[2] / 32768 * 180;
    angle_speed = PID_Realize(&Coord, Angle);
    return;
}

/**
 * @description: this function is the software callback function that analyse data
 * @return {*}
*/
static void analyse_data(void)
{
    u8 i = 0, num_f = 0, num_r = 0, TOF_Data[7];
    const static u8 VL53_Agreement_RX[] = {0x50, 0x03, 0x02};
    const u8 TOFSENSE[] = {0x57,0x00,0xFF,0x01};
    for (i = 0; i < 5; i++)
    {
        VL53_Send_Agrement();
		vTaskDelay(1);
		if(have_enough_data(&U3_buffer,3,1,16))
		{
			while(have_enough_data(&U3_buffer, 3,1,16))
				Read_buff_Void(&U3_buffer, VL53_Agreement_RX, 3, &Distance.F[i], 1, 16, 1);
			num_f++;
		}
		else
			Distance.F[i] = 0;
		if(have_enough_data(&VL53_USARTX_Buff,20,0,8))
		{
			while(have_enough_data(&VL53_USARTX_Buff, 3, 4, 8))
            {
				Read_buff_Void(&VL53_USARTX_Buff,TOFSENSE,4,TOF_Data,7,8,1);
                Distance.R[i] = (int32_t)(TOF_Data[4] << 8 | TOF_Data[5] << 16 | TOF_Data[6] << 24) / 256;
            }
			num_r++;
		}
		else
			Distance.R[i] = 0;
        
    }
    for (i = 0; i < 5; i++)
    {
        Distance.R_OUT += Distance.R[i];
        Distance.F_OUT += Distance.F[i];
    }
	if(num_r)
		Distance.R_OUT /= num_r;
	else
		Distance.R_OUT = 0;
	if(num_f)
		Distance.F_OUT /= num_f;
	else
		Distance.F_OUT = 0;

    int32_t How_many_revolutions_of_the_motor[4] = {0,0,0,0};

    How_many_revolutions_of_the_motor[0] = Read_Encoder(2);
    How_many_revolutions_of_the_motor[1] = Read_Encoder(3);
    How_many_revolutions_of_the_motor[2] = Read_Encoder(4);
    How_many_revolutions_of_the_motor[3] = Read_Encoder(5);
	
	u8 get_howmanyencoder = 0;
	for(u8 n=0;n<4;n++)
	{
		if(fabs(How_many_revolutions_of_the_motor[n]) > 5)
			get_howmanyencoder++;
	}

	int32_t The_distance_that_has_gone_forward = 0;
    if(get_howmanyencoder != 0)
        The_distance_that_has_gone_forward = (How_many_revolutions_of_the_motor[0]
                            +How_many_revolutions_of_the_motor[1]
                            +How_many_revolutions_of_the_motor[2]
                            +How_many_revolutions_of_the_motor[3])/get_howmanyencoder;
    

    int32_t The_distance_that_has_gone_right_head_side = (-How_many_revolutions_of_the_motor[0]
                            +How_many_revolutions_of_the_motor[1]
                            +How_many_revolutions_of_the_motor[2]
                            -How_many_revolutions_of_the_motor[3])/4;
    
    //get data from upper computer
    const static u8 upper_head[] = {0x1C, 0x2C, 0x3C};
    u8 local_dataFromLinux_buff[4] = {0, 0};
    static u8 index = 0;
    static u8 local_dataFromLinux[4] = {0, 0};
    while(have_enough_data(&U5_buffer, 3, 4, 8))    //judge whether the data is enough
    {
        Read_buff_Void(&U5_buffer, (u8 *)upper_head, 3, local_dataFromLinux_buff, 4, 8, 1); //read the data from buffer
        
        if(!memcmp(local_dataFromLinux_buff, local_dataFromLinux, 4))   //judge whether the data is same as the last data
        {
            index++;
            if(index >= 4)   //if the data is same as the last data, then judge whether the data is same as the last data for 4 times
            {
                index = 0;
                //
                memcpy(dataFromLinux, local_dataFromLinux, 4);   //copy the data from local variable to global variable
            }
        }
        else
        {
            index = 0;
            memcpy(local_dataFromLinux, local_dataFromLinux_buff, 4);   //copy the data from local variable to global variable
        }
    
    }
    
    

    taskENTER_CRITICAL();           //进入基本临界区

    
    VOFA_Data[2] = Distance.R_OUT;           //右边激光测距得到的距离
    VOFA_Data[3] = Distance.F_OUT;           //前面激光测距得到的距离
    Distance.F_OUT = 0;
    Distance.R_OUT = 0;
    position_of_car[0] += The_distance_that_has_gone_forward;            //the distance that car have gone forward
    position_of_car[1] += The_distance_that_has_gone_right_head_side;    //the distance that car have gone right hand side
    Read_RGB();
    
    taskEXIT_CRITICAL();            //退出基本临界区
}

/**
 * the software timer callback function is end in here
*/



/**
 * the following code is about open the software timer
*/
// there are some define about the secord param of the function "startStraight_Line_For_Laser" and "startStraight_Line_Base_On_Encoder"
#define forward 1
#define pan 0
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
    else if (which_laser == 1)  //根据前面的激光距离来调整距离
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
    else if (forwardOrPan == 1)  //根据编码器向前走
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
	float Angle;
	Angle = (float)stcAngle.Angle[2] / 32768 * 180;
	if(i == 1)
		Turn_Angle_PID.Target = Angle - 90;
	else if (i==0)
		Turn_Angle_PID.Target = Angle + 90;
	already_turned = 0;
	xTimerStart(Turn_Angle_Handle, 0);
    xTimerStart(Car_Running_Handle,0);
}

/**
 * the code that open the software timer is end in here
*/



/**
 * the following code is about the definition of the first param the function "check_rgb" 
*/
#define red_color 1
#define yellow_color 2
/**
 * @description: this function is used to check the color of the ground below the drop
 * @param {int} color :the color that you want to check 
 * @return {bool} :if the color is right,return true,else return false
*/
bool check_rgb(int color)
{
    if (color == red_color)
    {
        if (RGB.R > 40 && RGB.R < 65)
        {
            if (RGB.G > 25 && RGB.G < 55)
            {
                if (RGB.B > 25 && RGB.B < 55)
                {
                    return true;
                }
            }
        }
        return false;
    }
    else if (color == yellow_color)
    {
        if (RGB.R > 120 && RGB.R < 155)
        {
            if (RGB.G > 110 && RGB.G < 140)
            {
                if (RGB.B > 55 && RGB.B < 90)
                {
                    return true;
                }
            }
        }
        return false;
    }
    else
    {
        return false;
    }
    
    
}


/**
 * the following code is about the task that we create
*/

/**
 * @description: this task is is used to show necessary information on the OLED
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
*/
static void OLED_SHOW(void *pvParameters)
{
    while (1)
    {
		vTaskDelay(1);
        OLED_SHOW_TASK();
    }
}

/**
 * @description: this task is including the main logic of the program
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
static void Task__ONE(void *parameter)
{
    


    // char qrcode=0x07;
    while (1)
    {
        // while(1)
        // {
        //     if(check_keystart == RESET)
        //     {
        //         vTaskDelay(50);
        //         if(check_keystart == RESET)
        //         {
        //             break;
        //         }
        //     }
        // }
//        while(1)
//		{
//			vTaskDelay(1000);
//			printf("1:%4d 2:%4d 3:%4d 4:%4d\r\n",Read_Encoder(2), Read_Encoder(3),Read_Encoder(4), Read_Encoder(5));
//		}
        
        //xEventGroupWaitBits(Group_One_Handle, 0x01, pdTRUE, pdTRUE, portMAX_DELAY); //! 开始比赛
        //-开箱 狗叫
        // float Angle;
		// Angle = (float)stcAngle.Angle[2] / 32768 * 180;
		// Turn_Angle_PID.Target = Angle + 90;
		// already_turned = 0;
		// xTimerStart(Turn_Angle_Handle, 0);
        startStraight_Line_Base_On_Encoder(12500,forward);	//根据编码器向前走到t型路口
        startgostraight(0);								//保证走直线
        while(!Y_have_achieved)							//检测到达位置
            vTaskDelay(20);
        
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);				//小车停止移动
        xTimerStop(Car_Running_Handle, 1);				//小车停止移动
        xTimerStop(line_walking_Handle, 1);				//停止走直线
		start_trun(1);                                  //左转
		while(!already_turned)                          //等待转弯完成
            vTaskDelay(20);
        
        startStraight_Line_For_Laser(175,pan);      //!the distance need to be changed,it is because only the center of road do not have barrier
        startgostraight(-90);       //保证车的方向不变
        while (!X_have_achieved)
        {
            vTaskDelay(20);
        }
        xTimerStop(line_walking_Handle, 1);				//停止走直线
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);				//小车停止移动
        // here we need to switch the task that first go to the red area in the right hand side or in the left hand side

        
        if(0)        //!the code in here is unfinished,we need to judge the value of the dataFromLinux[0] to decide which task we should switch to
            vTaskResume(Task__TWO_Handle);
        else
            vTaskResume(Task__FOUR_Handle);
        
        while(1)
        {
            vTaskDelay(1000);
        }

        
    }
}

/**
 * @description: this task is used to control the car,make it go to the red area in the right hand side firstly,then go to the red area in the left hand side
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
static void Task__TWO(void *parameter)
{
    while (1)
    {

        startStraight_Line_Base_On_Encoder(-22500, forward);   //向后走到右边红色区域
        startgostraight(-90);                           //保证走直线
        while(!check_rgb(1))                    //用rgb颜色识别检测到达红色区域
        vTaskDelay(20);

        xTimerStop(Car_Running_Handle, 1);              //小车停止移动
		xTimerStop(line_walking_Handle, 1);             //停止走直线
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);   //停止前后走

        //!打开仓库，取出物品，这是后面需要加的代码
        //播报到达二号仓库
        USART_Send(voice[2],6);
        //向前走到左边红色区域
        startStraight_Line_Base_On_Encoder(12000, forward);     //!code is unfinished,the first param need to be changed

        startgostraight(-90);                           //保证走直线
        while(!Y_have_achieved)                         //检测到达位置
            vTaskDelay(20);
        startStraight_Line_For_Laser(175, pan);       //根据右边激光测距调整距离
        startStraight_Line_For_Laser(150, forward);     //根据前面激光测距调整距离
        while((!X_have_achieved)||(!Y_have_achieved))    //检测到达位置
            vTaskDelay(20);
        
        //!打开仓库，取出物品，这是后面需要加的代码
        //播报到达一号仓库
        USART_Send(voice[1],6);

        startStraight_Line_Base_On_Encoder(-6800, forward);    //!向后走到t字路口 code is unfinished,the first param need to be changed
        startgostraight(-90);                    //保证走直线
        while(!Y_have_achieved)             //检测到达位置
            vTaskDelay(20);
        
        xTimerStop(line_walking_Handle, 1);     //停止走直线
        xTimerStop(Car_Running_Handle, 1);      //小车停止移动

        //开始右转
        start_trun(0);                                  //右转
        while(!already_turned)                          //等待转弯完成
            vTaskDelay(20);
        
        startStraight_Line_For_Laser(240, forward);     //根据前面激光测距调整距离
        while(!Y_have_achieved)                         //检测到达位置
            vTaskDelay(20);
        
        startStraight_Line_Base_On_Encoder(-22500, forward);   //向后走到右边黄色区域
        startgostraight(0);                             //保证走直线
        while(!check_rgb(2))                    //用rgb颜色识别检测到达黄色区域
            vTaskDelay(20);
        //小车停止移动
        xTimerStop(Car_Running_Handle, 1);
        xTimerStop(line_walking_Handle, 1);


        PULL_High();
        while(1)                                        //任务完成，挂机
        {
            vTaskDelay(1000);
        }
    }
}


/**
 * @description: this task is used to control loudspeaker
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
static void Task__THREE(void *parameter)
{
    while (1)
    {

        	USART_Send(voice[0],6);     //播报打开仓库门
        vTaskDelay(5000); /* 延时500个tick */
			USART_Send(voice[1],6);     //播报到一号仓库
        vTaskDelay(5000); /* 延时500个tick */
			USART_Send(voice[2],6);     //播报到达二号仓库
        vTaskDelay(5000); /* 延时500个tick */
    }
}
/**
 * @description: this task is used to control the car,make it go to the red area in the left hand side firstly,then go to the red area in the right hand side
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
static void Task__FOUR(void *parameter)
{
    while (1)
    {
        startStraight_Line_Base_On_Encoder(5500, forward);      //向前走到左边红色区域
		startgostraight(-90);                           //保证走直线

		while(!Y_have_achieved)         //检测到达左边红色区域
            vTaskDelay(10);
		
        startStraight_Line_For_Laser(175, pan);       //根据右边激光测距调整距离
        startStraight_Line_For_Laser(150, forward);     //根据前面激光测距调整距离
        while((!X_have_achieved)||(!Y_have_achieved))    //检测到达位置
            vTaskDelay(10);
		xTimerStop(Car_Running_Handle, 1);      //小车停止移动
        //!打开仓库，取出物品，这是后面需要加的代码


        startStraight_Line_Base_On_Encoder(-22500, forward);    //向后走到左边红色区域
        startgostraight(-90);                    //保证走直线
        vTaskDelay(1500);             //延时1.5s，否则会检测到左边红色区域
        while(!check_rgb(1))            //用rgb颜色识别检测到达右边红色区域
        vTaskDelay(20);                 //延时20ms

        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);   //停止前后走
        PULL_High();

        startStraight_Line_For_Laser(175, pan);       //根据右边激光测距调整距离
        while(!X_have_achieved)         //检测到达位置
            vTaskDelay(20);

        //!打开仓库，取出物品，这是后面需要加的代码


        startStraight_Line_Base_On_Encoder(6100, forward);    //!向前走到十字t字路口 code is unfinished,the first param need to be changed
        startgostraight(-90);               //保证走直线
        while(!Y_have_achieved)             //检测到达位置
            vTaskDelay(20);
        
        xTimerStop(line_walking_Handle, 1);     //停止走直线
        //开始右转
        start_trun(0);                                  //右转
        while(!already_turned)                          //等待转弯完成
            vTaskDelay(20);

        startStraight_Line_For_Laser(240, forward);     //根据前面激光测距调整距离
        while(!Y_have_achieved)                         //检测到达位置
            vTaskDelay(20);
        
        startStraight_Line_Base_On_Encoder(-22500, forward);   //向后走到右边黄色区域
        startgostraight(0);                             //保证走直线
        while(!check_rgb(2))                    //用rgb颜色识别检测到达黄色区域
        vTaskDelay(20);
        //小车停止移动
        xTimerStop(Car_Running_Handle, 1);
        xTimerStop(line_walking_Handle, 1);
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);

        PULL_High();        //挂机 it is necessary to add the code that make the direction pin of the motor high
        while(1)
        {
            vTaskDelay(1000);
        }
    }
}
/**
 * the code that create the task is end in here
*/

/***********************************************************************
 * @ 函数名  ： BSP_Init
 * @ 功能说明： 板级外设初始化，所有板子上的初始化均可放在这个函数里面
 * @ 参数    ：
 * @ 返回值  ： 无
 ***********************************************************************/
static void BSP_Init(void)
{
    /*
     * STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15
     * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
     * 都统一用这个优先级分组，千万不要再分组，切忌。
     */
    RCC_ClocksTypeDef get_rcc_clock;
    RCC_GetClocksFreq(&get_rcc_clock);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    PWM_TIM1_config(2000, 3, Initial_Speed, Initial_Speed, Initial_Speed, Initial_Speed);
    TIMX_Delay_Init(RCC_APB1Periph_TIM6, 65530, 72, TIM6);
    Encoder_Init();
    USART1_Config();      //*调试信息输出
    Iinitial_BUFF(&U3_buffer);
	Iinitial_BUFF(&U1_buffer);
	Iinitial_BUFF(&U5_buffer);
    USART3_Config();      //*USART3前激光测距
    USART5_Config();      //*USART5 responsible for the communication between the car and the upper computer Raspberry PI   负责和上位机树莓派通信
	
    USART4_Config_JY62(); //*UART4陀螺仪
    
    VL53_Initial(); //*USART2右激光测距
    Initial_Control_PIN();
	
    //pid初始化
    PID_Initialize(&Coord, 45, 0, 0, 0, 25, -25);                   //微调巡线的pid初始化
    PID_Initialize(&Turn_Angle_PID, 30, 0, 10, 0, 25, -25);         //转弯的pid初始化
    PID_Initialize(&X_Speed_PID, 3.75, 0, .5, 0, 100, -100);         //x方向的远距离基于编码器的pid
    PID_Initialize(&Y_Speed_PID, 3.75, 0, .5, 0, 100, -100);         //y方向的远距离基于编码器的pid
    PID_Initialize(&X_Base_On_Laser_PID, 50, 0, 1., 0, 100, -100);
    PID_Initialize(&Y_Base_On_Laser_PID, 50, 0, 1., 0, 100, -100);
	Software_USART_IOConfig();
    LED_GPIO_Config();

    keystart();     //开始启动小车
    KEY_ONE();
    OLED_Init();
    IIC_Init();
    sendcmd(ACCCMD);
    Delayms(500);
    sendcmd(YAWCMD);
    Delayms(2000);
    GPIO_SetBits(GPIOE, GPIO_Pin_1);
	
	
}


/********************************END OF FILE****************************/
