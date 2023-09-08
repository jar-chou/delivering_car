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
 * @LastEditTime: 2023-09-08 14:00:50
 * @FilePath: \delivering_car\User\main.c
 * @Description: �������Ӵ��ļ���bug������
 */



/*
*************************************************************************
*                             ������ͷ�ļ�
*************************************************************************
*/
/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"
/* ������Ӳ��bspͷ�ļ� */
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


/**************************** ������ ********************************/
/*
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ��������������������������������Լ�����ô
 * ����������ΪNULL��
 */

QueueHandle_t Task_Number_Handle = NULL;

static TimerHandle_t Turn_Angle_Handle = NULL;     			//+�������ݵ���λ����ʱ�����
static TimerHandle_t sendto_Upper_Handle = NULL;   			//+�������ݵ���λ����ʱ�����
static TimerHandle_t line_walking_Handle = NULL;   			//+Ѳ��PID��ʱ�����
static TimerHandle_t analyse_data_Handle = NULL;   			//+�������ݶ�ʱ�����
static TimerHandle_t Achieve_Distance_For_Front_Laser_Handle = NULL;	//���ݼ�������ֱ��
static TimerHandle_t Achieve_Distance_For_Right_Laser_Handle = NULL;
static TimerHandle_t Car_Running_Handle = NULL;
static TimerHandle_t Go_Forward_Base_On_Encoder_Handle = NULL;
static TimerHandle_t Pan_Left_Base_On_Encoder_Handle = NULL;

static TaskHandle_t OLED_SHOW_Handle = NULL;       			//+OLDE��ʾ���
static TaskHandle_t AppTaskCreate_Handle = NULL;   			//+����������
static TaskHandle_t Task__ONE_Handle = NULL;       			//+����1���
static TaskHandle_t Task__TWO_Handle = NULL;       			//+����2���
static TaskHandle_t Task__THREE_Handle = NULL;     			//+����3���
static TaskHandle_t Task__FOUR_Handle = NULL;      			//+����4���
static EventGroupHandle_t Group_One_Handle = NULL; 			//+�¼�����

/******************************* Global variable declaration ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩȫ�ֱ�����
 */
#define Initial_Speed 600
#define Turn_Speed 0
extern const char ACCCMD[3];
extern const char YAWCMD[3];
int32_t Y_Speed = Initial_Speed;
int32_t X_Speed = 0;
int32_t angle_speed = 0;
float VOFA_Data[4];
extern struct Buff VL53_USARTX_Buff;
struct PID Coord, Turn_Angle_PID, X_Speed_PID, Y_Speed_PID, X_Base_On_Laser_PID, Y_Base_On_Laser_PID;
u8 already_turned = 0, Y_have_achieved = 0, X_have_achieved = 0;   //�Ƿ�ﵽ��ʱ��Ŀ�ĵ��ź���
int32_t CCR_wheel[4]={0,0,0,0};
int32_t position_of_car[3]={0,0,0};
u8 dataFromLinux[2] = {0, 0};       //the data get from linux
u8 voice[3][6] = {0xaa,0x07,0x02,0x00,0x01,0xb4,0xaa,0x07,0x02,0x00,0x02,0xb5,0xaa,0x07,0x02,0x00,0x03,0xb6};
struct distance
{
    u16 F[5];
    u16 R[5];
    u16 F_OUT;
    u16 R_OUT;
} Distance;

/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void analyse_data(void);
static void line_walking(void);
static void AppTaskCreate(void);           	/* ���ڴ������� */
static void Task__TWO(void *pvParameters); 	/* Test_Task����ʵ�� */
static void OLED_SHOW(void *pvParameters); 	/* Test_Task����ʵ�� */
static void Task__ONE(void *pvParameters); 	/* Test_Task����ʵ�� */
static void BSP_Init(void);                	/* ���ڳ�ʼ�����������Դ */
static void sendto_Upper(void);            	/* ���ڳ�ʼ�����������Դ */
static void Task__THREE(void);             	//
static void Task__FOUR(void);              	//
static void Turn_Angle(void);              	//
static void Achieve_Distance_Front_Head_Laser(void);
static void Achieve_Distance_For_Right_Laser(void);
static void Car_Running(void);
static void Go_Forward_Base_On_Encoder(void);
static void Pan_Left_Base_On_Encoder(void);
void Allocation_PID(int PIDOUT);


