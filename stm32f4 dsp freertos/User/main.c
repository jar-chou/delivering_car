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
#include "bsp_led.h"
#include "usart.h"
#include "delay.h"
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
#include "my_tasks.h"
// #include "bsp.h"

/* arm或c标准库 */
#include "stdio.h"
#include <math.h>
#include <stdbool.h>
#include "arm_math.h"
#include "kalman filter.h"

/* freertos timer callback */
#include "software_timer_tasks.h"

/**************************** 任务句柄 ********************************/
/*
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */

QueueHandle_t Task_Number_Handle = NULL;

/******************************* Global variable declaration ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */
#define Initial_Speed 0
#define Turn_Speed 0

struct PID Coord, Turn_Angle_PID, X_Speed_PID, Y_Speed_PID, X_Base_On_Laser_PID, Y_Base_On_Laser_PID;
static EventGroupHandle_t Group_One_Handle = NULL; //+事件组句柄
/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void AppTaskCreate(void); /* 用于创建任务 */
static void BSP_Init(void);      /* 用于初始化板载相关资源 */
static void Allocation_PID(int PIDOUT);

/*
*********************************************************************************************************
*	函 数 名: DSP_MatMultiply
*	功能说明: here is a example that show how to multiple using cmsis dsp lib
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_MatMultiply(void)
{
  uint8_t i;

  arm_status sta;

  /****浮点数数组******************************************************************/
  float32_t pDataA[12] = {1.1f, 1.1f, 2.1f, 2.1f, 3.1f, 3.1f, 4.1f, 4.1f, 5.1f, 0, 0, 0}; // 顺序是一行一行的排序，例如这个矩阵最后一行是0向量
  float32_t pDataB[9] = {1.1f, 1.1f, 2.1f, 2.1f, 3.1f, 3.1f, 4.1f, 4.1f, 5.1f};
  float32_t pDataDst[12];

  arm_matrix_instance_f32 pSrcA; // 4行3列数据
  arm_matrix_instance_f32 pSrcB; // 3行3列数据;
  arm_matrix_instance_f32 pDst;

  /****浮点数***********************************************************************/
  pSrcA.numCols = 3;
  pSrcA.numRows = 4;
  pSrcA.pData = pDataA;

  pSrcB.numCols = 3;
  pSrcB.numRows = 3;
  pSrcB.pData = pDataB;

  pDst.numCols = 3;
  pDst.numRows = 4;
  pDst.pData = pDataDst;

  sta = arm_mat_mult_f32(&pSrcA, &pSrcB, &pDst);

  /*
    sta = ARM_MATH_SUCCESS, 即返回0，表示点积成功。
    sta = ARM_MATH_SINGULAR, 即返回-5，表示点积失败，也表示矩阵不匹配。
    注意，ARM提供的DSP库逆矩阵求发有局限性，通过Matlab验证是可以求逆矩阵的，而DSP库却不能正确求解。

  */
  printf("----sta %d\r\n", sta);

  for (i = 0; i < 12; i++)
  {
    printf("pDataB[%d] = %f\r\n", i, pDataDst[i]);
  }
}

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
  DSP_MatMultiply();
  // while(1);
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
                        (UBaseType_t)3,                     /* 任务的优先级 */
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
  xReturn = xTaskCreate((TaskFunction_t)Get_Start,          /* 任务入口函数 */
                        (const char *)"Get_Start",          /* 任务名字 */
                        (uint16_t)512,                      /* 任务栈大小 */
                        (void *)NULL,                       /* 任务入口函数参数 */
                        (UBaseType_t)3,                     /* 任务的优先级 */
                        (TaskHandle_t *)&Get_Start_Handle); /* 任务控制块指针 */
  if (xReturn == pdPASS)
    printf("Task__THREE任务创建成功\r\n");
  xReturn = xTaskCreate((TaskFunction_t)Task__FOUR,          /* 任务入口函数 */
                        (const char *)"Task__FOUR",          /* 任务名字 */
                        (uint16_t)512,                       /* 任务栈大小 */
                        (void *)NULL,                        /* 任务入口函数参数 */
                        (UBaseType_t)3,                      /* 任务的优先级 */
                        (TaskHandle_t *)&Task__FOUR_Handle); /* 任务控制块指针 */
  if (xReturn == pdPASS)
    printf("Task__FOUR任务创建成功\r\n");
  xReturn = xTaskCreate((TaskFunction_t)Task__FIVE,          /* 任务入口函数 */
                        (const char *)"Task__FIVE",          /* 任务名字 */
                        (uint16_t)512,                       /* 任务栈大小 */
                        (void *)NULL,                        /* 任务入口函数参数 */
                        (UBaseType_t)3,                      /* 任务的优先级 */
                        (TaskHandle_t *)&Task__FIVE_Handle); /* 任务控制块指针 */
  if (xReturn == pdPASS)
    printf("Task__FIVE任务创建成功\r\n");
  line_walking_Handle = xTimerCreate((const char *)"line_walking",
                                     (TickType_t)20,                         /* 定时器周期 1000(tick) */
                                     (UBaseType_t)pdTRUE,                    /* 周期模式 */
                                     (void *)1,                              /* 为每个计时器分配一个索引的唯一ID */
                                     (TimerCallbackFunction_t)line_walking); //! 回调函数名
  analyse_data_Handle = xTimerCreate((const char *)"analyse_data",
                                     (TickType_t)20,                         /* 定时器周期 1000(tick) */
                                     (UBaseType_t)pdTRUE,                    /* 周期模式 */
                                     (void *)2,                              /* 为每个计时器分配一个索引的唯一ID */
                                     (TimerCallbackFunction_t)analyse_data); //! 回调函数名

  sendto_Upper_Handle = xTimerCreate((const char *)"sendto_Upper",
                                     (TickType_t)5,                          /* 定时器周期 1000(tick) */
                                     (UBaseType_t)pdTRUE,                    /* 周期模式 */
                                     (void *)3,                              /* 为每个计时器分配一个索引的唯一ID */
                                     (TimerCallbackFunction_t)sendto_Upper); //! 回调函数名
  Turn_Angle_Handle = xTimerCreate((const char *)"Turn_Angle",
                                   (TickType_t)20,                       /* 定时器周期 1000(tick) */
                                   (UBaseType_t)pdTRUE,                  /* 周期模式 */
                                   (void *)4,                            /* 为每个计时器分配一个索引的唯一ID */
                                   (TimerCallbackFunction_t)Turn_Angle); //! 回调函数名
  Achieve_Distance_For_Front_Laser_Handle = xTimerCreate((const char *)"Achieve_Distance_Front_Head_Laser",
                                                         (TickType_t)20,                                              /* 定时器周期 1000(tick) */
                                                         (UBaseType_t)pdTRUE,                                         /* 周期模式 */
                                                         (void *)5,                                                   /* 为每个计时器分配一个索引的唯一ID */
                                                         (TimerCallbackFunction_t)Achieve_Distance_Front_Head_Laser); //! 回调函数名
  Achieve_Distance_For_Right_Laser_Handle = xTimerCreate((const char *)"Achieve_Distance_For_Right_Laser",
                                                         (TickType_t)20,                                             /* 定时器周期 1000(tick) */
                                                         (UBaseType_t)pdTRUE,                                        /* 周期模式 */
                                                         (void *)6,                                                  /* 为每个计时器分配一个索引的唯一ID */
                                                         (TimerCallbackFunction_t)Achieve_Distance_For_Right_Laser); //! 回调函数名
  Car_Running_Handle = xTimerCreate((const char *)"Car_Running",
                                    (TickType_t)20,                        /* 定时器周期 1000(tick) */
                                    (UBaseType_t)pdTRUE,                   /* 周期模式 */
                                    (void *)7,                             /* 为每个计时器分配一个索引的唯一ID */
                                    (TimerCallbackFunction_t)Car_Running); //! 回调函数名
  Go_Forward_Base_On_Encoder_Handle = xTimerCreate((const char *)"Go_Forward_Base_On_Encoder",
                                                   (TickType_t)20,                                       /* 定时器周期 1000(tick) */
                                                   (UBaseType_t)pdTRUE,                                  /* 周期模式 */
                                                   (void *)8,                                            /* 为每个计时器分配一个索引的唯一ID */
                                                   (TimerCallbackFunction_t)Go_Forward_Base_On_Encoder); //! 回调函数名
  Pan_Left_Base_On_Encoder_Handle = xTimerCreate((const char *)"Pan_Left_Base_On_Encoder",
                                                 (TickType_t)20,                                     /* 定时器周期 1000(tick) */
                                                 (UBaseType_t)pdTRUE,                                /* 周期模式 */
                                                 (void *)9,                                          /* 为每个计时器分配一个索引的唯一ID */
                                                 (TimerCallbackFunction_t)Pan_Left_Base_On_Encoder); //! 回调函数名
  Keep_The_Car_Have_Special_Right_Distance_Handle = xTimerCreate((const char *)"Keep_The_Car_Have_Special_Right_Distance",
                                                                 (TickType_t)20,                                                     /* 定时器周期 1000(tick) */
                                                                 (UBaseType_t)pdTRUE,                                                /* 周期模式 */
                                                                 (void *)9,                                                          /* 为每个计时器分配一个索引的唯一ID */
                                                                 (TimerCallbackFunction_t)Keep_The_Car_Have_Special_Right_Distance); //! 回调函数名

  xTimerStart(analyse_data_Handle, 1);
  xTimerStop(line_walking_Handle, 1);
  xTimerStop(sendto_Upper_Handle, 1);
  xTimerStop(Turn_Angle_Handle, 1);
  xTimerStop(Achieve_Distance_For_Front_Laser_Handle, 1);
  xTimerStop(Achieve_Distance_For_Right_Laser_Handle, 1);
  xTimerStop(Car_Running_Handle, 1);
  xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);
  xTimerStop(Pan_Left_Base_On_Encoder_Handle, 1);
  xTimerStop(Keep_The_Car_Have_Special_Right_Distance_Handle, 1);

  // xTimerStart(sendto_Upper_Handle, 0); //! 发送数据到上位机定时器

  Task_Number_Handle = xQueueCreate(1, 1); // 开始解析数据
  Group_One_Handle = xEventGroupCreate();
  Group_One_Handle = Group_One_Handle;

  // 挂机任务，等待选择任务
  vTaskSuspend(Task__ONE_Handle);
  vTaskSuspend(Task__TWO_Handle);
  vTaskSuspend(Task__FOUR_Handle);
  vTaskSuspend(Task__FIVE_Handle);
  vTaskDelete(AppTaskCreate_Handle); // 删除AppTaskCreate任务

  taskEXIT_CRITICAL(); // 退出临界区
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
  PWM_TIM8_config(20000, 72, 920, 1520, 3, 2); // this is the pwm pin that control server motor
  PWM_TIM1_config(2000, 3, Initial_Speed, Initial_Speed, Initial_Speed, Initial_Speed);
	Forward();
  Encoder_Init();
  Init_USART3_All();    //*USART3前激光测距
  Init_UART4_All(); //*Linux 上位机通信
  Init_USART1_All();    //*调试信息输出
  Init_USART2_All();    //*101陀螺仪
  VL53_Initial(); //*UART5右激光测距
                  // Init_UART5_All();			//
  Initial_Control_PIN();
  Reverse();
  Software_USART_IOConfig();

  // pid初始化
  PID_Initialize(&Coord, 45, 3, 0, 0, 25, -25);            // 微调巡线的pid初始化
  PID_Initialize(&Turn_Angle_PID, 30, 0, 10, 0, 25, -25);  // 转弯的pid初始化
  PID_Initialize(&X_Speed_PID, 3.75, 0, .5, 0, 125, -125); // x方向的远距离基于编码器的pid
  PID_Initialize(&Y_Speed_PID, 2.5, 0, .35, 0, 200, -200); // y方向的远距离基于编码器的pid
  PID_Initialize(&X_Base_On_Laser_PID, 25, 0, 1., 0, 125, -125);
  PID_Initialize(&Y_Base_On_Laser_PID, 50, 0, 1., 0, 125, -125);

  Buzzer_ONE(); // 开始启动小车
  LED_GPIO_Config();
  KEY_ONE();
  OLED_Init();

  GPIO_SetBits(GPIOE, GPIO_Pin_1);
}

///********************************END OF FILE****************************/
