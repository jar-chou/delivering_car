#include "esp8266.h"
#include "Delay.h"
#include <stdarg.h>
// #include "tcp.h"
#if 0

void ESP()
{
    ESP8266_Init(115200); // 串口
}
struct STRUCT_USART_Fram ESP8266_Fram_Record_Struct = {0}; // 定义了一个数据帧结构体
void ESP8266_Init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(ESP8266_RST_Pin_Periph_Clock | ESP8266_CH_PD_Pin_Periph_Clock, ENABLE);

    GPIO_InitStructure.GPIO_Pin = ESP8266_RST_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ESP8266_RST_Pin_Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ESP8266_CH_PD_Pin;
    GPIO_Init(ESP8266_CH_PD_Pin_Port, &GPIO_InitStructure);
    ESP8266_CH_PD_Pin_SetH;
    ESP8266_RST_Pin_SetH;
    uart2_Init(bound);
    ESP8266_Rst();
    Delayms(1000);
}
// 初始化ESP8266串口
void uart2_Init(u32 bound)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 使能指定端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);          // 初始化GPIO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure); // 初始化GPIO

    // Usart2 NVIC 配置
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // USART3配置
    USART_InitStructure.USART_BaudRate = bound;                                     // 设置串口波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长为8
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 1个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无奇偶校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无流控
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 收发模式
    USART_Init(USART3, &USART_InitStructure);                                       // 配置USART参数

    USART_ITConfig(USART3, USART_IT_RXNE | USART_IT_IDLE, ENABLE); // 配置了接收中断和总线空闲中断

    USART_Cmd(USART3, ENABLE); // 使能USART
}
void USART3_IRQHandler(void)
{
    u8 ucCh;

    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        ucCh = USART_ReceiveData(USART3);
        USART_SendData(USART1, ucCh);
        if (ESP8266_Fram_Record_Struct.InfBit.FramLength < (RX_BUF_MAX_LEN - 1))
        {

            ESP8266_Fram_Record_Struct.Data_RX_BUF[ESP8266_Fram_Record_Struct.InfBit.FramLength++] = ucCh;
        }
    }

    if (USART_GetITStatus(USART3, USART_IT_IDLE) == SET) // 如果总线空闲
    {
        ESP8266_Fram_Record_Struct.InfBit.FramFinishFlag = 1;

        ucCh = USART_ReceiveData(USART3); // 由软件序列清除中断标志位（先读USART_SR,然后读USART_DR）
        USART_SendData(USART1, ucCh);
        // TcpClosedFlag = strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, "CLOSED\r\n") ? 1 : 0;
    }
}

// 对ESP8266模块发送AT指令
//  cmd 待发送的指令
//  ack1,ack2;期待的响应，为NULL表不需响应，两者为或逻辑关系
//  time 等待响应时间
// 返回1发送成功， 0失败
bool ESP8266_Send_AT_Cmd(char *cmd, char *ack1, char *ack2, u32 time)
{
    ESP8266_Fram_Record_Struct.InfBit.FramLength = 0; // 重新接收新的数据包
    ESP8266_USART("%s\r\n", cmd);
    if (ack1 == 0 && ack2 == 0) // 不需要接收数据
    {
        return true;
    }
    Delayms(time); // 延时
    Delayms(1000);
    ESP8266_Fram_Record_Struct.Data_RX_BUF[ESP8266_Fram_Record_Struct.InfBit.FramLength] = '\0';

    printf("%s", ESP8266_Fram_Record_Struct.Data_RX_BUF);
    if (ack1 != 0 && ack2 != 0)
    {
        return ((bool)strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, ack1) ||
                (bool)strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, ack2));
    }
    else if (ack1 != 0) // strstr(s1,s2);检测s2是否为s1的一部分，是返回该位置，否则返回false，它强制转换为bool类型了
        return ((bool)strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, ack1));

    else
        return ((bool)strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, ack2));
}

// 复位重启
void ESP8266_Rst(void)
{
    ESP8266_RST_Pin_SetL;
    Delayms(1000);
    ESP8266_RST_Pin_SetH;
}

// 发送恢复出厂默认设置指令将模块恢复成出厂设置
void ESP8266_AT_Test(void)
{
    char count = 0;
    Delayms(1000);
    while (count < 10)
    {
        if (ESP8266_Send_AT_Cmd("AT+RESTORE", "OK", NULL, 500))
        {
            printf("OK\r\n");
            return;
        }
        ++count;
    }
}

