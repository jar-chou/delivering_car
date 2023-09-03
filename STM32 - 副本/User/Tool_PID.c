
#include "Tool_PID.h"
#include "PID.h"
#include <string.h>

struct prot_frame_parser_t
{
  uint8_t *recv_ptr;
  uint16_t r_oft;
  uint16_t w_oft;
  uint16_t frame_len;
  uint16_t found_frame_head;
};
union pid_send
{
  char c[4];
  float f;
} y;
static struct prot_frame_parser_t parser;

static uint8_t recv_buf[PROT_FRAME_LEN_RECV];

/**
 * @brief 计算校验和
 * @param ptr：需要计算的数据
 * @param len：需要计算的长度
 * @retval 校验和
 */
uint8_t check_sum(uint8_t init, uint8_t *ptr, uint8_t len)
{
  uint8_t sum = init;

  while (len--)
  {
    sum += *ptr;
    ptr++;
  }

  return sum;
}

/**
 * @brief   得到帧类型（帧命令）
 * @param   *frame:  数据帧
 * @param   head_oft: 帧头的偏移位置
 * @return  帧长度.
 */
static uint8_t get_frame_type(uint8_t *frame, uint16_t head_oft)
{
  return (frame[(head_oft + CMD_INDEX_VAL) % PROT_FRAME_LEN_RECV] & 0xFF);
}

/**
 * @brief   得到帧长度
 * @param   *buf:  数据缓冲区.
 * @param   head_oft: 帧头的偏移位置
 * @return  帧长度.
 */
static uint16_t get_frame_len(uint8_t *frame, uint16_t head_oft)
{
  return ((frame[(head_oft + LEN_INDEX_VAL + 0) % PROT_FRAME_LEN_RECV] << 0) |
          (frame[(head_oft + LEN_INDEX_VAL + 1) % PROT_FRAME_LEN_RECV] << 8) |
          (frame[(head_oft + LEN_INDEX_VAL + 2) % PROT_FRAME_LEN_RECV] << 16) |
          (frame[(head_oft + LEN_INDEX_VAL + 3) % PROT_FRAME_LEN_RECV] << 24)); // 合成帧长度
}

/**
 * @brief   获取 crc-16 校验值
 * @param   *frame:  数据缓冲区.
 * @param   head_oft: 帧头的偏移位置
 * @param   head_oft: 帧长
 * @return  帧长度.
 */
static uint8_t get_frame_checksum(uint8_t *frame, uint16_t head_oft, uint16_t frame_len)
{
  return (frame[(head_oft + frame_len - 1) % PROT_FRAME_LEN_RECV]);
}

/**
 * @brief   查找帧头
 * @param   *buf:  数据缓冲区.
 * @param   ring_buf_len: 缓冲区大小
 * @param   start: 起始位置
 * @param   len: 需要查找的长度
 * @return  -1：没有找到帧头，其他值：帧头的位置.
 */
static int32_t recvbuf_find_header(uint8_t *buf, uint16_t ring_buf_len, uint16_t start, uint16_t len)
{
  uint16_t i = 0;

  for (i = 0; i < (len - 3); i++)
  {
    if (((buf[(start + i + 0) % ring_buf_len] << 0) |
         (buf[(start + i + 1) % ring_buf_len] << 8) |
         (buf[(start + i + 2) % ring_buf_len] << 16) |
         (buf[(start + i + 3) % ring_buf_len] << 24)) == FRAME_HEADER)
    {
      return ((start + i) % ring_buf_len);
    }
  }
  return -1;
}

/**
 * @brief   计算为解析的数据长度
 * @param   *buf:  数据缓冲区.
 * @param   ring_buf_len: 缓冲区大小
 * @param   start: 起始位置
 * @param   end: 结束位置
 * @return  为解析的数据长度
 */
static int32_t recvbuf_get_len_to_parse(uint16_t frame_len, uint16_t ring_buf_len, uint16_t start, uint16_t end)
{
  uint16_t unparsed_data_len = 0;

  if (start <= end)
    unparsed_data_len = end - start;
  else
    unparsed_data_len = ring_buf_len - start + end;

  if (frame_len > unparsed_data_len)
    return 0;
  else
    return unparsed_data_len;
}

