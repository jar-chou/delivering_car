#ifndef SY
#define SY

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "event_groups.h"
void Forward(void);
void Reverse(void);
void Turn_Right_Founction(void);
void Turn_Left_Founction(void);
void Walking_Right(void);
void Walking_Left(void);
float calculate_adjust_angle(int32_t last_X, int32_t now_X, int32_t distance_Y);

extern const uint8_t bigone[4][32];
extern const uint8_t bigtwo[4][32];
struct COLOR
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

struct angle
{
    uint8_t data[9];
    int16_t z;
};

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

extern EventGroupHandle_t Group_One_Handle;
/**
 * those following marcos are used to operate the event group
 */
#define WAIT_FOR_EVENT(specific_event_bit) xEventGroupWaitBits(Group_One_Handle, (specific_event_bit), pdTRUE, pdTRUE, portMAX_DELAY)
#define WAIT_FOR_EVENT_UNTIL(specific_event_bit, wait_time) xEventGroupWaitBits(Group_One_Handle, (specific_event_bit), pdTRUE, pdTRUE, (wait_time))
#define CHECK_EVENT(specific_event_bit) xEventGroupGetBits(Group_One_Handle) & (specific_event_bit)
#define CLEAR_EVENT(specific_event_bit) xEventGroupClearBits(Group_One_Handle, (specific_event_bit))
#define SET_EVENT(specific_event_bit) xEventGroupSetBits(Group_One_Handle, (specific_event_bit))

/**
 * those following marcos are used to define the specific event bit
 */
#define Y_HAVE_ARRIVED (1 << 0)
#define X_HAVE_ARRIVED (1 << 1)
#define W_HAVE_ARRIVED (1 << 2)
#define GOT_QR_CODE (1 << 3)

#endif
