/*
 * @Date: 2023-10-16 11:45:55
 * @LastEditors: zjc
 * @LastEditTime: 2023-10-17 11:44:39
 * @FilePath: \RVMDK（uv5）f:\桌面\F407_test\User\buffer.c
 * @Verson:V0.2
 * V0.2增加每个buffer可以指定大小
 */
#include "buffer.h"
#include "string.h"
#include "stdio.h"

// struct Buff U3_buffer, U2_buffer, Soft_Usart, U1_buffer, U4_buffer, U5_buffer,IIC_buff;
/**
 * @description: 初始化环形缓冲区
 * @param {Buff} *BUFF
 * @return {*}
 */
void Iinitial_BUFF(struct Buff *BUFF, u8 size_)
{
    BUFF->head_p = BUFF->Data;
    BUFF->end_p = &BUFF->Data[size_ - 1];
    BUFF->write_p = BUFF->Data;
    BUFF->read_p = BUFF->Data;
    BUFF->max = 0;
}
/**
 * @description: 把数据写入缓冲区
 * @param {u8} *P
 * @param {Buff} *BUFF
 * @return {*}
 */
void Write_BUFF(u8 *P, struct Buff *BUFF)
{
    if (BUFF->write_p == BUFF->end_p)
    {

        *BUFF->write_p = *P;
        BUFF->write_p = BUFF->head_p;
        // printf("get data");
    }
    else
    {
        *BUFF->write_p = *P;
        BUFF->write_p++;
    }
    if ((BUFF->read_p) == (BUFF->write_p))
    {
        BUFF->max = 1;
        // BUFF->read_p++;
        // if (BUFF->read_p > BUFF->end_p)
        //     BUFF->read_p = BUFF->head_p;
        // printf("buff full %x %x\r\n", BUFF->write_p, BUFF->read_p);
    }
    if (BUFF->max == 1)
        BUFF->read_p = BUFF->write_p;
}

void Write_BUFF_P(u8 num, struct Buff *BUFF)
{
    // BUFF->write_p = BUFF->head_p + num;
    u8 *eventually_write_p = BUFF->head_p + num;
    // case one: the write pointer is over the end of the buffer
    if (eventually_write_p > BUFF->end_p)
    {
        eventually_write_p = BUFF->head_p + (eventually_write_p - BUFF->end_p) - 1;
        // check whether the read pointer is between the write pointer and the eventually write pointer
        if (BUFF->read_p < eventually_write_p || BUFF->read_p >= BUFF->write_p)
        {
            // if so, move the read pointer to the eventually write pointer
            BUFF->read_p = eventually_write_p + 1;
        }
    }
    else
    {
        // case two: the write pointer is not over the end of the buffer
        // check whether the read pointer is between the write pointer and the eventually write pointer
        if (BUFF->read_p < eventually_write_p && BUFF->read_p >= BUFF->write_p)
        {
            // if so, move the read pointer to the eventually write pointer
            BUFF->read_p = eventually_write_p + 1;
        }
    }
    BUFF->write_p = eventually_write_p; // move the write pointer to the eventually write pointer
}

/**
 * @description: 检查缓冲区是否有数据读
 * @param {Buff} *BUFF 传入需要检测的缓冲区
 * @return {*}
 */
u8 Check_Buffer(struct Buff *BUFF)
{
    if (((BUFF->read_p) == (BUFF->write_p)) && (BUFF->max == 0))
        return 0;
    else
        return 1;
}
/**
 * @description: 从缓冲区读取数据
 * @param {Buff} *BUFF
 * @return {*}
 */