/**
 * @brief   接收数据写入缓冲区
 * @param   *buf:  数据缓冲区.
 * @param   ring_buf_len: 缓冲区大小
 * @param   w_oft: 写偏移
 * @param   *data: 需要写入的数据
 * @param   *data_len: 需要写入数据的长度
 * @return  void.
 */
static void recvbuf_put_data(uint8_t *buf, uint16_t ring_buf_len, uint16_t w_oft,
                             uint8_t *data, uint16_t data_len)
{
  if ((w_oft + data_len) > ring_buf_len) // 超过缓冲区尾
  {
    uint16_t data_len_part = ring_buf_len - w_oft; // 缓冲区剩余长度

    /* 数据分两段写入缓冲区*/
    memcpy(buf + w_oft, data, data_len_part);                    // 写入缓冲区尾
    memcpy(buf, data + data_len_part, data_len - data_len_part); // 写入缓冲区头
  }
  else
    memcpy(buf + w_oft, data, data_len); // 数据写入缓冲区
}

/**
 * @brief   查询帧类型（命令）
 * @param   *data:  帧数据
 * @param   data_len: 帧数据的大小
 * @return  帧类型（命令）.
 */
static uint8_t protocol_frame_parse(uint8_t *data, uint16_t *data_len)
{
  uint8_t frame_type = CMD_NONE;
  uint16_t need_to_parse_len = 0;
  int16_t header_oft = -1;
  uint8_t checksum = 0;

  need_to_parse_len = recvbuf_get_len_to_parse(parser.frame_len, PROT_FRAME_LEN_RECV, parser.r_oft, parser.w_oft); // 得到为解析的数据长度
  if (need_to_parse_len < 9)                                                                                       // 肯定还不能同时找到帧头和帧长度
    return frame_type;

  /* 还未找到帧头，需要进行查找*/
  if (0 == parser.found_frame_head)
  {
    /* 同步头为四字节，可能存在未解析的数据中最后一个字节刚好为同步头第一个字节的情况，
       因此查找同步头时，最后一个字节将不解析，也不会被丢弃*/
    header_oft = recvbuf_find_header(parser.recv_ptr, PROT_FRAME_LEN_RECV, parser.r_oft, need_to_parse_len);
    if (0 <= header_oft)
    {
      /* 已找到帧头*/
      parser.found_frame_head = 1;
      parser.r_oft = header_oft;

      /* 确认是否可以计算帧长*/
      if (recvbuf_get_len_to_parse(parser.frame_len, PROT_FRAME_LEN_RECV,
                                   parser.r_oft, parser.w_oft) < 9)
        return frame_type;
    }
    else
    {
      /* 未解析的数据中依然未找到帧头，丢掉此次解析过的所有数据*/
      parser.r_oft = ((parser.r_oft + need_to_parse_len - 3) % PROT_FRAME_LEN_RECV);
      return frame_type;
    }
  }

  /* 计算帧长，并确定是否可以进行数据解析*/
  if (0 == parser.frame_len)
  {
    parser.frame_len = get_frame_len(parser.recv_ptr, parser.r_oft);
    if (need_to_parse_len < parser.frame_len)
      return frame_type;
  }

  /* 帧头位置确认，且未解析的数据超过帧长，可以计算校验和*/
  if ((parser.frame_len + parser.r_oft - PROT_FRAME_LEN_CHECKSUM) > PROT_FRAME_LEN_RECV)
  {
    /* 数据帧被分为两部分，一部分在缓冲区尾，一部分在缓冲区头 */
    checksum = check_sum(checksum, parser.recv_ptr + parser.r_oft,
                         PROT_FRAME_LEN_RECV - parser.r_oft);
    checksum = check_sum(checksum, parser.recv_ptr, parser.frame_len - PROT_FRAME_LEN_CHECKSUM + parser.r_oft - PROT_FRAME_LEN_RECV);
  }
  else
  {
    /* 数据帧可以一次性取完*/
    checksum = check_sum(checksum, parser.recv_ptr + parser.r_oft, parser.frame_len - PROT_FRAME_LEN_CHECKSUM);
  }

  if (checksum == get_frame_checksum(parser.recv_ptr, parser.r_oft, parser.frame_len))
  {
    /* 校验成功，拷贝整帧数据 */
    if ((parser.r_oft + parser.frame_len) > PROT_FRAME_LEN_RECV)
    {
      /* 数据帧被分为两部分，一部分在缓冲区尾，一部分在缓冲区头*/
      uint16_t data_len_part = PROT_FRAME_LEN_RECV - parser.r_oft;
      memcpy(data, parser.recv_ptr + parser.r_oft, data_len_part);
      memcpy(data + data_len_part, parser.recv_ptr, parser.frame_len - data_len_part);
    }
    else
    {
      /* 数据帧可以一次性取完*/
      memcpy(data, parser.recv_ptr + parser.r_oft, parser.frame_len);
    }
    *data_len = parser.frame_len;
    frame_type = get_frame_type(parser.recv_ptr, parser.r_oft);

    /* 丢弃缓冲区中的命令帧*/
    parser.r_oft = (parser.r_oft + parser.frame_len) % PROT_FRAME_LEN_RECV;
  }
  else
  {
    /* 校验错误，说明之前找到的帧头只是偶然出现的废数据*/
    parser.r_oft = (parser.r_oft + 1) % PROT_FRAME_LEN_RECV;
  }
  parser.frame_len = 0;
  parser.found_frame_head = 0;

  return frame_type;
}

