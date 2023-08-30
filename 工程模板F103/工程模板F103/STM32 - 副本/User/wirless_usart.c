
#include "wirless_usart.h"
#include "buffer.h"
#include "string.h"
#include "Delay.h"
#include "stdarg.h"
#include "stdio.h"
#define EN_Wirless 0
#if EN_Wirless
// const char *SSD = "HONOR";
// const char *Password = "94265426";
// const char *Tcp = "192.168.3.32";
// const char *Port = "1234";
u8 cite = 0;
void ESP8266_Enable()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_13);
}

static void
NVIC_USART3_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void USART3_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
    // 打开串口外设的时钟

    // 将USART Tx的GPIO配置为推挽复用模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // 将USART Rx的GPIO配置为浮空输入模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    NVIC_USART3_Configuration();

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    USART_Cmd(USART3, ENABLE);
}
u8 abn;
void USART3_IRQHandler()
{

    if (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == 1)
    {
        abn = USART_ReceiveData(USART3);
        Write_BUFF(&abn, &U3_buffer);
        printf("%c", abn);
    }
    USART_ClearFlag(USART3, USART_FLAG_RXNE);
}

void U3_printf(char *fmt, ...)
{
    char buffer[50 + 1]; // ????
    u8 i = 0;
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    vsnprintf(buffer, 50 + 1, fmt, arg_ptr);
    while ((i < 50) && (i < strlen(buffer)))
    {
        USART_SendData(USART3, (u8)buffer[i++]);
        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
            ;
    }
    va_end(arg_ptr);
}

u8 ESP_Check(u8 Timeout)
{
    RSR_ESP();
    while (Timeout--)
    {
        Delayms(80);
        if (Find_Char(&U3_buffer, "ready"))
            break;
        else
            NVIC_SystemReset();
        // if (strstr((char *)U3_buffer.Data, ready))
        //     break;
        printf("%d ", Timeout);
    }
    // U3_buffer.read_p = U3_buffer.head_p;
    // memset(U3_buffer.Data, 1, 128);
    printf("\r\n");
    if (Timeout == 0xFF)
    {
        NVIC_SystemReset();
        return 1;
    }
    else
        return 0;
}
/**
 * @description: 检测是否连接函数
 * @param {u8} sign 是否使用透传
 * @param {int} time 超时时间
 * @return {*}
 */
u8 Check_Connect(u8 sign,int time)
{
    static int ping = 0;
    u8 i,p;

    if (sign)
    {
        for (i = 0; i < 10; i++)
        {
            p = 3;

            while (p--)
            {
                if (Find_Char(&U3_buffer, "Hello"))
                {
                    Delayms(50);
                    U3_printf("Hi");
                    return 1;
                }else
                {
                    ping++;
                    printf("%d\r\n", ping);
                }
                

                Delayms(time);
            }
        }
        if (!cite)
        {
            NVIC_SystemReset();//系统复位
        }
        else RSR_ESP();//ESP复位
    }
    else
    {
        p = 3;
        while (p--)
        {
            for (i = 0; i < 5; i++)
            {
                

                U3_printf("AT+CIPSEND=%d,5\r\n", i);
                Delayms(50);
                U3_printf("Hello");
                Delayms(200);
                if (Find_Char(&U3_buffer, "Hi"))
                    return 1;
            }
        }
        if (!cite)
        {
            NVIC_SystemReset();
        }
        else
       {
            //ESP复位
            RSR_ESP();
            ESP_Check(100);
            U3_printf("AT+CIPMUX=1\r\n");
            ESP_Check_OK(50);
            U3_printf("AT+CIPSERVER=1,80\r\n");
            ESP_Check_OK(50);
        }
    }
}

u8 ESP_Check_OK(u8 Timeout)
{
    while (Timeout--)
    {
        Delayms(100);
			if (Find_Char(&U3_buffer, "OK"))
            break;
        else
            NVIC_SystemReset();
//        if (strstr((char *)U3_buffer.Data, "OK"))
//            break;
        printf("%d ", Timeout);
    }
//    U3_buffer.read_p = U3_buffer.head_p;
//    memset(U3_buffer.Data, 1, 128);
    printf("\r\n");
    if (Timeout == 0xFF)
    {
        NVIC_SystemReset();
        return 1;
    }
    else
        return 0;
}
/**
 * @description: 
 * @return {*}
 */

void Send_ESP_CMD()
{
    //	printf("AT+RST\r\n");
    //    ESP_Check(100);
    // GPIO_ResetBits(GPIOA, GPIO_Pin_5);
    // Delayms(200);
    // GPIO_SetBits(GPIOA, GPIO_Pin_5);
    // Delayms(1000);
    // printf("AT+CWMODE=3\r\n");
    // ESP_Check_OK(100);
    // printf("AT+CWJAP=\"%s\",\"%s\"\r\n", SSD, Password);
    // ESP_Check_OK(200);
    // Delayms(100);
    // printf("AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", Tcp, Port);
    // ESP_Check_OK(200);
    // Delayms(100);
    // printf("AT+CIPMODE=1\r\n");
    // ESP_Check_OK(100);
    // Delayms(100);
    // printf("AT+CIPSEND\r\n");
    // ESP_Check_OK(100);
}
/**
 * @description: 
 * @return {*}
 */
void Wriless_Serial_port_Initial(u8 a)
{
    ESP8266_Enable();
    USART3_Config();
    Iinitial_BUFF(&U3_buffer);
    // Send_ESP_CMD();
    ESP_Check(100);
    if (!a)
    {
        U3_printf("AT+CIPMUX=1\r\n");
        ESP_Check_OK(50);
        U3_printf("AT+CIPSERVER=1,80\r\n");
        ESP_Check_OK(50);
    }
    
   
}
/**
 * @description: ESPchong
 * @return {*}
 */
void RSR_ESP()
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_13);
    Delayms(300);
    GPIO_SetBits(GPIOB, GPIO_Pin_13);
    Delayms(1000);
}
/**
 * @description: 初始化函数
 * @param {u8} a 是否透传 1为透传 0为不透传
 * @param {int} time 等待时间，当到达等待时间还没连接上，会复位
 * @return {*}
 */
void ESP_Init(u8 a,int time)
{
    
    Wriless_Serial_port_Initial(a);
    Check_Connect(a, time);
    cite = 1;
}
#endif
