
#ifndef buffer__
#define buffer__
#include "stm32f4xx.h"

#define BUFFER_SIZE 64
#define BUFFER_SIZE_U1 30
#define BUFFER_SIZE_U2 30
#define BUFFER_SIZE_U3 30
#define BUFFER_SIZE_U4 32
#define BUFFER_SIZE_U5 30
struct Buff
{
    u8 Data[BUFFER_SIZE];
    u8 *head_p;
    u8 *end_p;
    u8 *write_p;
    u8 *read_p;
    u8 max;
};
// extern struct Buff BUFF;
extern struct Buff U3_buffer, U2_buffer, IIC_buff, U4_buffer, U5_buffer;
void Iinitial_BUFF(struct Buff *BUFF, u8 size_);
void Write_BUFF_P(u8 num, struct Buff *BUFF);
void Write_BUFF(u8 *P, struct Buff *BUFF);
u8 Read_BUFF(struct Buff *BUFF);
u8 Find_Char(struct Buff *BUFF, char *p);
u8 Read_buff_Void(struct Buff *BUFF, const u8 *head, u8 head_number, void *data, u8 data_number, u8 size, u8 bigfron);
u8 have_enough_data(volatile struct Buff *BUFF, u8 head_number, u8 data_number, u8 size);
#endif
