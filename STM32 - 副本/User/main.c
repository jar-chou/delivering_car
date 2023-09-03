
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
#include "PID.h"
#include "limits.h"
#include "encoder.h"
#include "mpu6050.h"
#include "OLED.h"
#include "VL53.h"
#include "sys.h"
#include <math.h>

/**************************** ������ ********************************/
/*
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
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
static TaskHandle_t OLED_SHOW_Handle = NULL;       			//+OLDE��ʾ���
static TaskHandle_t AppTaskCreate_Handle = NULL;   			//+����������
static TaskHandle_t Task__ONE_Handle = NULL;       			//+����1���
static TaskHandle_t Task__TWO_Handle = NULL;       			//+����2���
static TaskHandle_t Task__THREE_Handle = NULL;     			//+����3���
static TaskHandle_t Task__FOUR_Handle = NULL;      			//+����4���
static EventGroupHandle_t Group_One_Handle = NULL; 			//+�¼�����

/******************************* ȫ�ֱ������� ************************************/
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
extern struct PID Coord, Turn_Angle_PID, X_Speed_PID, Y_Speed_PID;
u8 already_turned = 0, Y_have_achieved = 0, X_have_achieved = 0;   //�Ƿ�ﵽ��ʱ��Ŀ�ĵ��ź���
int32_t CCR_wheel[4]={0,0,0,0};

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
static void
analyse_data(void);
void Angle_Speed_X_PID_fun(u8 EN, int Coordinate_PID);
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
                                       (TickType_t)10,                         /* ��ʱ������ 1000(tick) */
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

    xTimerStop(line_walking_Handle, 0);
    xTimerStop(sendto_Upper_Handle, 0);
    xTimerStart(analyse_data_Handle, 0);
    xTimerStop(Turn_Angle_Handle, 0);
	xTimerStop(Achieve_Distance_For_Front_Laser_Handle, 0);
    xTimerStop(Achieve_Distance_For_Right_Laser_Handle, 0);
    xTimerStop(Car_Running_Handle, 0);


    //xTimerStart(sendto_Upper_Handle, 0); //! �������ݵ���λ����ʱ��

    Task_Number_Handle = xQueueCreate(1, 1); // ��ʼ��������
    Group_One_Handle = xEventGroupCreate();

    // �һ����񣬵ȴ�ѡ������
    //  vTaskSuspend(Task__ONE_Handle);
    vTaskSuspend(Task__TWO_Handle);
    vTaskDelete(AppTaskCreate_Handle); // ɾ��AppTaskCreate����

    taskEXIT_CRITICAL(); // �˳��ٽ���
}

/**********************************************************************
 * @ ������  �� Timer_Task
 * @ ����˵���� Timer_Task��������
 * @ ����    ��
 * @ ����ֵ  �� ��
 ********************************************************************/