/**
 * @brief   接收数据处理
 * @param   *data:  要计算的数据的数组.
 * @param   data_len: 数据的大小
 * @return  void.
 */
void protocol_data_recv(uint8_t *data, uint16_t data_len)
{
  recvbuf_put_data(parser.recv_ptr, PROT_FRAME_LEN_RECV, parser.w_oft, data, data_len); // 接收数据
  parser.w_oft = (parser.w_oft + data_len) % PROT_FRAME_LEN_RECV;                       // 计算写偏移
}

/**
 * @brief   初始化接收协议
 * @param   void
 * @return  初始化结果.
 */
int32_t protocol_init(void)
{

  memset(&parser, 0, sizeof(struct prot_frame_parser_t));

  /* 初始化分配数据接收与解析缓冲区*/
  parser.recv_ptr = recv_buf;

  return 0;
}
extern struct PID extral_X, inner_X,inner_Y,extral_Y;
union pid__
{
  u8 a[4];
  float PID;
}pid___;

/**
 * @brief   接收的数据处理
 * @param   void
 * @return  -1：没有找到一个正确的命令.
 */
int8_t receiving_process(void)
{

  u8 i;
  uint8_t frame_data[128];     // 要能放下最长的帧
  uint16_t frame_len = 0;      // 帧长度
  uint8_t cmd_type = CMD_NONE; // 命令类型
  float p_temp, i_temp, d_temp;
  while (1)
  {
    cmd_type = protocol_frame_parse(frame_data, &frame_len);
    switch (cmd_type)
    {
    case CMD_NONE:
    {
      return -1;
    }

    case SET_P_I_D_CMD:
    {
      if (frame_data[4] == 0x01)
      {
        for ( i = 0; i < 4; i++)
        {
          pid___.a[i] = frame_data[i+10];
        }
				p_temp = pid___.PID ;
				for ( i = 0; i < 4; i++)
        {
          pid___.a[i] = frame_data[i+14];
        }
				i_temp = pid___.PID;
				for ( i = 0; i < 4; i++)
        {
          pid___.a[i] = frame_data[i+18];
        }
      d_temp = pid___.PID;


  
       float b[3] = {p_temp, i_temp, d_temp};
       Angle_Speed_PID_X.KP = b[0]; // 替换成你们的PID
       Angle_Speed_PID_X.KI = b[1]; // 替换成你们的PID
       Angle_Speed_PID_X.KD = b[2]; // 替换成你们的PID

       vTaskDelay(10);
       set_computer_value(SEND_P_I_D_CMD, CURVES_CH1, b, 3);
      }
      if (frame_data[4] == 0x02)
      {
        for ( i = 0; i < 4; i++)
        {
          pid___.a[i] = frame_data[i+10];
        }
				p_temp = pid___.PID ;
				for ( i = 0; i < 4; i++)
        {
          pid___.a[i] = frame_data[i+14];
        }
				i_temp = pid___.PID;
				for ( i = 0; i < 4; i++)
        {
          pid___.a[i] = frame_data[i+18];
        }
      d_temp = pid___.PID;


        float b[3] = {p_temp, i_temp, d_temp};
        // inner_X.KP = b[0];//替换成你们的PID
        // inner_X.KI = b[1];//替换成你们的PID
        // inner_X.KD = b[2];//替换成你们的PID
         vTaskDelay(10);
        set_computer_value(SEND_P_I_D_CMD, CURVES_CH2, b, 3);
      }
        }
    break;

    case SET_TARGET_CMD:
    {
      int actual_temp = COMPOUND_32BIT(&frame_data[13]); // 得到数据
      //! 设置目标值
    }
    break;

    case START_CMD:
    {
      //!启动电机
    }
    break;

    case STOP_CMD:
    {
      //!停止电机
    }
    break;

    case RESET_CMD:
    {
      //!复位系统
    }
    break;

    case SET_PERIOD_CMD:
    {
      uint32_t temp = COMPOUND_32BIT(&frame_data[13]); // 周期数
      //!设置定时器周期1~1000ms
    }
    break;

    default:
      return -1;
    }
  }
}

