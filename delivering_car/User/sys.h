#ifndef SYS_H
#define SYS_H

#include "stm32f10x.h"
#include <stdbool.h>
#include <stdlib.h>
void OLED_SHOW_TASK(void);
void Read_RGB(void);
void Forward(void);
void Reverse(void);
void Turn_Right_Founction(void);
void Turn_Left_Founction(void);
void Walking_Right(void);
void Walking_Left(void);

/**
 * @description: this function is used to check whether all the elements in the array are in the range
 * @param {u8} *charArray :the array that you want to check
 * @param {int} length :the length of the array
 * @param {int} min :the min value of the range
 * @param {int} max :the max value of the range
 * @return {bool} :if all the elements in the array are in the range,return true,else return false
 */
static inline bool check_whetherCharArrayInRange(u8 *charArray, int length, int min, int max)
{
    for (size_t i = 0; i < length; i++)
    {
        if (charArray[i] < min || charArray[i] > max)
            return false;
    }
    return true;
}

extern struct COLOR RGB;
struct COLOR
{
    u8 R;
    u8 G;
    u8 B;
};

#endif
