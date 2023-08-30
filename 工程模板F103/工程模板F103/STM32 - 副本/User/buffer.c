#include "buffer.h"
#include "string.h"
#include "stdio.h"
struct Buff U3_buffer, U2_buffer, Soft_Usart, U1_buffer, U4_buffer, U5_buffer, U1_buffer,IIC_buff;
/**
 * @description: ��ʼ�����λ�����
 * @param {Buff} *BUFF
 * @return {*}
 */
void Iinitial_BUFF(struct Buff *BUFF)
{
    BUFF->head_p = BUFF->Data;
    BUFF->end_p = &BUFF->Data[BUFFER_SIZE - 1];
    BUFF->write_p = BUFF->Data;
    BUFF->read_p = BUFF->Data;
    BUFF->max = 0;
    printf("%x\n", BUFF->head_p);
    printf("%x\n", BUFF->end_p);
    printf("%x\n", BUFF->write_p);
    printf("%x\n", BUFF->read_p);
}
/**
 * @description: ������д�뻺����
 * @param {u8} *P
 * @param {Buff} *BUFF
 * @return {*}
 */
void Write_BUFF(u8 *P, struct Buff *BUFF)
{
    if (BUFF->write_p == BUFF->end_p)
    {
        BUFF->max = 1;
        *BUFF->write_p = *P;
        BUFF->write_p = BUFF->head_p;
    }
    else
    {
        *BUFF->write_p = *P;
        BUFF->write_p++;
        if (BUFF->max == 1)
        {
            BUFF->read_p = BUFF->write_p;
        }
    }
}
/**
 * @description: ��黺�����Ƿ������ݶ�
 * @param {Buff} *BUFF ������Ҫ���Ļ�����
 * @return {*}
 */
u8 Check_Buffer(struct Buff *BUFF)
{
    if ((BUFF->read_p == BUFF->write_p) && BUFF->max == 0)
    {
        return 0;
    }
    else
        return 1;
}
/**
 * @description: �ӻ�������ȡ����
 * @param {Buff} *BUFF
 * @return {*}
 */
u8 Read_BUFF(struct Buff *BUFF)
{
    u8 *p;
    if (BUFF->read_p == BUFF->write_p && BUFF->max == 0)
    {
        // printf("%u", *BUFF->read_p);
        // printf("\n����Ϊ��\n");
        return 0;
    }
    else
    {
        if (BUFF->read_p == BUFF->end_p)
        {

            p = BUFF->read_p;
            BUFF->max = 0;
            // printf("\n����������Ϊ:%c\n", *p);
            BUFF->read_p = BUFF->head_p;
            return *p;
        }
        else
        {
            p = BUFF->read_p;
            //  printf("\n����������Ϊ1:%c\n", *p);
            BUFF->read_p++;
        }
        return *p;
    }
}
/**
 * @description: �ӻ�����Ѱ��ָ���ַ���
 * @param {Buff} *BUFF
 * @param {char} *p
 * @return {*}
 */
u8 Find_Char(struct Buff *BUFF, char *p)
{
    u8 i, n, count = 0;
    n = BUFFER_SIZE;

    for (i = 0; i < strlen(p); i++)
    {
        while (n--)
        {
            if (((u8)Read_BUFF(BUFF) == (u8)p[i]))
            {
                count++;
                break;
            }
            else
                i = 0, count = 0;
            if (n == 0)
                return 0;
        }
    }
    if (count == strlen(p))
    {
        return 1;
    }
    else
        return 0;
}
// void Analysis_Contract(struct Buff *BUFF,char *Contract,u8 *data)
//{
//     if (Check_Buffer(BUFF))
//     {
//         /* code */
//
//
//     u8 i,n,count;
//     n = BUFFER_SIZE;
//		count = 	strlen(Contract);
//     for (i = 0; i <count; i++)
//     {
//         while (n--)
//	    {
//         if (!((u8)Read_BUFF(BUFF)== (u8)Contract[i]))break;
//	    }
//     }

//    for ( i = 0; i < sizeof(data); i++)
//    {
//       data[i] = (u8)Read_BUFF(BUFF);
//			printf("%c",data[i]);
//    }
//    }
//}

