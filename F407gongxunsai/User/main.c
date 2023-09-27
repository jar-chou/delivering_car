/*
 * ......................................&&.........................
 * ....................................&&&..........................
 * .................................&&&&............................
 * ...............................&&&&..............................
 * .............................&&&&&&..............................
 * ...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * ..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * ................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * .......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * ...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * ..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * ...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * ..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * ..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * ........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * .......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * ........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * ..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * ................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * ..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * ..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * ...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * .........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * .......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * ......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * .....&&&&&&&&&&&&&&&&............................&&..............
 * ....&&&&&&&&&&&&&&&.................&&...........................
 * ...&&&&&&&&&&&&&&&.....................&&&&......................
 * ...&&&&&&&&&&.&&&........................&&&&&...................
 * ..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * ..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * ..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * ..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * ..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * ...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * ....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * .......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * .......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * ........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * .......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * .......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * ........&&...................&&&&&&.........................&&&..
 * .........&.....................&&&&........................&&....
 * ...............................&&&.......................&&......
 * ................................&&......................&&.......
 * .................................&&..............................
 * ..................................&..............................
 * 
 * @Author: zhaojianchao jar-chou 2722642511@qq.com 
 * @Date: 2023-09-06 13:02:19
 * @LastEditors: jar-chou 2722642511@qq.com
 * @LastEditTime: 2023-09-07 13:50:25
 * @FilePath: \delivering_car\User\main.c
 * @Description: 龙王保佑此文件无bug！！！
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
#include "Delay.h"
#include "PWM.h"
#include "Driver.h"
#include "software_usart.h"
#include "Software_iic.h"
#include "JY62.h"
#include "buffer.h"
#include "PID.h"
#include "limits.h"
#include "encoder.h"
#include "OLED.h"
#include "VL53.h"
#include "sys.h"
#include <math.h>
#include <stdbool.h>
#include "bsp.h"


/**************************** 任务句柄 ********************************/
/*
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */

QueueHandle_t Task_Number_Handle = NULL;

static TimerHandle_t sendto_Upper_Handle = NULL;   			//+发送数据到上位机定时器句柄
static TimerHandle_t line_walking_Handle = NULL;   			//+巡线PID定时器句柄
static TimerHandle_t analyse_data_Handle = NULL;   			//+解析数据定时器句柄



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
#define Initial_Speed 1500
#define Turn_Speed 0
extern const char ACCCMD[3];
extern const char YAWCMD[3];

