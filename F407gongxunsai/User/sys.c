#include "bsp.h"
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
#include "oled_draw.h"
#include "oled_buffer.h"

extern int32_t temp;

void OLED_SHOW_TASK()
{
	static int i = 0;
    ClearScreenBuffer(0x00);
    UpdateScreenDisplay();
	
    char buff[20];
	float current_angle = (float)stcAngle.Angle[2] / 32768 * 180;
    sprintf(buff, "Angle_Z:%.2f", current_angle);
    DrawString(0, 0, buff);
	sprintf(buff, "Dis:%d", i++);
    DrawString(1, 0, buff);
    UpdateScreenDisplay();
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