static void Achieve_Distance_For_Right_Laser(void)
{
    static int i = 0;
    X_Speed = -(int32_t)PID_Realize(&X_Speed_PID, Distance.R_OUT);
    if (fabs(Distance.R_OUT - X_Speed_PID.Target)<7)
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


static void Achieve_Distance_Front_Head_Laser()
{
    static int i = 0;
    Y_Speed = -(int32_t)PID_Realize(&Y_Speed_PID, Distance.F_OUT);
	Y_Speed = Y_Speed > 800 ? 800 : Y_Speed;
	Y_Speed = Y_Speed < -800 ? -800 : Y_Speed;
    if (fabs(Distance.F_OUT - Y_Speed_PID.Target)<10)
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

static void Car_Running()
{
    
    CCR_wheel[0] = Y_Speed;
    CCR_wheel[1] = Y_Speed;
    CCR_wheel[2] = Y_Speed;
    CCR_wheel[3] = Y_Speed;
	
	Y_Speed = 0;

    CCR_wheel[0] += X_Speed;
    CCR_wheel[1] -= X_Speed;
    CCR_wheel[2] += X_Speed;
    CCR_wheel[3] -= X_Speed;
	
	X_Speed = 0;

    CCR_wheel[0] -= angle_speed;
    CCR_wheel[1] += angle_speed;
    CCR_wheel[2] += angle_speed;
    CCR_wheel[3] -= angle_speed;
	
	angle_speed = 0;
    
    if (CCR_wheel[0] > 0)
    {
        Advance(2);
    }
    else
    {
        Back(2);
        CCR_wheel[0]=-CCR_wheel[0];
    }

    if (CCR_wheel[1] > 0)
    {
        Advance(3);
    }
    else
    {
        Back(3);
        CCR_wheel[1]=-CCR_wheel[1];
    }

    if (CCR_wheel[2] > 0)
    {
        Advance(4);
    }
    else
    {
        Back(4);
        CCR_wheel[2]=-CCR_wheel[2];
    }

    if (CCR_wheel[3] > 0)
    {
        Advance(5);
    }
    else
    {
        Back(5);
        CCR_wheel[3]=-CCR_wheel[3];
    }
    
//    for (size_t i = 0; i < 4; i++)
//    {
//        if (CCR_wheel[i]>0)
//        {
//            Advance(i+2);
//        }
//        else
//        {
//            Back(i+2);
//        }
//    }
    
    SetCompare1(TIM1, CCR_wheel[0], 1);
    SetCompare1(TIM1, CCR_wheel[1], 2);
    SetCompare1(TIM1, CCR_wheel[2], 3);
    SetCompare1(TIM1, CCR_wheel[3], 4);

}


static void Turn_Angle()
{
    float Angle, PIDOUT;
    Angle = (float)stcAngle.Angle[2] / 32768 * 180;
    PIDOUT = PID_Realize(&Turn_Angle_PID, Angle);
    Allocation_PID((int)PIDOUT);
}

static void sendto_Upper()
{

    VOFA_Send_float(VOFA_Data, 3); //! �������ݸ�VOFA
    VOFA_Data[0] = (float)stcAngle.Angle[2] / 32768 * 180;
}
static void line_walking()
{
    // float a, Angle;
    float Angle;
    //?PID��ʼѲ��
    Angle = (float)stcAngle.Angle[2] / 32768 * 180;
    angle_speed = PID_Realize(&Coord, Angle);
    
    // VOFA_Data[1] = a;
    // Angle_Speed_X_PID_fun(0, (int)a);
}

void analyse_data()
{
    u8 i = 0;
    const u8 VL53_Agreement_RX[] = {0x50, 0x03, 0x02};
    for (i = 0; i < 3; i++)
    {
        VL53_Send_Agrement();
        Read_buff_Void(&VL53_USARTX_Buff, VL53_Agreement_RX, 3, &Distance.R[i], 1, 16, 1);
        Read_buff_Void(&U3_buffer, VL53_Agreement_RX, 3, &Distance.F[i], 1, 16, 1);
    }
    for (i = 0; i < 3; i++)
    {
        Distance.R_OUT += Distance.R[i];
        Distance.F_OUT += Distance.F[i];
    }
    Distance.R_OUT /= 5;
    Distance.F_OUT /= 5;
    VOFA_Data[2] = (float)Distance.R_OUT;
    VOFA_Data[3] = (float)Distance.F_OUT;
    Read_RGB();
    
}


/**
 * @description: 
 * @param {float} target        what is the distance of laser you want?
 * @param {int} which_laser     which is the laser that you want to refer to? When the value is equal to 1, it indicates the laser on the front of the reference
 * @return {*}
 */
void startStraight_Line_For_Laser(float target, int which_laser)
{
    

    if (which_laser == 0)
    {
        X_Speed_PID.Target = target;
        X_Speed_PID.Cumulation_Error = 0;
        X_have_achieved = 0;
        xTimerStart(Achieve_Distance_For_Right_Laser_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }
    else if (which_laser == 1)  //����ǰ��ļ����������������
    {
        Y_Speed_PID.Target = target;
        Y_Speed_PID.Cumulation_Error = 0;
        Y_have_achieved = 0;
        xTimerStart(Achieve_Distance_For_Front_Laser_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }

     

}

void startgostraight(float target_angle)
{
	Coord.Target = target_angle;
	xTimerStart(line_walking_Handle, 1);
}

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
}

//void start
/**********************************************************************
 * @ ������  �� Test_Task
 * @ ����˵���� Test_Task��������
 * @ ����    ��
 * @ ����ֵ  �� ��
 ********************************************************************/



static void OLED_SHOW(void *pvParameters)
{
    while (1)
    {

        OLED_SHOW_TASK();
    }
}
static void Task__ONE(void *parameter)
{



    // char qrcode=0x07;
    while (1)
    {
        

        // startStraight_Line_For_Laser(150,0);
        // startStraight_Line_For_Laser(150,1);
        // while((!X_have_achieved)&&(!Y_have_achieved))
        //     vTaskDelay(10);
		// xTimerStop(Car_Running_Handle, 1);
		// xTimerStop(line_walking_Handle, 1);
        


        //xEventGroupWaitBits(Group_One_Handle, 0x01, pdTRUE, pdTRUE, portMAX_DELAY); //! ��ʼ����
        //-���� ����
        // float Angle;
		// Angle = (float)stcAngle.Angle[2] / 32768 * 180;
		// Turn_Angle_PID.Target = Angle + 90;
		// already_turned = 0;
		// xTimerStart(Turn_Angle_Handle, 0);
        startStraight_Line_For_Laser(150,1);
		startgostraight((float)stcAngle.Angle[2] / 32768 * 180);
        while(!Y_have_achieved)
            vTaskDelay(10);
		xTimerStop(Car_Running_Handle, 1);
		xTimerStop(line_walking_Handle, 1);
		float currentangle = (float)stcAngle.Angle[2] / 32768 * 180;
		start_trun(1);//��ת
		while(!already_turned)
            vTaskDelay(10);
		
        startStraight_Line_For_Laser(150,1);
		startgostraight(currentangle-90);
		
		while(!Y_have_achieved)
            vTaskDelay(10);
		
		xTimerStop(Car_Running_Handle, 0);
		// xTimerStop(line_walking_Handle, 1);
		
        startStraight_Line_For_Laser(150,0);
        startStraight_Line_For_Laser(150,1);
        while((!X_have_achieved)||(!Y_have_achieved))
            vTaskDelay(10);
		xTimerStop(Car_Running_Handle, 1);
		xTimerStop(line_walking_Handle, 1);
        PULL_High();

        while(1)
        vTaskDelay(10);
        
        //-����
        Forward();
        //xTimerStart(line_walking_Handle, 0);
        vTaskDelay(300);
        //ȥʮ��·��
        while (1)
		{
			if (Distance.F_OUT<600 & Distance.F_OUT> 100)
            {
                PULL_High();
                //xTimerStop(line_walking_Handle, 0);
				vTaskDelay(15);
                break;
            }
		}
        //����ʮ��·��

        //�ȴ�ʶ���ά��
        while(1)
        {

        }
        //ʶ���ά��ɹ�

        /*
        ���ݶ�ά��ȥָ�����ջ���
        */
       //Go to the first pickup point
        
		float Angle;
		Angle = (float)stcAngle.Angle[2] / 32768 * 180;
		Turn_Angle_PID.Target = Angle - 90;
		already_turned = 0;
		xTimerStart(Turn_Angle_Handle, 0);
        

        while(!already_turned)//�ȴ�ת�����
        {
            
        }
		
        while(1)
        {
            
        }
        
        
        
        
        // printf("HHHHHHH");
        vTaskDelay(100);
    }
}

static void Task__TWO(void *parameter)
{
    while (1)
    {

        vTaskDelay(1000);
    }
}
static void Task__THREE(void)
{
    while (1)
    {

        vTaskDelay(1000); /* ��ʱ500��tick */
    }
}
static void Task__FOUR(void)
{
    while (1)
    {
        // xEventGroupSetBits(Group_One_Handle, 0x01);
        // printf("angle:%.2f",(float)stcAngle.Angle[2] / 32768 * 180);
        vTaskDelay(1000);
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
    TIMX_Delay_Init(RCC_APB1Periph_TIM6, 65530, 72, TIM6);
    Encoder_Init();
    USART3_Config();      //*USART3ǰ������
    USART4_Config_JY62(); //*UART4������
    USART1_Config();      //*������Ϣ���
    Iinitial_BUFF(&U3_buffer);
    VL53_Initial(); //*USART2�Ҽ�����
    Initial_Control_PIN();
    PULL_High();
    //pid��ʼ��
    PID_Initialize(&Coord, 10, 0, 0, 0, 100, -100);
    PID_Initialize(&Turn_Angle_PID, 20, 0, 0, 0, 25, -25);
    PID_Initialize(&X_Speed_PID, 3.5, 0, .5, 0, 100, -100);
    PID_Initialize(&Y_Speed_PID, 3, 0, .5, 0, 100, -100);

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

void Angle_Speed_X_PID_fun(u8 EN, int Coordinate_PID)
{

    if (EN == 1)
    {
    }

    if (Coordinate_PID >= 0)
    {
        /* code */

        if (Coordinate_PID >= 600)
        {
            /* code */
            Coordinate_PID = 600;
            SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 2);
            SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 3);
            SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 1);
            SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 4);
        }
        else
        {
            SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 2);
            SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 3);
            SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 1);
            SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 4);
        }
    }
    if (Coordinate_PID <= 0)
    {
        /* code */

        if (Coordinate_PID < -600)
        {
            /* code */
            Coordinate_PID = -600;
            SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 2);
            SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 3);
            SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 1);
            SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 4);
        }
        else
        {
            SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 2);
            SetCompare1(TIM1, Initial_Speed + Coordinate_PID, 3);
            SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 1);
            SetCompare1(TIM1, Initial_Speed - Coordinate_PID, 4);
        }
    }
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
    if (PIDOUT<200 & PIDOUT> -200)
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
