#ifndef MY_TASKS_H
#define MY_TASKS_H

// including freertos lib
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"

extern TaskHandle_t OLED_SHOW_Handle;     //+OLDE显示句柄
extern TaskHandle_t AppTaskCreate_Handle; //+创建任务句柄
extern TaskHandle_t Task__ONE_Handle;     //+任务1句柄
extern TaskHandle_t Task__TWO_Handle;     //+任务2句柄
extern TaskHandle_t Get_Start_Handle;     //+任务3句柄
extern TaskHandle_t Task__FOUR_Handle;    //+任务4句柄
extern TaskHandle_t Task__FIVE_Handle;    //+任务4句柄

// void AppTaskCreate(void);             /* 用于创建任务 */
void Task__TWO(void *pvParameters);  /* Test_Task任务实现 */
void OLED_SHOW(void *pvParameters);  /* Test_Task任务实现 */
void Task__ONE(void *pvParameters);  /* Test_Task任务实现 */
void Task__FIVE(void *parameter);    /* Test_Task任务实现 */
void Get_Start(void *pvParameters);  //
void Task__FOUR(void *pvParameters); //

#endif
