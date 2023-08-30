
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
/**************************** ������ ********************************/
/*
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */

QueueHandle_t Task_Number_Handle = NULL;

static EventGroupHandle_t Task_Handle = NULL;
static TimerHandle_t Task_Change_Handle = NULL;    //+�����л���ʱ�����
static TimerHandle_t sendto_Upper_Handle = NULL;   //+�������ݵ���λ����ʱ�����
static TimerHandle_t line_walking_Handle = NULL;   //+Ѳ��PID��ʱ�����
static TimerHandle_t analyse_data_Handle = NULL;   //+�������ݶ�ʱ�����
static TaskHandle_t OLED_SHOW_Handle = NULL;       //+OLDE��ʾ���
static TaskHandle_t AppTaskCreate_Handle = NULL;   //+����������
static TaskHandle_t Task__ONE_Handle = NULL;       //+����1���
static TaskHandle_t Task__TWO_Handle = NULL;       //+����2���
static TaskHandle_t Task__THREE_Handle = NULL;     //+����3���
static TaskHandle_t Task__FOUR_Handle = NULL;      //+����4���
static EventGroupHandle_t Group_One_Handle = NULL; //+�¼�����
/********************************** �ں˶����� *********************************/
/*
 * �ź�������Ϣ���У��¼���־�飬�����ʱ����Щ�������ں˵Ķ���Ҫ��ʹ����Щ�ں�
 * ���󣬱����ȴ����������ɹ�֮��᷵��һ����Ӧ�ľ����ʵ���Ͼ���һ��ָ�룬������
 * �ǾͿ���ͨ��������������Щ�ں˶���
 *
 * �ں˶���˵���˾���һ��ȫ�ֵ����ݽṹ��ͨ����Щ���ݽṹ���ǿ���ʵ��������ͨ�ţ�
 * �������¼�ͬ���ȸ��ֹ��ܡ�������Щ���ܵ�ʵ��������ͨ��������Щ�ں˶���ĺ���
 * ����ɵ�
 *
 */

/******************************* ȫ�ֱ������� ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩȫ�ֱ�����
 */
#define Initial_Speed 1000
extern const char ACCCMD[3];
extern const char YAWCMD[3];
float Angle_X, Angle_Y, Angle_Z, VOFA_Data[3];
u8 X_Y[2];
extern unsigned char F8X16[];
/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void
analyse_data(void);
void Angle_Speed_X_PID_fun(u8 EN, int Coordinate_PID);
static void line_walking(void);
static void AppTaskCreate(void);           /* ���ڴ������� */
static void Task__TWO(void *pvParameters); /* Test_Task����ʵ�� */
static void OLED_SHOW(void *pvParameters); /* Test_Task����ʵ�� */
static void Task__ONE(void *pvParameters); /* Test_Task����ʵ�� */
static void BSP_Init(void);                /* ���ڳ�ʼ�����������Դ */
static void sendto_Upper(void);            /* ���ڳ�ʼ�����������Դ */
static void Task_Change(void);             /* ���ڳ�ʼ�����������Դ */
static void Task__THREE(void);             //
static void Task__FOUR(void);              //

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
 * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
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
                          (UBaseType_t)10,                    /* ��������ȼ� */
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
    xReturn = xTaskCreate((TaskFunction_t)Task__FOUR,           /* ������ں��� */
                          (const char *)"Task__FOUR",           /* �������� */
                          (uint16_t)256,                        /* ����ջ��С */
                          (void *)NULL,                         /* ������ں������� */
                          (UBaseType_t)2,                       /* ��������ȼ� */
                          (TaskHandle_t *)&Task__FOUR_Handle);  /* ������ƿ�ָ�� */
    if (xReturn == pdPASS)
        printf("Task__FOUR���񴴽��ɹ�\r\n");
    line_walking_Handle = xTimerCreate((const char *)"line_walking",
                                       (TickType_t)30,                         /* ��ʱ������ 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* ����ģʽ */
                                       (void *)1,                              /* Ϊÿ����ʱ������һ��������ΨһID */
                                       (TimerCallbackFunction_t)line_walking); //! �ص�������
    analyse_data_Handle = xTimerCreate((const char *)"analyse_data",
                                       (TickType_t)25,                         /* ��ʱ������ 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* ����ģʽ */
                                       (void *)4,                              /* Ϊÿ����ʱ������һ��������ΨһID */
                                       (TimerCallbackFunction_t)analyse_data); //! �ص�������

    sendto_Upper_Handle = xTimerCreate((const char *)"sendto_Upper",
                                       (TickType_t)40,                         /* ��ʱ������ 1000(tick) */
                                       (UBaseType_t)pdTRUE,                    /* ����ģʽ */
                                       (void *)4,                              /* Ϊÿ����ʱ������һ��������ΨһID */
                                       (TimerCallbackFunction_t)sendto_Upper); //! �ص�������
    Task_Change_Handle = xTimerCreate((const char *)"Task_Change",
                                      (TickType_t)40,                        /* ��ʱ������ 1000(tick) */
                                      (UBaseType_t)pdTRUE,                   /* ����ģʽ */
                                      (void *)4,                             /* Ϊÿ����ʱ������һ��������ΨһID */
                                      (TimerCallbackFunction_t)Task_Change); //! �ص�������

    xTimerStop(line_walking_Handle, 0);
    xTimerStop(analyse_data_Handle, 0);
    xTimerStop(sendto_Upper_Handle, 0);

    xTimerStart(sendto_Upper_Handle, 0); //! �������ݵ���λ����ʱ��
    xTimerStart(Task_Change_Handle, 0);  //! ���������л���ʱ��

    Task_Number_Handle = xQueueCreate(1, 1); // ��ʼ��������
    Task_Handle = xEventGroupCreate();
    Group_One_Handle = xEventGroupCreate();

    //+�һ����񣬵ȴ�ѡ������
    vTaskSuspend(Task__ONE_Handle);
    vTaskSuspend(Task__TWO_Handle);
    vTaskDelete(AppTaskCreate_Handle); // ɾ��AppTaskCreate����

    taskEXIT_CRITICAL(); // �˳��ٽ���
}

/**********************************************************************
 * @ ������  �� Test_Task
 * @ ����˵���� Test_Task��������
 * @ ����    ��
 * @ ����ֵ  �� ��
 ********************************************************************/

static void sendto_Upper()
{

    // VOFA_Send_float(VOFA_Data); //! �������ݸ�VOFA
}
static void line_walking()
{
    //?PID��ʼѲ��
}
void analyse_data()
{
    //?��ȡMV,K210���ݣ�˫��ͨѶ����
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
                //*����1
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
                //*����2
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
                //*����3
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
                //*����4
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
                //*����5
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
