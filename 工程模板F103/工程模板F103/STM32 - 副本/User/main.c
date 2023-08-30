
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
#include "wirless_usart.h"
#include "PID.h"
#include "limits.h"
#include "encoder.h"
#include "mpu6050.h"
#include "tcs34725.h"
/**************************** 任务句柄 ********************************/
/*
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */

QueueHandle_t Task_Number_Handle = NULL;

static EventGroupHandle_t Task_Handle = NULL;
static TimerHandle_t Task_Change_Handle = NULL;    //+任务切换定时器句柄
static TimerHandle_t sendto_Upper_Handle = NULL;   //+发送数据到上位机定时器句柄
static TimerHandle_t line_walking_Handle = NULL;   //+巡线PID定时器句柄
static TimerHandle_t analyse_data_Handle = NULL;   //+解析数据定时器句柄
static TaskHandle_t OLED_SHOW_Handle = NULL;       //+OLDE显示句柄
static TaskHandle_t AppTaskCreate_Handle = NULL;   //+创建任务句柄
static TaskHandle_t Task__ONE_Handle = NULL;       //+任务1句柄
static TaskHandle_t Task__TWO_Handle = NULL;       //+任务2句柄
static TaskHandle_t Task__THREE_Handle = NULL;     //+任务3句柄
static TaskHandle_t Task__FOUR_Handle = NULL;      //+任务4句柄
static EventGroupHandle_t Group_One_Handle = NULL; //+事件组句柄
/********************************** 内核对象句柄 *********************************/
/*
 * 信号量，消息队列，事件标志组，软件定时器这些都属于内核的对象，要想使用这些内核
 * 对象，必须先创建，创建成功之后会返回一个相应的句柄。实际上就是一个指针，后续我
 * 们就可以通过这个句柄操作这些内核对象。
 *
 * 内核对象说白了就是一种全局的数据结构，通过这些数据结构我们可以实现任务间的通信，
 * 任务间的事件同步等各种功能。至于这些功能的实现我们是通过调用这些内核对象的函数
 * 来完成的
 *
 */