//!   指令           参数                        功能
//?   0x01    1个，目标值，int类型            设置上位机通道的目标值
//?   0x02    1个，实际值，int类型            设置上位机通道实际值
//?   0x03    3个，P、I、D，float类型         设置上位机PID值
//?   0x04           无                      设置上位机启动指令（同步上位机的按钮状态）set_computer_value(SEND_STOP_CMD, CURVES_CH1, NULL, 0)
//?   0x05           无                      设置上位机停止指令（同步上位机的按钮状态）set_computer_value(SEND_START_CMD, CURVES_CH1, NULL, 0)
//?   0x06    1个，目标值，unsigned int类型   设置上位机周期

//*@note：使用0x03命令时要把PID3个值打包成float数组形式，然后传入数组指针
//*float pid_temp[3] = {22, 33, 44};

/*
way：1.先初始化protocol_init()
way：2.接下来就可以通讯了，使用set_computer_value()函数
*/
//!注：要配合串口中断函数使用
//!如果使用配套串口文件就可以不改
//!如没用，添加串口1中断函数
/*
*uint8_t dr;
*   void USART1_IRQHandler()
*   {
*     if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == 1)
*     {
*       dr = USART_ReceiveData(USART1);
*       protocol_data_recv(&dr, 1);
*     }
*      USART_ClearFlag(USART1, USART_FLAG_RXNE);
*}
!注意要把“*”去掉
*/

/**
 * @brief 设置上位机的值
 * @param cmd：命令
 * @param ch: 曲线通道
 * @param data：参数指针
 * @param num：参数个数
 * @retval 无
 */
void set_computer_value(uint8_t cmd, uint8_t ch, void *data, uint8_t num)
{
  uint8_t sum, i = 0; // 校验和
  num *= 4;           // 一个参数 4 个字节
  // void *data = &data;
  static packet_head_t set_packet;

  set_packet.head = FRAME_HEADER;                                 // 包头 0x59485A53
  set_packet.len = 0x0B + num;                                    // 包长
  set_packet.ch = ch;                                             // 设置通道
  set_packet.cmd = cmd;                                           // 设置命令
  sum = check_sum(0, (uint8_t *)&set_packet, sizeof(set_packet)); // 计算包头校验和
  sum = check_sum(sum, (uint8_t *)data, num);                     // 计算参数校验和

  for (i = 0; i < 4; i++)
  {
    Usart_SendByte(USART1, (uint8_t)(set_packet.head >> i * 8) & 0xFF);
  }
  Usart_SendByte(USART1, set_packet.ch);
  for (i = 0; i < 4; i++)
  {
    Usart_SendByte(USART1, (uint8_t)(set_packet.len >> i * 8) & 0xFF);
  }
  Usart_SendByte(USART1, cmd);
  if (num == 4)
  {
    for (i = 0; i < 4; i++)
    {
      Usart_SendByte(USART1, (uint8_t)((*(int *)data >> i * 8) & 0xFF));
    }
  }
  if (num == 12)
  {
    u8 n;
    for (n = 0; n < 3; n++)
    {
      y.f = *((float *)data + n);
      for (i = 0; i < 4; i++)
      {
        Usart_SendByte(USART1, y.c[i]);
      }
    }
  }
  Usart_SendByte(USART1, sum);
}

/**********************************************************************************************/