// 选择ESP8266的工作模式
//  enumMode 模式类型
// 成功返回true，失败返回false
bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode)
{
    switch (enumMode)
    {
    case STA:
        return ESP8266_Send_AT_Cmd("AT+CWMODE=1", "OK", "no change", 2500);

    case AP:
        return ESP8266_Send_AT_Cmd("AT+CWMODE=2", "OK", "no change", 2500);

    case STA_AP:
        return ESP8266_Send_AT_Cmd("AT+CWMODE=3", "OK", "no change", 2500);

    default:
        return false;
    }
}

// ESP8266连接外部的WIFI
// pSSID WiFi帐号
// pPassWord WiFi密码
// 设置成功返回true 反之false
bool ESP8266_JoinAP(char *pSSID, char *pPassWord)
{
    char cCmd[120];

    sprintf(cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord);
    return ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 5000);
}

// ESP8266 透传使能
// enumEnUnvarnishTx  是否多连接，bool类型
// 设置成功返回true，反之false
bool ESP8266_Enable_MultipleId(FunctionalState enumEnUnvarnishTx)
{
    char cStr[20];

    sprintf(cStr, "AT+CIPMUX=%d", (enumEnUnvarnishTx ? 1 : 0));

    return ESP8266_Send_AT_Cmd(cStr, "OK", 0, 500);
}

// ESP8266 连接服务器
// enumE  网络类型
// ip ，服务器IP
// ComNum  服务器端口
// id，连接号，确保通信不受外界干扰
// 设置成功返回true，反之fasle
bool ESP8266_Link_Server(ENUM_NetPro_TypeDef enumE, char *ip, char *ComNum, ENUM_ID_NO_TypeDef id)
{
    char cStr[100] = {0}, cCmd[120];

    switch (enumE)
    {
    case enumTCP:
        sprintf(cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum);
        break;

    case enumUDP:
        sprintf(cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum);
        break;

    default:
        break;
    }

    if (id < 5)
        sprintf(cCmd, "AT+CIPSTART=%d,%s", id, cStr);

    else
        sprintf(cCmd, "AT+CIPSTART=%s", cStr);

    return ESP8266_Send_AT_Cmd(cCmd, "OK", "ALREAY CONNECT", 4000);
}

// 透传使能
// 设置成功返回true， 反之false
bool ESP8266_UnvarnishSend(void)
{
    if (!ESP8266_Send_AT_Cmd("AT+CIPMODE=1", "OK", 0, 500))
        return false;

    return ESP8266_Send_AT_Cmd("AT+CIPSEND", "OK", ">", 500);
}

// ESP8266发送字符串
// enumEnUnvarnishTx是否使能透传模式
// pStr字符串
// ulStrLength字符串长度
// ucId 连接号
// 设置成功返回true， 反之false
bool ESP8266_SendString(FunctionalState enumEnUnvarnishTx, char *pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId)
{
    char cStr[20];
    bool bRet = false;

    if (enumEnUnvarnishTx)
    {
        ESP8266_USART("%s", pStr);

        bRet = true;
    }

    else
    {
        if (ucId < 5)
            sprintf(cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2);

        else
            sprintf(cStr, "AT+CIPSEND=%d", ulStrLength + 2);

        ESP8266_Send_AT_Cmd(cStr, "> ", 0, 1000);

        bRet = ESP8266_Send_AT_Cmd(pStr, "SEND OK", 0, 1000);
    }

    return bRet;
}

// ESP8266退出透传模式
void ESP8266_ExitUnvarnishSend(void)
{
    Delayms(1000);
    ESP8266_USART("+++");
    Delayms(500);
}

// ESP8266 检测连接状态
// 返回0：获取状态失败
// 返回2：获得ip
// 返回3：建立连接
// 返回4：失去连接
u8 ESP8266_Get_LinkStatus(void)
{
    if (ESP8266_Send_AT_Cmd("AT+CIPSTATUS", "OK", 0, 500))
    {
        if (strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, "STATUS:2\r\n"))
            return 2;

        else if (strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, "STATUS:3\r\n"))
            return 3;

        else if (strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, "STATUS:4\r\n"))
            return 4;
    }

    return 0;
}