/*****************************************************************
  * @brief  ������
  * @param  ��
  * @retval ��
  * @note   ��һ����������Ӳ����ʼ��
            �ڶ���������APPӦ������
            ������������FreeRTOS����ʼ���������
  ****************************************************************/
int main(void)
{
    BaseType_t xReturn = pdPASS; /* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */

    /* ������Ӳ����ʼ�� */
    BSP_Init();
    /* ����AppTaskCreate���� */
    xReturn = xTaskCreate((TaskFunction_t)AppTaskCreate,          /* ������ں��� */
                          (const char *)"AppTaskCreate",          /* �������� */
                          (uint16_t)256,                          /* ����ջ��С */
                          (void *)NULL,                           /* ������ں������� */
                          (UBaseType_t)1,                         /* ��������ȼ� */
                          (TaskHandle_t *)&AppTaskCreate_Handle); /* ������ƿ�ָ�� */
    /* ����������� */
    if (pdPASS == xReturn)
        vTaskStartScheduler(); /* �������񣬿������� */
    else
        return -1;

    while (1)
        ; /* ��������ִ�е����� */
}

/***********************************************************************
 * @ ������  �� AppTaskCreate
 * @ ����˵���� Ϊ�˷�����������е����񴴽����������������������
 * @ ����    �� ��
 * @ ����ֵ  �� ��
 **********************************************************************/