u8 Read_BUFF(struct Buff *BUFF)
{
    volatile u8 *p;
    if ((BUFF->read_p) == (BUFF->write_p))
    {
        if(BUFF->max == 0)
            return 0;
        else
        {
            BUFF->max = 0;
            p = BUFF->read_p;
            BUFF->read_p++;
            return *p;
        }
        // printf("%u", *BUFF->read_p);
        // printf("\n数据为空\n");
    }
    else
    {
        p = BUFF->read_p;
        if (BUFF->read_p == BUFF->end_p)
        {
            // printf("\n读到的数据为:%c\n", *p);
            BUFF->read_p = BUFF->head_p;
            return *p;
        }
        else
        {
            //  printf("\n读到的数据为1:%c\n", *p);
            BUFF->read_p++;
            return *p;
        }
    }
}
/**
 * @description: 从缓冲区寻找指定字符串
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
/**
 * @description: 读取不同类型数据
 * @param {Buff} *BUFF 缓冲区
 * @param {u8} *head 协议头
 * @param {u8} head_number 协议头个数
 * @param {void} *data 接受数据指针
 * @param {u8} data_number 接受数据的个数
 * @param {u8} size 接受数据的类型 如u8 u16 u32
 * @param {u8} bigfron 高位在前则1，低位在前则0
 * @return {*} 返回0读取失败，1成功
 */
u8 Read_buff_Void(struct Buff *BUFF, const u8 *head, u8 head_number, void *data, u8 data_number, u8 size, u8 bigfron)
{
    u8 i, n, count = 0;
    n = BUFFER_SIZE;
    if (!Check_Buffer(BUFF))
        return 0;
    if (size == 8)
    {
        u8 *p = data;
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
                *(u8 *)p = (u8)Read_BUFF(BUFF);
                p++;
                // ((u8 *)data)++;
            }
            return 1;
        }
        else
            return 0;
    }
    else if (size == 16)
    {
        u16 *p = data;
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
                    (*(u16 *)p) = (u8)Read_BUFF(BUFF);
                    (*(u16 *)p) = ((*(u16 *)p) << 8) + (u8)Read_BUFF(BUFF);
                    p++;
                    // ((u16 *)data)++;
                }
                else
                {

                    (*(u16 *)p) = (u8)Read_BUFF(BUFF);
                    big = (u8)Read_BUFF(BUFF);
                    (*(u16 *)p) = (big << 8) + (*(u16 *)p);
                    p++;
                    // ((u16 *)data)++;
                }
            }
            return 1;
        }
        else
            return 0;
    }
    else if (size == 32)
    {
        u32 *p = data;
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
                    (*(u32 *)p) = four + (three << 8) + (two << 16) + (one << 24);
                    p++;
                    // ((uint32_t *)data)++;
                }
                else
                {
                    one = (u8)Read_BUFF(BUFF);
                    two = (u8)Read_BUFF(BUFF);
                    three = (u8)Read_BUFF(BUFF);
                    four = (u8)Read_BUFF(BUFF);
                    (*(u32 *)p) = one + (two << 8) + (three << 16) + (four << 24);
                    p++;
                }
            }
            return 1;
        }
        else
            return 0;
    }
    else
        return 0;
}

/**
 * @description: 判断环形缓冲区是否有足够数据
 * @param {Buff} *BUFF
 * @param {u8} head_number 协议头个数
 * @param {u8} data_number 数据个数
 * @param {u8} size 数据类型
 * @return {*}
 */
u8 have_enough_data(volatile struct Buff *BUFF, u8 head_number, u8 data_number, u8 size)
{
    int need_len = (head_number + data_number * size / 8);
    if (BUFF->write_p >= BUFF->read_p)
    {
        if ((BUFF->write_p - BUFF->read_p) >= need_len)
        {

            // printf("aa");
            return 1;
        }
    }
    else
    {
        if ((BUFF->end_p - BUFF->read_p + 1 + BUFF->write_p - BUFF->head_p) >= need_len)
        {
            // printf("!!\r\n");
            return 1;
        }

        // printf("%x \r\n%x \r\n%d \r\n", (BUFF->write_p), (BUFF->read_p), need_len);
    }

    return 0;
}
