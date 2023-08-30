/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-07-18 23:25:24
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-08-01 17:18:59
 * @FilePath: \RVMDK��uv5��f:\Desktop\��ҩС��\STM32 - ����\User\buffer.h
 * @Description: ����Ĭ������,������`customMade`, ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef buffer__
#define buffer__
#include "stm32f10x.h"

#define BUFFER_SIZE 128
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

extern struct Buff U3_buffer, U2_buffer, IIC_buff;
void Iinitial_BUFF(struct Buff *BUFF);
void Write_BUFF(u8 *P, struct Buff *BUFF);
u8 Read_BUFF(struct Buff *BUFF);
void Read_Data(struct Buff *BUFF, u8 head1, u8 head2, u8 head3,u8 count ,u8 *p);
u8 Find_Char(struct Buff *BUFF, char *p);
void Analysis_Contract(struct Buff *BUFF,char *Contract,u8 *data);
u8 Read_buff(struct Buff *BUFF,const u8 *head, u8 head_number, u8 *data, u8 data_number);
u8 Read_buff_Void(struct Buff *BUFF, const u8 *head, u8 head_number, void *data, u8 data_number, u8 size, u8 bigfron);
#endif