static void AppTaskCreate(void)
{
    BaseType_t xReturn = pdPASS; /* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */

    taskENTER_CRITICAL(); // �����ٽ���

    /* ����Test_Task���� */
    xReturn = xTaskCreate((TaskFunction_t)Task__TWO,          /* ������ں��� */
                          (const char *)"Task__TWO",          /* �������� */
                          (uint16_t)256,                      /* ����ջ��С */
                          (void *)NULL,                       /* ������ں������� */
                          (UBaseType_t)2,                    /* ��������ȼ� */
                          (TaskHandle_t *)&Task__TWO_Handle); /* ������ƿ�ָ�� */
    if (xReturn == pdPASS)
        printf("Task__TWO���񴴽��ɹ�\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task__ONE,          /* ������ں��� */
                          (const char *)"Task__ONE",          /* �������� */
                          (uint16_t)256,                      /* ����ջ��С */
                          (void *)NULL,                       /* ������ں������� */
                          (UBaseType_t)2,                     /* ��������ȼ� */
                          (TaskHandle_t *)&Task__ONE_Handle); /* ������ƿ�ָ�� */
    if (xReturn == pdPASS)
        printf("Task__ONE���񴴽��ɹ�\r\n");
    xReturn = xTaskCreate((TaskFunction_t)OLED_SHOW,          /* ������ں��� */
                          (const char *)"OLED_SHOW",          /* �������� */
                          (uint16_t)256,                      /* ����ջ��С */
                          (void *)NULL,                       /* ������ں������� */
                          (UBaseType_t)2,                     /* ��������ȼ� */
                          (TaskHandle_t *)&OLED_SHOW_Handle); /* ������ƿ�ָ�� */
    if (xReturn == pdPASS)
        printf("OLED_SHOW���񴴽��ɹ�\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task__THREE,          /* ������ں��� */
                          (const char *)"Task__THREE",          /* �������� */
                          (uint16_t)256,                        /* ����ջ��С */
                          (void *)NULL,                         /* ������ں������� */
                          (UBaseType_t)2,                       /* ��������ȼ� */
                          (TaskHandle_t *)&Task__THREE_Handle); /* ������ƿ�ָ�� */
    if (xReturn == pdPASS)
        printf("Task__THREE���񴴽��ɹ�\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task__FOUR,          /* ������ں��� */
                          (const char *)"Task__FOUR",          /* �������� */
                          (uint16_t)256,                       /* ����ջ��С */
                          (void *)NULL,                        /* ������ں������� */
                          (UBaseType_t)2,                      /* ��������ȼ� */
                          (TaskHandle_t *)&Task__FOUR_Handle); /* ������ƿ�ָ�� */
    if (xReturn == pdPASS)
        printf("Task__FOUR���񴴽��ɹ�\r\n");
    line_walking_Handle = xTimerCreate((const char *)"line_walking",
                                       (TickType_t)30,                         /* ��ʱ������ 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* ����ģʽ */
                                       (void *)1,                              /* Ϊÿ����ʱ������һ��������ΨһID */
                                       (TimerCallbackFunction_t)line_walking); //! �ص�������
    analyse_data_Handle = xTimerCreate((const char *)"analyse_data",
                                       (TickType_t)20,                         /* ��ʱ������ 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* ����ģʽ */
                                       (void *)4,                              /* Ϊÿ����ʱ������һ��������ΨһID */
                                       (TimerCallbackFunction_t)analyse_data); //! �ص�������

    sendto_Upper_Handle = xTimerCreate((const char *)"sendto_Upper",
                                       (TickType_t)40,                         /* ��ʱ������ 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* ����ģʽ */
                                       (void *)4,                              /* Ϊÿ����ʱ������һ��������ΨһID */
                                       (TimerCallbackFunction_t)sendto_Upper); //! �ص�������
    Turn_Angle_Handle = xTimerCreate((const char *)"Turn_Angle",
                                     (TickType_t)40,                       /* ��ʱ������ 1000(tick) */
                                     (UBaseType_t)pdTRUE,                  /* ����ģʽ */
                                     (void *)4,                            /* Ϊÿ����ʱ������һ��������ΨһID */
                                     (TimerCallbackFunction_t)Turn_Angle); //! �ص�������
	Achieve_Distance_For_Front_Laser_Handle = xTimerCreate((const char *)"Achieve_Distance_Front_Head_Laser",
                                    (TickType_t)20,                      /* ��ʱ������ 1000(tick) */
                                    (UBaseType_t)pdTRUE,                 /* ����ģʽ */
                                    (void *)4,                           /* Ϊÿ����ʱ������һ��������ΨһID */
                                    (TimerCallbackFunction_t)Achieve_Distance_Front_Head_Laser); //! �ص�������
    Achieve_Distance_For_Right_Laser_Handle = xTimerCreate((const char *)"Achieve_Distance_For_Right_Laser",
                                    (TickType_t)20,                      /* ��ʱ������ 1000(tick) */
                                    (UBaseType_t)pdTRUE,                 /* ����ģʽ */
                                    (void *)4,                           /* Ϊÿ����ʱ������һ��������ΨһID */
                                    (TimerCallbackFunction_t)Achieve_Distance_For_Right_Laser); //! �ص�������
    Car_Running_Handle = xTimerCreate((const char *)"Car_Running",
                                    (TickType_t)20,                      /* ��ʱ������ 1000(tick) */
                                    (UBaseType_t)pdTRUE,                 /* ����ģʽ */
                                    (void *)4,                           /* Ϊÿ����ʱ������һ��������ΨһID */
                                    (TimerCallbackFunction_t)Car_Running); //! �ص�������
    Go_Forward_Base_On_Encoder_Handle = xTimerCreate((const char *)"Go_Forward_Base_On_Encoder",
                                    (TickType_t)20,                      /* ��ʱ������ 1000(tick) */
                                    (UBaseType_t)pdTRUE,                 /* ����ģʽ */
                                    (void *)4,                           /* Ϊÿ����ʱ������һ��������ΨһID */
                                    (TimerCallbackFunction_t)Go_Forward_Base_On_Encoder); //! �ص�������
    Pan_Left_Base_On_Encoder_Handle = xTimerCreate((const char *)"Pan_Left_Base_On_Encoder",
                                    (TickType_t)20,                      /* ��ʱ������ 1000(tick) */
                                    (UBaseType_t)pdTRUE,                 /* ����ģʽ */
                                    (void *)4,                           /* Ϊÿ����ʱ������һ��������ΨһID */
                                    (TimerCallbackFunction_t)Pan_Left_Base_On_Encoder); //! �ص�������

    xTimerStop(line_walking_Handle, 1);
    xTimerStop(sendto_Upper_Handle, 1);
    xTimerStart(analyse_data_Handle, 1);
    xTimerStop(Turn_Angle_Handle, 1);
	xTimerStop(Achieve_Distance_For_Front_Laser_Handle, 1);
    xTimerStop(Achieve_Distance_For_Right_Laser_Handle, 1);
    xTimerStop(Car_Running_Handle, 1);
    xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);
    xTimerStop(Pan_Left_Base_On_Encoder_Handle, 1);


    //xTimerStart(sendto_Upper_Handle, 0); //! �������ݵ���λ����ʱ��

    Task_Number_Handle = xQueueCreate(1, 1); // ��ʼ��������
    Group_One_Handle = xEventGroupCreate();
    Group_One_Handle = Group_One_Handle;
    
    // �һ����񣬵ȴ�ѡ������
    vTaskSuspend(Task__THREE_Handle);
    vTaskSuspend(Task__TWO_Handle);
    vTaskSuspend(Task__FOUR_Handle);
    vTaskDelete(AppTaskCreate_Handle); // ɾ��AppTaskCreate����

    taskEXIT_CRITICAL(); // �˳��ٽ���
}