static char *itoa(int value, char *string, int radix)
{
    int i, d;
    int flag = 0;
    char *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */

void USART_printf(USART_TypeDef *USARTx, char *Data, ...)
{
    const char *s;
    int d;
    char buf[16];

    va_list ap;
    va_start(ap, Data);

    while (*Data != 0) // 判断数据是否到达结束符
    {
        if (*Data == 0x5c) //'\'
        {
            switch (*++Data)
            {
            case 'r': // 回车符
                USART_SendData(USARTx, 0x0d);
                Data++;
                break;

            case 'n': // 换行符
                USART_SendData(USARTx, 0x0a);
                Data++;
                break;

            default:
                Data++;
                break;
            }
        }

        else if (*Data == '%')
        {
            switch (*++Data)
            {
            case 's': // 字符串
                s = va_arg(ap, const char *);
                for (; *s; s++)
                {
                    USART_SendData(USARTx, *s);
                    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
                        ;
                }
                Data++;
                break;

            case 'd':
                // 十进制
                d = va_arg(ap, int);
                itoa(d, buf, 10);
                for (s = buf; *s; s++)
                {
                    USART_SendData(USARTx, *s);
                    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
                        ;
                }
                Data++;
                break;
            default:
                Data++;
                break;
            }
        }
        else
            USART_SendData(USARTx, *Data++);
        while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
            ;
    }
}

// 下面为ESP8266MQTT功能指令

/*
 *MQTT配置用户属性
 *LinkID 连接ID,目前只支持0
 *scheme 连接方式，这里选择MQTT over TCP,这里设置为1
 *client_id MQTTclientID 用于标志client身份
 *username 用于登录 MQTT 服务器 的 username
 *password 用于登录 MQTT 服务器 的 password
 *cert_key_ID 证书 ID, 目前支持一套 cert 证书, 参数为 0
 *CA_ID 目前支持一套 CA 证书, 参数为 0
 *path 资源路径，这里设置为""
 *设置成功返回true 反之false
 */
bool ESP8266_MQTTUSERCFG(char *pClient_Id, char *pUserName, char *PassWord)
{
    char cCmd[120];
    sprintf(cCmd, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"", pClient_Id, pUserName, PassWord);
    return ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 500);
}

/*
 *连接指定的MQTT服务器
 *LinkID 连接ID,目前只支持0
 *IP：MQTT服务器上对应的IP地址
 *ComNum MQTT服务器上对应的端口号，一般为1883
 *设置成功返回true 反之false
 */
bool ESP8266_MQTTCONN(char *Ip, int Num)
{
    char cCmd[120];
    sprintf(cCmd, "AT+MQTTCONN=0,\"%s\",%d,0", Ip, Num);
    return ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 500);
}

/*
 *订阅指定连接的 MQTT 主题, 可重复多次订阅不同 topic
 *LinkID 连接ID,目前只支持0
 *Topic 订阅的主题名字，这里设置为Topic
 *Qos值：一般为0，这里设置为1
 *设置成功返回true 反之false
 */
bool ESP8266_MQTTSUB(char *Topic)
{
    char cCmd[120];
    sprintf(cCmd, "AT+MQTTSUB=0,\"%s\",1", Topic);
    return ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 500);
}

/*
 *在LinkID上通过 topic 发布数据 data, 其中 data 为字符串消息
 *LinkID 连接ID,目前只支持0
 *Topic 订阅的主题名字，这里设置为Topic
 *data：字符串信息
 *设置成功返回true 反之false
 */
bool ESP8266_MQTTPUB(char *Topic, char *temp)
{
    char cCmd[120];
    sprintf(cCmd, "AT+MQTTPUB=0,\"%s\",\"%s\",1,0", Topic, temp);
    return ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 1000);
}

/*
 *关闭 MQTT Client 为 LinkID 的连接, 并释放内部占用的资源
 *LinkID 连接ID,目前只支持0
 *Topic 订阅的主题名字，这里设置为Topic
 *data：字符串信息
 *设置成功返回true 反之false
 */
bool ESP8266_MQTTCLEAN(void)
{
    char cCmd[120];
    sprintf(cCmd, "AT+MQTTCLEAN=0");
    return ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 500);
}

// ESP8266发送字符串
// enumEnUnvarnishTx是否使能透传模式
// pStr字符串
// ulStrLength字符串长度
// ucId 连接号
// 设置成功返回true， 反之false
bool MQTT_SendString(char *pTopic, char *temp2)
{

    bool bRet = false;
    ESP8266_MQTTPUB(pTopic, temp2);
    Delayms(1000);
    bRet = true;
    return bRet;
}
#endif
