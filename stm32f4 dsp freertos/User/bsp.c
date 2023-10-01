#include "sys.h"
#include "stdio.h"
#include "OLED.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "JY62.h"
#include "PID.h"
#include "Driver.h"
#include "Software_iic.h"
#include <math.h>
#include <stdbool.h>

struct COLOR RGB;
int count = 0;
extern float VOFA_Data[4];
extern u8 already_turned, Y_have_achieved, X_have_achieved;
extern int32_t Y_Speed;
extern int32_t X_Speed;
extern int32_t angle_speed;

extern struct PID Coord, Turn_Angle_PID;
extern int32_t position_of_car[3];
extern int32_t CCR_wheel[4];

extern bool check_rgb(int color);


void OLED_SHOW_TASK()
{
    OLED_Clear();
    char buff[20];
	float current_angle = (float)stcAngle.Angle[2] / 32768 * 180;
    sprintf(buff, "Angle_Z:%.2f", current_angle);
    OLED_ShowString(1, 1, buff);
	sprintf(buff, "R:%d G:%d B:%d", RGB.R,RGB.G,RGB.B);
    OLED_ShowString(2, 1, buff);
    sprintf(buff, "RD:%.1f", VOFA_Data[2]);
    OLED_ShowString(3, 1, buff);
    sprintf(buff, "FD:%.1f", VOFA_Data[3]);
    OLED_ShowString(3, 9, buff);
    sprintf(buff, "%d", check_rgb(1));
    OLED_ShowString(4,1,buff);
}
void Read_RGB()
{
    IIC_Read_Len(0X5A, 0X0C, 3, (u8*)&RGB);
}
void Forward()
{
    Advance(2);
    Advance(3);
    Advance(4);
    Advance(5);
}
void Reverse()
{
    Back(2);
    Back(3);
    Back(4);
    Back(5);
}
void Turn_Right_Founction()
{
    Back(2);
    Advance(3);
    Advance(4);
    Back(5);
}
void Turn_Left_Founction()
{
    Advance(2);
    Back(3);
    Back(4);
    Advance(5);
}
void Walking_Right()
{
    Advance(2);
    Back(3);
    Advance(4);
    Back(5);
}
void Walking_Left()
{
    Back(2);
    Advance(3);
    Back(4);
    Advance(5);
}