// OPENMV 0xFF
void Read_Data(struct Buff *BUFF, u8 head1, u8 head2, u8 head3, u8 count, u8 *p)
{
    u8 i, A;
    A = 128;
    while (A--)
    {
        if (Read_BUFF(BUFF) == head1)
        {
            if (Read_BUFF(BUFF) == head2)
            {
                if (Read_BUFF(BUFF) == head3)
                {
                    for (i = 0; i < count; i++)
                    {
                        *p = Read_BUFF(BUFF);
                        p++;
                    }
                    A = 0;
                }
            }
        }
    }
}
u8 Read_buff(struct Buff *BUFF, const u8 *head, u8 head_number, u8 *data, u8 data_number)
{
    u8 i, n, count = 0;
    n = BUFFER_SIZE;

    for (i = 0; i < head_number; i++)
    {
        while (n--)
        {
            if (((u8)Read_BUFF(BUFF) == (u8)head[i]))
            {
                count++;
                break;
            }
            else
                i = 0, count = 0;
            if (n == 0)
                return 0;
        }
    }
    if (count == head_number)
    {
        for (i = 0; i < data_number; i++)
        {
            *data = (u8)Read_BUFF(BUFF);
            data++;
        }
        return 1;
    }
    else
        return 0;
}
/**
 * @description: ��ȡ��ͬ��������   
 * @param {Buff} *BUFF ������
 * @param {u8} *head Э��ͷ
 * @param {u8} head_number Э��ͷ����
 * @param {void} *data ��������ָ��
 * @param {u8} data_number �������ݵĸ���
 * @param {u8} size �������ݵ����� ��u8 u16 u32
 * @param {u8} bigfron ��λ��ǰ��1����λ��ǰ��0
 * @return {*} ����0��ȡʧ�ܣ�1�ɹ�
 */
u8 Read_buff_Void(struct Buff *BUFF, const u8 *head, u8 head_number, void *data, u8 data_number, u8 size, u8 bigfron)
{
    u8 i, n, count = 0;
    n = BUFFER_SIZE;
    if (size == 8)
    {
        for (i = 0; i < head_number; i++)
        {
            while (n--)
            {
                if (((u8)Read_BUFF(BUFF) == (u8)head[i]))
                {
                    count++;
                    break;
                }
                else
                    i = 0, count = 0;
                if (n == 0)
                    return 0;
            }
        }
        if (count == head_number)
        {
            for (i = 0; i < data_number; i++)
            {
                *(u8 *)data = (u8)Read_BUFF(BUFF);
                ((u8 *)data)++;
            }
            return 1;
        }
        else
            return 0;
    }
    else if (size == 16)
    {
        for (i = 0; i < head_number; i++)
        {
            while (n--)
            {
                if (((u8)Read_BUFF(BUFF) == (u8)head[i]))
                {
                    count++;
                    break;
                }
                else
                    i = 0, count = 0;
                if (n == 0)
                    return 0;
            }
        }
        if (count == head_number)
        {
            u16 big;
            for (i = 0; i < data_number; i++)
            {
                if (bigfron == 1)
                {
                    (*(u16 *)data) = (u8)Read_BUFF(BUFF);
                    (*(u16 *)data) = ((*(u16 *)data) << 8) + (u8)Read_BUFF(BUFF);
                    ((u16 *)data)++;
                }
                else
                {

                    (*(u16 *)data) = (u8)Read_BUFF(BUFF);
                    big = (u8)Read_BUFF(BUFF);
                    (*(u16 *)data) = (big << 8) + (*(u16 *)data);
                    ((u16 *)data)++;
                }
            }
            return 1;
        }
		else
            return 0;
    }
    else if (size == 32)
    {
        for (i = 0; i < head_number; i++)
        {
            while (n--)
            {
                if (((u8)Read_BUFF(BUFF) == (u8)head[i]))
                {
                    count++;
                    break;
                }
                else
                    i = 0, count = 0;
                if (n == 0)
                    return 0;
            }
        }
        if (count == head_number)
        {
            u32 one, two, three, four;
            for (i = 0; i < data_number; i++)
            {
                if (bigfron == 1)
                {
                    one = (u8)Read_BUFF(BUFF);
                    two = (u8)Read_BUFF(BUFF);
                    three = (u8)Read_BUFF(BUFF);
                    four = (u8)Read_BUFF(BUFF);
                    (*(u32 *)data) = four + (three << 8) + (two << 16) + (one << 24);
                    ((u32 *)data)++;
                }
                else
                {

                    one = (u8)Read_BUFF(BUFF);
                    two = (u8)Read_BUFF(BUFF);
                    three = (u8)Read_BUFF(BUFF);
                    four = (u8)Read_BUFF(BUFF);
                    (*(u32 *)data) = one + (two << 8) ; 
                    (*(u32 *)data) +=(three << 16) + (four << 24);
                    ((u32 *)data)++;
                }
            }
            return 1;
        }
				return 0;
    }
	else
		return 0;
}