/******************************* 全局变量声明 ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */
#define Initial_Speed 1000
extern const char ACCCMD[3];
extern const char YAWCMD[3];
float Angle_X, Angle_Y, Angle_Z, VOFA_Data[3];
u8 X_Y[2];
extern unsigned char F8X16[];
/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void
analyse_data(void);
void Angle_Speed_X_PID_fun(u8 EN, int Coordinate_PID);
static void line_walking(void);
static void AppTaskCreate(void);           /* 用于创建任务 */
static void Task__TWO(void *pvParameters); /* Test_Task任务实现 */
static void OLED_SHOW(void *pvParameters); /* Test_Task任务实现 */
static void Task__ONE(void *pvParameters); /* Test_Task任务实现 */
static void BSP_Init(void);                /* 用于初始化板载相关资源 */
static void sendto_Upper(void);            /* 用于初始化板载相关资源 */
static void Task_Change(void);             /* 用于初始化板载相关资源 */
static void Task__THREE(void);             //
static void Task__FOUR(void);              //

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
                          (uint16_t)256,                      /* 任务栈大小 */
                          (void *)NULL,                       /* 任务入口函数参数 */
                          (UBaseType_t)10,                    /* 任务的优先级 */
                          (TaskHandle_t *)&Task__TWO_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("Task__TWO任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task__ONE,          /* 任务入口函数 */
                          (const char *)"Task__ONE",          /* 任务名字 */
                          (uint16_t)256,                      /* 任务栈大小 */
                          (void *)NULL,                       /* 任务入口函数参数 */
                          (UBaseType_t)2,                     /* 任务的优先级 */
                          (TaskHandle_t *)&Task__ONE_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("Task__ONE任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)OLED_SHOW,          /* 任务入口函数 */
                          (const char *)"OLED_SHOW",          /* 任务名字 */
                          (uint16_t)256,                      /* 任务栈大小 */
                          (void *)NULL,                       /* 任务入口函数参数 */
                          (UBaseType_t)2,                     /* 任务的优先级 */
                          (TaskHandle_t *)&OLED_SHOW_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("OLED_SHOW任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task__THREE,          /* 任务入口函数 */
                          (const char *)"Task__THREE",          /* 任务名字 */
                          (uint16_t)256,                        /* 任务栈大小 */
                          (void *)NULL,                         /* 任务入口函数参数 */
                          (UBaseType_t)2,                       /* 任务的优先级 */
                          (TaskHandle_t *)&Task__THREE_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("Task__THREE任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task__FOUR,           /* 任务入口函数 */
                          (const char *)"Task__FOUR",           /* 任务名字 */
                          (uint16_t)256,                        /* 任务栈大小 */
                          (void *)NULL,                         /* 任务入口函数参数 */
                          (UBaseType_t)2,                       /* 任务的优先级 */
                          (TaskHandle_t *)&Task__FOUR_Handle);  /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("Task__FOUR任务创建成功\r\n");
    line_walking_Handle = xTimerCreate((const char *)"line_walking",
                                       (TickType_t)30,                         /* 定时器周期 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* 周期模式 */
                                       (void *)1,                              /* 为每个计时器分配一个索引的唯一ID */
                                       (TimerCallbackFunction_t)line_walking); //! 回调函数名
    analyse_data_Handle = xTimerCreate((const char *)"analyse_data",
                                       (TickType_t)25,                         /* 定时器周期 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* 周期模式 */
                                       (void *)4,                              /* 为每个计时器分配一个索引的唯一ID */
                                       (TimerCallbackFunction_t)analyse_data); //! 回调函数名

    sendto_Upper_Handle = xTimerCreate((const char *)"sendto_Upper",
                                       (TickType_t)40,                         /* 定时器周期 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* 周期模式 */
                                       (void *)4,                              /* 为每个计时器分配一个索引的唯一ID */
                                       (TimerCallbackFunction_t)sendto_Upper); //! 回调函数名
    Task_Change_Handle = xTimerCreate((const char *)"Task_Change",
                                      (TickType_t)40,                        /* 定时器周期 1000(tick) */
                                      (UBaseType_t)pdTRUE,                   /* 周期模式 */
                                      (void *)4,                             /* 为每个计时器分配一个索引的唯一ID */
                                      (TimerCallbackFunction_t)Task_Change); //! 回调函数名

    xTimerStop(line_walking_Handle, 0);
    xTimerStop(analyse_data_Handle, 0);
    xTimerStop(sendto_Upper_Handle, 0);

    xTimerStart(sendto_Upper_Handle, 0); //! 发送数据到上位机定时器
    xTimerStart(Task_Change_Handle, 0);  //! 开启任务切换定时器

    Task_Number_Handle = xQueueCreate(1, 1); // 开始解析数据
    Task_Handle = xEventGroupCreate();
    Group_One_Handle = xEventGroupCreate();

    //+挂机任务，等待选择任务
    vTaskSuspend(Task__ONE_Handle);
    vTaskSuspend(Task__TWO_Handle);
    vTaskDelete(AppTaskCreate_Handle); // 删除AppTaskCreate任务

    taskEXIT_CRITICAL(); // 退出临界区
}

/**********************************************************************
 * @ 函数名  ： Test_Task
 * @ 功能说明： Test_Task任务主体
 * @ 参数    ：
 * @ 返回值  ： 无
 ********************************************************************/

static void sendto_Upper()
{

    // VOFA_Send_float(VOFA_Data); //! 发送数据给VOFA
}
static void line_walking()
{
    //?PID开始巡线
}
void analyse_data()
{
    //?获取MV,K210数据，双击通讯解析
    // Check_Connect(0, 1000);
}

static void OLED_SHOW(void *pvParameters)
{
    while (1)
    {
        OLED_SHOW_TASK();

        vTaskDelay(100);
    }
}
static void Task__ONE(void *parameter)
{

    while (1)
    {
        // xEventGroupWaitBits(Group_One_Handle, 0x01, pdTRUE, pdTRUE, portMAX_DELAY);
        printf("HHHHHHH");
        vTaskDelay(100);
    }
}

static void Task__TWO(void *parameter)
{
    while (1)
    {

        vTaskDelay(100);
    }
}
static void Task__THREE(void)
{
    while (1)
    {

        vTaskDelay(100);
    }
}
static void Task__FOUR(void)
{
    while (1)
    {

        vTaskDelay(100);
    }
}
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
    PWM_TIM8_config(2000, 720, 50, 250, 1000, 1000);
    TIMX_Delay_Init(RCC_APB1Periph_TIM6, 65530, 72, TIM6);

    USART1_Config();
    USART2_Config();
    Iinitial_BUFF(&U2_buffer);
    Iinitial_BUFF(&U3_buffer);
    USART4_Config_JY62();
    Initial_Control_PIN();
    Back(2);
    Advance(3);
    Back(4);
    Back(5);
    LED_GPIO_Config();
    KEY_ONE();
    GPIO_SetBits(GPIOE, GPIO_Pin_1);
}
void Task_Change()
{
    static u8 Event_Task = 1;
    static u8 SURE_TASK = 0;
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0))
    {
        vTaskDelay(40);
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0))
        {
            vTaskDelay(150);
            if (SURE_TASK == 0)
            {
                Event_Task += 2;
            }
            else
                Event_Task++;

            if (Event_Task >= 13)
            {
                Event_Task = 3;
            }
        }
    }
    if (Event_Task == 3) //! Task 1
    {
        xQueueOverwrite(Task_Number_Handle, &Event_Task);
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7))
        {
            vTaskDelay(40);
            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7))
            {
                Event_Task++;
                vTaskDelay(130);
                SURE_TASK = 2;
                xQueueOverwrite(Task_Number_Handle, &SURE_TASK);
                //*任务1
                vTaskResume(Task__ONE_Handle);
            }
        }
        else
            SURE_TASK = 0;
    }
    if (Event_Task == 5) //! Task 2
    {
        xQueueOverwrite(Task_Number_Handle, &Event_Task);

        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7))
        {
            vTaskDelay(40);
            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7))
            {
                Event_Task++;

                vTaskDelay(100);
                SURE_TASK = 4;
                xQueueOverwrite(Task_Number_Handle, &SURE_TASK);
                //*任务2
                vTaskResume(Task__TWO_Handle);
            }
        }
        else
            SURE_TASK = 0;
    }
    if (Event_Task == 7) //! Task 3
    {
        xQueueOverwrite(Task_Number_Handle, &Event_Task);

        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7))
        {
            vTaskDelay(40);
            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7))
            {
                Event_Task++;

                vTaskDelay(130);
                SURE_TASK = 6;
                xQueueOverwrite(Task_Number_Handle, &SURE_TASK);
                //*任务3
            }
        }
        else
            SURE_TASK = 0;
    }
    if (Event_Task == 9) //! Task 4
    {
        xQueueOverwrite(Task_Number_Handle, &Event_Task);

        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7))
        {
            vTaskDelay(40);
            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7))
            {
                Event_Task++;

                vTaskDelay(130);
                SURE_TASK = 8;
                xQueueOverwrite(Task_Number_Handle, &SURE_TASK);
                //*任务4
            }
        }
        else
            SURE_TASK = 0;
    }
    if (Event_Task == 11) //! Task 5
    {
        xQueueOverwrite(Task_Number_Handle, &Event_Task);

        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7))
        {
            vTaskDelay(40);
            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7))
            {
                Event_Task++;

                vTaskDelay(130);
                SURE_TASK = 10;
                xQueueOverwrite(Task_Number_Handle, &SURE_TASK);
                //*任务5
            }
        }
        else
            SURE_TASK = 0;
    }
}
// void Angle_Speed_X_PID_fun(u8 EN, int Coordinate_PID)
// {

//     if (EN == 1)
//     {
//     }

//     if (Coordinate_PID > 0)
//     {
//         /* code */

//         if (Coordinate_PID >= 600)
//         {
//             /* code */
//             Coordinate_PID = 600;
//             SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 2);
//             SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 4);
//             SetCompare1(TIM1, Initial_Speed, 1);
//             SetCompare1(TIM1, Initial_Speed, 3);
//         }
//         else
//         {
//             SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 2);
//             SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 4);
//             SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 1);
//             SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 3);
//         }
//     }
//     if (Coordinate_PID < 0)
//     {
//         /* code */

//         if (Coordinate_PID < -600)
//         {
//             /* code */
//             Coordinate_PID = -600;
//             SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 1);
//             SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 3);
//             SetCompare1(TIM1, Initial_Speed, 2);
//             SetCompare1(TIM1, Initial_Speed, 4);
//         }
//         else
//         {
//             SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 1);
//             SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 3);
//             SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 2);
//             SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 4);
//         }
//     }
// }

/********************************END OF FILE****************************/
