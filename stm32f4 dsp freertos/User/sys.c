#include "bsp.h"

#include "OLED.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "JY62.h"
#include "PID.h"
#include "Driver.h"
#include "Software_iic.h"
#include <math.h>
#include <stdbool.h>

extern int32_t temp;

void OLED_SHOW_TASK()
{
	
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


