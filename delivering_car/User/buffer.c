#include "buffer.h"
#include "string.h"
#include "stdio.h"
volatile struct Buff U3_buffer, U2_buffer, Soft_Usart, U1_buffer, U4_buffer, U5_buffer, IIC_buff;
/**
 * @description: 初始化环形缓冲区
 * @param {Buff} *BUFF
 * @return {*}
 */
void Iinitial_BUFF(volatile struct Buff *BUFF)
{
    BUFF->head_p = (volatile u8*)BUFF->Data;
    BUFF->end_p = (volatile u8*)&BUFF->Data[BUFFER_SIZE - 1];
    BUFF->write_p = (volatile u8*)BUFF->Data;
    BUFF->read_p = (volatile u8*)BUFF->Data;
    printf("head_p:%x\n\r", BUFF->head_p);
    printf("end_p:%x\n\r", BUFF->end_p);
    printf("write_p:%x\n\r", BUFF->write_p);
    printf("read_p:%x\n\r", BUFF->read_p);
}
/**
 * @description: 把数据写入缓冲区
 * @param {u8} *P
 * @param {Buff} *BUFF
 * @return {*}
 */
void Write_BUFF(u8 *P, volatile struct Buff *BUFF)
{
	
    if (BUFF->write_p == BUFF->end_p)
    {
		
        *BUFF->write_p = *P;
        BUFF->write_p = BUFF->head_p;
		//printf("get data");
    }
    else
    {
        *BUFF->write_p = *P;
        BUFF->write_p++;
		
    }
	if((BUFF->read_p == BUFF->write_p))
	{
		BUFF->read_p++;
		if(BUFF->read_p > BUFF->end_p)
			BUFF->read_p = BUFF->head_p;
		//printf("buff full %x %x\r\n", BUFF->write_p, BUFF->read_p);
	}
}
/**
 * @description: 检查缓冲区是否有数据读
 * @param {Buff} *BUFF 传入需要检测的缓冲区
 * @return {*}
 */
u8 Check_Buffer(volatile struct Buff *BUFF)
{
    if ((BUFF->read_p == BUFF->write_p))
    {
        return 0;
    }
    else
        return 1;
}
/**
 * @description: 从缓冲区读取数据
 * @param {Buff} *BUFF
 * @return {*}
 */
u8 Read_BUFF(volatile struct Buff *BUFF)
{
    volatile u8 *p;
    if (BUFF->read_p == BUFF->write_p)
    {
        // printf("%u", *BUFF->read_p);
        // printf("\n数据为空\n");
        return 0;
    }
    else
    {
        if (BUFF->read_p == BUFF->end_p)
        {

            p = BUFF->read_p;
            // printf("\n读到的数据为:%c\n", *p);
            BUFF->read_p = BUFF->head_p;
            return *p;
        }
        else
        {
            p = BUFF->read_p;
            //  printf("\n读到的数据为1:%c\n", *p);
            BUFF->read_p++;
        }
        return *p;
    }
}
/**
 * @description: 从缓冲区寻找指定字符串
 * @param {Buff} *BUFF
 * @param {char} *p
 * @return {*}
 */
u8 Find_Char(volatile struct Buff *BUFF, char *p)
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
void Read_Data(volatile struct Buff *BUFF, u8 head1, u8 head2, u8 head3, u8 count, u8 *p)
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
u8 Read_buff(volatile struct Buff *BUFF, const u8 *head, u8 head_number, u8 *data, u8 data_number)
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
u8 Read_buff_Void(volatile struct Buff *BUFF, const u8 *head, u8 head_number, void *data, u8 data_number, u8 size, u8 bigfron)
{
    u8 i, n, count = 0;
    n = BUFFER_SIZE;
    //判断环形缓冲区是否有足够数据
    // if (BUFF->write_p > BUFF->read_p)
    // {
    //     if ((BUFF->write_p - BUFF->read_p) > (head_number + data_number * size / 8))
    //         return 0;
    // }
    // else
    // {
    //     if ((BUFF->end_p - BUFF->read_p + 1 + BUFF->write_p - BUFF->head_p) > (head_number + data_number * size / 8))
    //         return 0;
    // }
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
			
			//printf("aa");
			return 1;
		}
    }
    else
    {
        if ((BUFF->end_p - BUFF->read_p + 1 + BUFF->write_p - BUFF->head_p) >= need_len)
		{
			//printf("!!\r\n");
			return 1;
			
		}
		
			//printf("%x \r\n%x \r\n%d \r\n", (BUFF->write_p), (BUFF->read_p), need_len);
    }
	
    return 0;
}