float VOFA_Data[4];


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
static void Task__THREE(void);             	//
static void Task__FOUR(void);              	//
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
                          (UBaseType_t)1,                    /* 任务的优先级 */
                          (TaskHandle_t *)&Task__TWO_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("Task__TWO任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task__ONE,          /* 任务入口函数 */
                          (const char *)"Task__ONE",          /* 任务名字 */
                          (uint16_t)512,                      /* 任务栈大小 */
                          (void *)NULL,                       /* 任务入口函数参数 */
                          (UBaseType_t)2,                     /* 任务的优先级 */
                          (TaskHandle_t *)&Task__ONE_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("Task__ONE任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)OLED_SHOW,          /* 任务入口函数 */
                          (const char *)"OLED_SHOW",          /* 任务名字 */
                          (uint16_t)256,                      /* 任务栈大小 */
                          (void *)NULL,                       /* 任务入口函数参数 */
                          (UBaseType_t)3,                     /* 任务的优先级 */
                          (TaskHandle_t *)&OLED_SHOW_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("OLED_SHOW任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task__THREE,          /* 任务入口函数 */
                          (const char *)"Task__THREE",          /* 任务名字 */
                          (uint16_t)256,                        /* 任务栈大小 */
                          (void *)NULL,                         /* 任务入口函数参数 */
                          (UBaseType_t)4,                       /* 任务的优先级 */
                          (TaskHandle_t *)&Task__THREE_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("Task__THREE任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task__FOUR,          /* 任务入口函数 */
                          (const char *)"Task__FOUR",          /* 任务名字 */
                          (uint16_t)256,                       /* 任务栈大小 */
                          (void *)NULL,                        /* 任务入口函数参数 */
                          (UBaseType_t)5,                      /* 任务的优先级 */
                          (TaskHandle_t *)&Task__FOUR_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("Task__FOUR任务创建成功\r\n");
    line_walking_Handle = xTimerCreate((const char *)"line_walking",
                                       (TickType_t)30,                         /* 定时器周期 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* 周期模式 */
                                       (void *)1,                              /* 为每个计时器分配一个索引的唯一ID */
                                       (TimerCallbackFunction_t)line_walking); //! 回调函数名
    analyse_data_Handle = xTimerCreate((const char *)"analyse_data",
                                       (TickType_t)20,                         /* 定时器周期 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* 周期模式 */
                                       (void *)2,                              /* 为每个计时器分配一个索引的唯一ID */
                                       (TimerCallbackFunction_t)analyse_data); //! 回调函数名

    sendto_Upper_Handle = xTimerCreate((const char *)"sendto_Upper",
                                       (TickType_t)40,                         /* 定时器周期 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* 周期模式 */
                                       (void *)3,                              /* 为每个计时器分配一个索引的唯一ID */
                                       (TimerCallbackFunction_t)sendto_Upper); //! 回调函数名


    xTimerStop(line_walking_Handle, 1);
    xTimerStop(sendto_Upper_Handle, 1);
    xTimerStart(analyse_data_Handle, 1);



    //xTimerStart(sendto_Upper_Handle, 0); //! 发送数据到上位机定时器

    Task_Number_Handle = xQueueCreate(1, 1); // 开始解析数据
    Group_One_Handle = xEventGroupCreate();
    Group_One_Handle = Group_One_Handle;
    
    // 挂机任务，等待选择任务
    vTaskSuspend(Task__THREE_Handle);
    vTaskSuspend(Task__TWO_Handle);
    //vTaskSuspend(Task__FOUR_Handle);
    vTaskDelete(AppTaskCreate_Handle); // 删除AppTaskCreate任务

    taskEXIT_CRITICAL(); // 退出临界区
}


/**
 * @description: this function is the software callback function that send data to upper computer
 * @return {*}
*/
static void sendto_Upper(void)
{

   
}

/**
 * @description: this function is the software callback function that keep the car walking straightly
 * @return {*}
*/
static void line_walking(void)
{
  
}

/**
 * @description: this function is the software callback function that analyse data
 * @return {*}
*/
	u8 TOF_Data[7];
int32_t temp;
static void analyse_data(void)
{
  u8 Agreement[8] = {0x57,0x10,0xff,0xff,0x01,0xff,0xff,0x64};
	const u8 TOFSENSE[] = {0x57,0x00,0xFF,0x01};
	Usart_SendArray(USART2,Agreement,8);
	Read_buff_Void(&VL53_USARTX_Buff,TOFSENSE,4,TOF_Data,7,8,1);
	 temp = (int32_t)(TOF_Data[4] << 8 | TOF_Data[5] << 16 | TOF_Data[6] << 24) / 256;
	printf("distance:%d\r\n",temp);
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

        OLED_SHOW_TASK();
			vTaskDelay(1);
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
        
      vTaskDelay(50);
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

       vTaskDelay(50);
    }
}


/**
 * @description: this task is used to control loudspeaker
 * @return {*}
 */
static void Task__THREE(void)
{
    while (1)
    {

       	Software_USART_TXD(1);     //播报打开仓库门
        vTaskDelay(5000); /* 延时500个tick */
    }
}
/**
 * @description: this task is used to control the car,make it go to the red area in the left hand side firstly,then go to the red area in the right hand side
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
static void Task__FOUR(void)
{
    while (1)
    {
//			printf("count2:%d\r\n",Read_Encoder(2));   
//			printf("count3:%d\r\n",Read_Encoder(3)); 
//			printf("count4:%d\r\n",Read_Encoder(4)); 
//			printf("count5:%d\r\n",Read_Encoder(5)); 
			vTaskDelay(500);
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
		delay_init(168);
    PWM_TIM1_config(2000, 7, Initial_Speed, Initial_Speed, Initial_Speed, Initial_Speed);
    
    Encoder_Init();
    USART3_Config();      //*USART3前激光测距
    USART4_Config_JY62(); //*UART4陀螺仪
    USART1_Config();      //*调试信息输出
    Iinitial_BUFF(&U3_buffer);
    VL53_Initial(); //*USART2右激光测距
    Initial_Control_PIN();
    Reverse();
Software_USART_IOConfig();
    //pid初始化
  
    LED_GPIO_Config();
    KEY_ONE();
    OLED_Init();

    GPIO_SetBits(GPIOE, GPIO_Pin_1);
}


//void Allocation_PID(int PIDOUT)
//{
//    if (Turn_Angle_PID.Target > 0)
//    {
//        if (PIDOUT > 0)
//        {
//            Turn_Right_Founction();
//            if (PIDOUT >= 1500)
//            {
//                /* code */
//                PIDOUT = 1500;
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 2);
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 3);
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 1);
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 4);
//            }
//            else
//            {
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 2);
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 3);
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 1);
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 4);
//            }
//        }
//        if (PIDOUT < 0)
//        {
//            Turn_Left_Founction();
//            if (PIDOUT < -1500)
//            {
//                /* code */
//                PIDOUT = -1500;
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 2);
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 3);
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 1);
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 4);
//            }
//            else
//            {
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 2);
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 3);
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 1);
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 4);
//            }
//        }
//    }
//    if (Turn_Angle_PID.Target < 0)
//    {
//        if (PIDOUT > 0)
//        {
//            Turn_Right_Founction();
//            if (PIDOUT >= 1500)
//            {
//                /* code */
//                PIDOUT = 1500;
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 2);
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 3);
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 1);
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 4);
//            }
//            else
//            {
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 2);
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 3);
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 1);
//                SetCompare1(TIM1, Turn_Speed + PIDOUT, 4);
//            }
//        }
//        if (PIDOUT < 0)
//        {
//            Turn_Left_Founction();
//            if (PIDOUT < -1500)
//            {
//                /* code */
//                PIDOUT = -1500;
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 2);
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 3);
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 1);
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 4);
//            }
//            else
//            {
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 2);
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 3);
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 1);
//                SetCompare1(TIM1, Turn_Speed - PIDOUT, 4);
//            }
//        }
//    }
//	static int i;
//    if (PIDOUT<35 & PIDOUT> -35)
//    {
//		i++;
//		if( i > 3 )
//		{
//			PULL_High();
//			xTimerStop(Turn_Angle_Handle, 0);
//			already_turned = 1;
//			i=0;
//		}
//        

//    }
//}
///********************************END OF FILE****************************/
