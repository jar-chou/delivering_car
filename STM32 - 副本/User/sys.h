
#include "stm32f10x.h"
void OLED_SHOW_TASK(void);
void Read_RGB(void);
void Forward(void);
void Reverse(void);
void Turn_Right_Founction(void);
void Turn_Left_Founction(void);
void Walking_Right(void);
void Walking_Left(void);
extern struct COLOR RGB;
typedef struct COLOR
{
    u8 R;
    u8 G;
    u8 B;
};