/**********************************************************************
 * @ ������  �� Timer_Task
 * @ ����˵���� Timer_Task��������
 * @ ����    ��
 * @ ����ֵ  �� ��
 ********************************************************************/

/**
 * @description: this function is the software callback function that achieve pan left base on encoder num
 * @return {*}
 */
static void Pan_Left_Base_On_Encoder(void)
{
    static int i = 0;
    int32_t distance = position_of_car[0];
    int32_t speed = -(int32_t)PID_Realize(&X_Speed_PID, distance);
	speed = speed > 400 ? 400 : speed;
	speed = speed < -400 ? -400 : speed;
    X_Speed = speed;
    if (fabs(distance - X_Speed_PID.Target)<7)
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
 * @description: this function is the software callback function that achieve go forward base on encoder num
 * @return {*}
 */
static void Go_Forward_Base_On_Encoder(void)
{
    static int i = 0;
    int32_t distance = position_of_car[1];
    int32_t speed = (int32_t)PID_Realize(&Y_Speed_PID, distance);
	speed = speed > 1000 ? 1000 : speed;
	speed = speed < -1000 ? -1000 : speed;
    Y_Speed = speed;
    if (fabs(distance - Y_Speed_PID.Target)<10)
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
 * @description: this function is the software callback function that achieve reach the target distance for right laser
 * @return {*}
*/
static void Achieve_Distance_For_Right_Laser(void)
{
    static int i = 0;
    float distance = VOFA_Data[2];
    int32_t speed = -(int32_t)PID_Realize(&X_Base_On_Laser_PID, distance);
    speed = speed > 300 ? 300 : speed;
	speed = speed < -300 ? -300 : speed;
    X_Speed = speed;
    if (fabs(distance - X_Base_On_Laser_PID.Target)<5)
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
    Y_Speed = speed;
    if (fabs(distance - Y_Base_On_Laser_PID.Target)<20)
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
    CCR_wheel[1] -= Local_X_Speed;
    CCR_wheel[2] += Local_X_Speed;
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
    float Angle, PIDOUT;
    Angle = (float)stcAngle.Angle[2] / 32768 * 180;
    PIDOUT = PID_Realize(&Turn_Angle_PID, Angle);
    Allocation_PID((int)PIDOUT);
}

/**
 * @description: this function is the software callback function that send data to upper computer
 * @return {*}
*/
static void sendto_Upper(void)
{

    VOFA_Send_float(VOFA_Data, 3); //! �������ݸ�VOFA
    VOFA_Data[0] = (float)stcAngle.Angle[2] / 32768 * 180;
}

/**
 * @description: this function is the software callback function that keep the car walking straightly
 * @return {*}
*/
static void line_walking(void)
{
    float Angle;
    //?PID��ʼѲ��
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
    u8 i = 0;
    const u8 VL53_Agreement_RX[] = {0x50, 0x03, 0x02};
    for (i = 0; i < 5; i++)
    {
        VL53_Send_Agrement();
        Read_buff_Void(&VL53_USARTX_Buff, VL53_Agreement_RX, 3, &Distance.R[i], 1, 16, 1);
        Read_buff_Void(&U3_buffer, VL53_Agreement_RX, 3, &Distance.F[i], 1, 16, 1);
    }
    for (i = 0; i < 5; i++)
    {
        Distance.R_OUT += Distance.R[i];
        Distance.F_OUT += Distance.F[i];
    }
    Distance.R_OUT /= 5;
    Distance.F_OUT /= 5;
		

	
    static int32_t How_many_revolutions_of_the_motor[4] = {0,0,0,0};
    How_many_revolutions_of_the_motor[0] += Read_Encoder(2);
    How_many_revolutions_of_the_motor[1] += Read_Encoder(3);
    How_many_revolutions_of_the_motor[2] += Read_Encoder(4);
    How_many_revolutions_of_the_motor[3] += Read_Encoder(5);
		

    int32_t The_distance_that_has_gone_forward = (How_many_revolutions_of_the_motor[0]
                            +How_many_revolutions_of_the_motor[1]
                            +How_many_revolutions_of_the_motor[2]
                            +How_many_revolutions_of_the_motor[3])/4;
    int32_t The_distance_that_has_gone_right_head_side = (-How_many_revolutions_of_the_motor[0]
                            +How_many_revolutions_of_the_motor[1]
                            +How_many_revolutions_of_the_motor[2]
                            -How_many_revolutions_of_the_motor[3])/4;
	printf("%d\r\n",The_distance_that_has_gone_forward);
    taskENTER_CRITICAL();           //��������ٽ���
    VOFA_Data[2] = (float)Distance.R_OUT;           //�ұ߼�����õ��ľ���
    VOFA_Data[3] = (float)Distance.F_OUT;           //ǰ�漤����õ��ľ���
    position_of_car[0] += The_distance_that_has_gone_forward;            //the distance that car have gone forward
    position_of_car[1] += The_distance_that_has_gone_right_head_side;    //the distance that car have gone right hand side
    Read_RGB();
    taskEXIT_CRITICAL();            //�˳������ٽ���
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
    else if (which_laser == 1)  //����ǰ��ļ����������������
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
        position_of_car[0] = 0;
        xTimerStart(Pan_Left_Base_On_Encoder_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }
    else if (forwardOrPan == 1)  //���ݱ�������ǰ��
    {
        Y_Speed_PID.Target = target;
        Y_Speed_PID.Cumulation_Error = 0;
        Y_have_achieved = 0;
        position_of_car[1] = 0;
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
//	float Angle;
//	Angle = (float)stcAngle.Angle[2] / 32768 * 180;
	if(i == 1)
		Turn_Angle_PID.Target = -90;
	else if (i==0)
		Turn_Angle_PID.Target = 0;
	already_turned = 0;
	xTimerStart(Turn_Angle_Handle, 0);
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

        OLED_SHOW_TASK();
			vTaskDelay(50);
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
//		start_trun(1);
//		while(!already_turned)                          //�ȴ�ת�����
//            vTaskDelay(20);
//		while(1)
//			vTaskDelay(1000);
        //xEventGroupWaitBits(Group_One_Handle, 0x01, pdTRUE, pdTRUE, portMAX_DELAY); //! ��ʼ����
        //-���� ����
        // float Angle;
		// Angle = (float)stcAngle.Angle[2] / 32768 * 180;
		// Turn_Angle_PID.Target = Angle + 90;
		// already_turned = 0;
		// xTimerStart(Turn_Angle_Handle, 0);
        startStraight_Line_Base_On_Encoder(12500,forward);	//ֱ��
        startgostraight(0);								//��֤��ֱ��
        while(!Y_have_achieved)							//��⵽��λ��
            vTaskDelay(20);
        xTimerStop(line_walking_Handle, 1);				//ֹͣ��ֱ��
		xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);
        xTimerStop(Car_Running_Handle, 1);				//С��ֹͣ�ƶ�
		start_trun(1);                                  //��ת
		while(!already_turned)                          //�ȴ�ת�����
            vTaskDelay(20);
        startStraight_Line_For_Laser(240,pan);      //!the distance need to be changed,it is because only the center of road do not have barrier
        startgostraight(-90);       //��֤���ķ��򲻱�
        while (!X_have_achieved)
        {
            vTaskDelay(20);
        }
        xTimerStop(line_walking_Handle, 1);				//ֹͣ��ֱ��
        // here we need to switch the task that first go to the red area in the right hand side or in the left hand side

        
        if(1)        //!the code in here is unfinished,we need to judge the value of the dataFromLinux[0] to decide which task we should switch to
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

        startStraight_Line_Base_On_Encoder(-22500, forward);   //����ߵ��ұߺ�ɫ����
        startgostraight(-90);                           //��֤��ֱ��
        while(!check_rgb(1))                    //��rgb��ɫʶ���⵽���ɫ����
        vTaskDelay(20);

        xTimerStop(Car_Running_Handle, 1);              //С��ֹͣ�ƶ�
		xTimerStop(line_walking_Handle, 1);             //ֹͣ��ֱ��
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);   //ֹͣǰ����

        //!�򿪲ֿ⣬ȡ����Ʒ�����Ǻ�����Ҫ�ӵĴ���
        //����������Ųֿ�
        USART_Send(voice[2],6);
        //��ǰ�ߵ���ߺ�ɫ����
        startStraight_Line_Base_On_Encoder(11000, forward);     //!code is unfinished,the first param need to be changed

        startgostraight(-90);                           //��֤��ֱ��
        while(!Y_have_achieved)                         //��⵽��λ��
            vTaskDelay(20);
        startStraight_Line_For_Laser(240, pan);       //�����ұ߼������������
        startStraight_Line_For_Laser(190, forward);     //����ǰ�漤�����������
        while((!X_have_achieved)||(!Y_have_achieved))    //��⵽��λ��
            vTaskDelay(20);
        
        //!�򿪲ֿ⣬ȡ����Ʒ�����Ǻ�����Ҫ�ӵĴ���
        //��������һ�Ųֿ�
        USART_Send(voice[1],6);

        startStraight_Line_Base_On_Encoder(-6800, forward);    //!����ߵ�t��·�� code is unfinished,the first param need to be changed
        startgostraight(-90);                    //��֤��ֱ��
        while(!Y_have_achieved)             //��⵽��λ��
            vTaskDelay(20);
        
        xTimerStop(line_walking_Handle, 1);     //ֹͣ��ֱ��
        xTimerStop(Car_Running_Handle, 1);      //С��ֹͣ�ƶ�

        //��ʼ��ת
        start_trun(0);                                  //��ת
        while(!already_turned)                          //�ȴ�ת�����
            vTaskDelay(20);
        
        startStraight_Line_For_Laser(240, forward);     //����ǰ�漤�����������
        while(!Y_have_achieved)                         //��⵽��λ��
            vTaskDelay(20);
        
        startStraight_Line_Base_On_Encoder(-22500, forward);   //����ߵ��ұ߻�ɫ����
        startgostraight(0);                             //��֤��ֱ��
        while(!check_rgb(2))                    //��rgb��ɫʶ���⵽���ɫ����
            vTaskDelay(20);
        //С��ֹͣ�ƶ�
        xTimerStop(Car_Running_Handle, 1);
        xTimerStop(line_walking_Handle, 1);


        PULL_High();
        while(1)                                        //������ɣ��һ�
        {
            vTaskDelay(1000);
        }
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

//        	USART_Send(voice[0],6);     //�����򿪲ֿ���
        vTaskDelay(5000); /* ��ʱ500��tick */
//			USART_Send(voice[1],6);     //������һ�Ųֿ�
//        vTaskDelay(5000); /* ��ʱ500��tick */
//			USART_Send(voice[2],6);     //����������Ųֿ�
//        vTaskDelay(5000); /* ��ʱ500��tick */
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
        startStraight_Line_Base_On_Encoder(5500, forward);      //��ǰ�ߵ���ߺ�ɫ����
		startgostraight(-90);                           //��֤��ֱ��

		while(!Y_have_achieved)         //��⵽����ߺ�ɫ����
            vTaskDelay(10);
		
        startStraight_Line_For_Laser(240, pan);       //�����ұ߼������������
        startStraight_Line_For_Laser(190, forward);     //����ǰ�漤�����������
        while((!X_have_achieved)||(!Y_have_achieved))    //��⵽��λ��
            vTaskDelay(10);
		xTimerStop(Car_Running_Handle, 1);      //С��ֹͣ�ƶ�
        //!�򿪲ֿ⣬ȡ����Ʒ�����Ǻ�����Ҫ�ӵĴ���


        startStraight_Line_Base_On_Encoder(-22500, forward);    //����ߵ���ߺ�ɫ����
        startgostraight(-90);                    //��֤��ֱ��
        vTaskDelay(1500);             //��ʱ1.5s��������⵽��ߺ�ɫ����
        while(!check_rgb(1))            //��rgb��ɫʶ���⵽���ұߺ�ɫ����
        vTaskDelay(20);                 //��ʱ20ms

		xTimerStop(line_walking_Handle, 1);     //ֹͣ��ֱ��
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);   //ֹͣǰ����
        PULL_High();

        startStraight_Line_For_Laser(240, pan);       //�����ұ߼������������
        while(!X_have_achieved)         //��⵽��λ��
            vTaskDelay(20);

        //!�򿪲ֿ⣬ȡ����Ʒ�����Ǻ�����Ҫ�ӵĴ���


        startStraight_Line_Base_On_Encoder(5500, forward);    //!��ǰ�ߵ�ʮ��t��·�� code is unfinished,the first param need to be changed
        startgostraight(-90);               //��֤��ֱ��
        while(!Y_have_achieved)             //��⵽��λ��
            vTaskDelay(20);
        
        xTimerStop(line_walking_Handle, 1);     //ֹͣ��ֱ��
        //��ʼ��ת
        start_trun(0);                                  //��ת
        while(!already_turned)                          //�ȴ�ת�����
            vTaskDelay(20);

        startStraight_Line_For_Laser(240, forward);     //����ǰ�漤�����������
        while(!Y_have_achieved)                         //��⵽��λ��
            vTaskDelay(20);
        
        startStraight_Line_Base_On_Encoder(-22500, forward);   //����ߵ��ұ߻�ɫ����
        startgostraight(0);                             //��֤��ֱ��
        while(!check_rgb(2))                    //��rgb��ɫʶ���⵽���ɫ����
        vTaskDelay(20);
        //С��ֹͣ�ƶ�
        xTimerStop(Car_Running_Handle, 1);
        xTimerStop(line_walking_Handle, 1);
        xTimerStop(Go_Forward_Base_On_Encoder_Handle, 1);

        PULL_High();        //�һ� it is necessary to add the code that make the direction pin of the motor high
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
 * @ ������  �� BSP_Init
 * @ ����˵���� �弶�����ʼ�������а����ϵĳ�ʼ�����ɷ��������������
 * @ ����    ��
 * @ ����ֵ  �� ��
 ***********************************************************************/
static void BSP_Init(void)
{
    /*
     * STM32�ж����ȼ�����Ϊ4����4bit��������ʾ��ռ���ȼ�����ΧΪ��0~15
     * ���ȼ�����ֻ��Ҫ����һ�μ��ɣ��Ժ������������������Ҫ�õ��жϣ�
     * ��ͳһ��������ȼ����飬ǧ��Ҫ�ٷ��飬�мɡ�
     */
    RCC_ClocksTypeDef get_rcc_clock;
    RCC_GetClocksFreq(&get_rcc_clock);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
		delay_init(168);
	    Encoder_Init();
    PWM_TIM1_config(2000, 7, Initial_Speed, Initial_Speed, Initial_Speed, Initial_Speed);
    

    USART3_Config();      //*USART3ǰ������
    USART4_Config_JY62(); //*UART4������
    USART1_Config();      //*������Ϣ���
    Iinitial_BUFF(&U3_buffer);
    VL53_Initial(); //*USART2�Ҽ�����
    Initial_Control_PIN();
    PULL_High();

    //pid��ʼ��
    PID_Initialize(&Coord, 30, 0, 0, 0, 100, -100);         //΢��Ѳ�ߵ�pid��ʼ��
    PID_Initialize(&Turn_Angle_PID, 17.5, 0, 0, 0, 25, -25);  //ת���pid��ʼ��
    PID_Initialize(&X_Speed_PID, 3.5, 0, .5, 0, 100, -100); //x�����Զ������ڱ�������pid
    PID_Initialize(&Y_Speed_PID, 3.5, 0, .5, 0, 100, -100);   //y�����Զ������ڱ�������pid
    PID_Initialize(&X_Base_On_Laser_PID, 3.5, 0, 1., 0, 150, -150);
    PID_Initialize(&Y_Base_On_Laser_PID, 3.5, 0, 1., 0, 150, -150);
		Software_USART_IOConfig();
    LED_GPIO_Config();
    KEY_ONE();
    OLED_Init();
    IIC_Init();
    sendcmd(ACCCMD);
    Delayms(500);
    sendcmd(YAWCMD);
    Delayms(2000);
    GPIO_SetBits(GPIOE, GPIO_Pin_1);
}


void Allocation_PID(int PIDOUT)
{
    if (Turn_Angle_PID.Target > 0)
    {
        if (PIDOUT > 0)
        {
            Turn_Right_Founction();
            if (PIDOUT >= 1500)
            {
                /* code */
                PIDOUT = 1500;
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 2);
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 3);
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 1);
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 4);
            }
            else
            {
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 2);
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 3);
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 1);
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 4);
            }
        }
        if (PIDOUT < 0)
        {
            Turn_Left_Founction();
            if (PIDOUT < -1500)
            {
                /* code */
                PIDOUT = -1500;
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 2);
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 3);
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 1);
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 4);
            }
            else
            {
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 2);
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 3);
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 1);
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 4);
            }
        }
    }
    if (Turn_Angle_PID.Target < 0)
    {
        if (PIDOUT > 0)
        {
            Turn_Right_Founction();
            if (PIDOUT >= 1500)
            {
                /* code */
                PIDOUT = 1500;
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 2);
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 3);
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 1);
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 4);
            }
            else
            {
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 2);
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 3);
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 1);
                SetCompare1(TIM1, Turn_Speed + PIDOUT, 4);
            }
        }
        if (PIDOUT < 0)
        {
            Turn_Left_Founction();
            if (PIDOUT < -1500)
            {
                /* code */
                PIDOUT = -1500;
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 2);
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 3);
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 1);
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 4);
            }
            else
            {
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 2);
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 3);
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 1);
                SetCompare1(TIM1, Turn_Speed - PIDOUT, 4);
            }
        }
    }
	static int i;
    if (PIDOUT<35 & PIDOUT> -35)
    {
		i++;
		if( i > 3 )
		{
			PULL_High();
			xTimerStop(Turn_Angle_Handle, 0);
			already_turned = 1;
			i=0;
		}
        

    }
}
/********************************END OF FILE****************************/
