/*******************************************************************************************
											声 明
	本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现其他

不可估量的BUG，天际智联不负任何责任。请勿商用！

* 程序版本：V1.01
* 程序日期：2018-8-18
* 程序作者：愤怒的小孩
* 版权所有：西安天际智联信息技术有限公司
*******************************************************************************************/
#define EN_NRF24LO1 0
#if EN_NRF24LO1
#include "stm32f10x.h"
#include "NRF24L01.h"
#include "Delay.h"

#define SI24R1AddrMax 50 // NRF最后一个字节地址最大为50

uint8_t SI24R1addr = 0xFF; // 初始化NRF最后一字节地址

uint8_t SI24R1_TX_DATA[TX_PAYLO_WIDTH]; // NRF发送缓冲区
uint8_t SI24R1_RX_DATA[RX_PAYLO_WIDTH]; // NRF接收缓冲区

uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01}; // 发送地址
uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01}; // 接收地址

/*****************************************************************************
 * 函  数：void SPI_GPIO_Init(void)
 * 功  能：配置SI24R1的 SCK、MISO、MOSI引脚，以及SPI1初始化
 * 参  数：无
 * 返回值：无
 * 备  注：调试SPI通信时一定要分清主机从机模式
 *         主机从机模式的 空闲状态 电平
 *		  2.4G模块通信时，SPI速率一般不大于10Mbps
 *****************************************************************************/
void SPI_GPIO_Init(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	// 配置SPI的SCK，MISO和MOSI引脚为复用推挽模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;					   // 配置为主机模式
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;						   // NSS软件管理
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;					   // 第一个时钟沿捕获
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;						   // 空闲状态为低电平
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;				   // 8位数据帧
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // SPI波特率8分频 	36/8=4.5M
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // 全双工模式
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   // 数据高位先行
	SPI_InitStructure.SPI_CRCPolynomial = 7;						   // CRC计算多项式
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, ENABLE); // SPI1使能
}
/****************************************************************************************************
 * 函  数: void Exit_Init(void)
 * 功  能: 配置与SI24R1的IRQ相连的IO
 * 参  数: 无
 * 返回值：无
 * 备  注: 无
 ****************************************************************************************************/
void Exit_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct; // 定义GPIO结构体变量
	EXTI_InitTypeDef EXTI_InitStruct; // 定义外部中断结构体变量

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE); // 使能GPIOB的时钟并开启复用时钟

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4); // PB2中断线映射

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;		   // 配置GPIO第2引脚
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;	   // 配置GPIO为上拉输入
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; // 配置GPIO速率
	GPIO_Init(GPIOC, &GPIO_InitStruct);			   // GPIO初始化函数

	EXTI_InitStruct.EXTI_Line = EXTI_Line4;			 // 中断线2
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;	 // 外部中断模式
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;				 // 中断线使能
	EXTI_Init(&EXTI_InitStruct);						 // 外部中断初始化函数
	NVIC_EXIT_Config();
}
void NVIC_EXIT_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;	   // 配置外部中断通道
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; // 设置抢占优先级为0
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;		   // 设置子优先级为1
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;		   // 使能外部中断通道
	NVIC_Init(&NVIC_InitStruct);						   // 中断优先级初始化函数
}

void EXTI4_IRQHandler(void)
{
	uint8_t sta;
	if (EXTI_GetITStatus(EXTI_Line4) != RESET)
	{
		SI24R1_CE_LOW;								// 拉低CE，以便读取NRF中STATUS中的数据
		sta = SI24R1_read_reg(R_REGISTER + STATUS); // 读取STATUS中的数据，以便判断是由什么中断源触发的IRQ中断

		/* 发送完成中断 TX_OK */
		if (sta & TX_OK)
		{
			SI24R1set_Mode(IT_RX);
			SI24R1_write_reg(W_REGISTER + STATUS, TX_OK); // 清除发送完成标志·
			SI24R1_write_reg(FLUSH_TX, 0xff);			  // 清除TX_FIFO
		}
		/* 接收完成中断 RX_OK */
		if (sta & RX_OK)
		{
			SI24R1_RxPacket(SI24R1_RX_DATA);
			SI24R1_write_reg(W_REGISTER + STATUS, RX_OK); // 清除发送完成标志·
		}
		/* 达到最大重发次数中断  MAX_TX */
		if (sta & MAX_TX)
		{
			SI24R1set_Mode(IT_RX);
			SI24R1_write_reg(W_REGISTER + STATUS, MAX_TX); // 清除接达到最大重发标志
			SI24R1_write_reg(FLUSH_TX, 0xff);			   // 清除TX_FIFO
		}
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}

/*****************************************************************************
 * 函  数：uint8_t SPI1_WriteReadByte(uint8_t data)
 * 功  能：SPI1读写一个字节
 * 参  数：无
 * 返回值：无
 * 备  注：无
 *****************************************************************************/
uint8_t SPI1_WriteReadByte(uint8_t data)
{
	while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE))
		;
	SPI_I2S_SendData(SPI1, data);

	while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE))
		;
	return SPI_I2S_ReceiveData(SPI1);
}

/*****************************************************************************
 * 函  数：void SI24R1_Init(void)
 * 功  能：NRF引脚GPIO初始化
 * 参  数：无
 * 返回值：无
 * 备  注：无
 *****************************************************************************/
void SI24R1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE);

	/*   配置CSN引脚   */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	GPIO_ResetBits(GPIOC, GPIO_Pin_8);

	/*  配置CE引脚  */ // 使能引脚
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	GPIO_ResetBits(GPIOC, GPIO_Pin_7);

	Exit_Init();
	SPI_GPIO_Init(); // SPI1初始化

	SI24R1_Check(); // 检查SI24R1是否与MCU通信

	SI24R1_CSN_HIGH; // 失能NRF
	SI24R1_CE_LOW;	 // 待机模式
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_write_reg(uint8_t reg,uint8_t value)
 * 功  能：写一字节数据到寄存器
 * 参  数：reg： 寄存器地址
 *         val:  要写入的数据
 * 返回值：status
 * 备  注：SI24R1代码移植只需把SPI驱动修改成自己的即可
 *****************************************************************************/
uint8_t SI24R1_write_reg(uint8_t reg, uint8_t value)
{
	uint8_t status;

	SI24R1_CSN_LOW;
	status = SPI1_WriteReadByte(reg);
	SPI1_WriteReadByte(value);
	SI24R1_CSN_HIGH;

	return status;
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_read_reg(uint8_t reg)
 * 功  能：读一字节数据到寄存器
 * 参  数：reg： 寄存器地址
 * 返回值：reg_val
 * 备  注：SI24R1代码移植只需把SPI驱动修改成自己的即可
 *****************************************************************************/
uint8_t SI24R1_read_reg(uint8_t reg)
{
	uint8_t reg_val;

	SI24R1_CSN_LOW;
	SPI1_WriteReadByte(reg);
	reg_val = SPI1_WriteReadByte(0xff);
	SI24R1_CSN_HIGH;

	return reg_val;
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
 * 功  能：写一组数据到寄存器
 * 参  数：reg： 寄存器地址
 *         pBuf： 要写入数据的地址
 *         len:  要写入的数据长度
 * 返回值：status
 * 备  注：SI24R1代码移植只需把SPI驱动修改成自己的即可
 *****************************************************************************/
uint8_t SI24R1_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
	uint8_t status;
	int i;

	SI24R1_CSN_LOW;
	status = SPI1_WriteReadByte(reg);
	for (i = 0; i < len; i++)
	{
		SPI1_WriteReadByte(*pBuf);
		pBuf++;
	}
	SI24R1_CSN_HIGH;

	return status;
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
 * 功  能：读一组数据到寄存器
 * 参  数：reg： 	寄存器地址
 *         pBuf： 要读取数据的地址
 *         len:  	要读取的数据长度
 * 返回值：status
 * 备  注：SI24R1代码移植只需把SPI驱动修改成自己的即可
 *****************************************************************************/
uint8_t SI24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
	uint8_t status;
	int i;

	SI24R1_CSN_LOW;
	status = SPI1_WriteReadByte(reg);
	for (i = 0; i < len; i++)
	{
		*pBuf = SPI1_WriteReadByte(0xff);
		pBuf++;
	}
	SI24R1_CSN_HIGH;

	return status;
}

/*****************************************************************************
 * 函  数：void SI24R1set_Mode(uint8_t mode)
 * 功  能：切换SI24R1的工作模式模式
 * 参  数：无
 * 返回值：无
 * 备  注：无
 *****************************************************************************/
void SI24R1set_Mode(uint8_t mode)
{
	if (mode == IT_TX)
	{
		SI24R1_CE_LOW;
		SI24R1_write_reg(W_REGISTER + CONFIG, IT_TX);
		SI24R1_write_reg(W_REGISTER + STATUS, 0X7E); // 清除所有中断,防止一进去发送模式就触发中断
		SI24R1_CE_HIGH;
		//		Delay_us(15);
	}
	else
	{
		SI24R1_CE_LOW;
		SI24R1_write_reg(W_REGISTER + CONFIG, IT_RX); // 配置为接收模式
		SI24R1_write_reg(W_REGISTER + STATUS, 0X7E);  // 清除所有中断,防止一进去接收模式就触发中断
		SI24R1_CE_HIGH;
		Delayus(200);
	}
}

/*****************************************************************************
 * 函  数：void SI24R1_Config(void)
 * 功  能：SI24R1基本参数配置，并初始化为接收模式
 * 参  数：无
 * 返回值：无
 * 备  注：无
 *****************************************************************************/
void SI24R1_Config(void)
{
	SI24R1_CE_LOW;
	SI24R1_write_reg(W_REGISTER + SETUP_AW, 0x03);									// 配置通信地址的长度，默认值时0x03,即地址长度为5字节
	SI24R1_Write_Buf(W_REGISTER + TX_ADDR, (uint8_t *)TX_ADDRESS, TX_ADR_WIDTH);	// 写TX节点地址
	SI24R1_Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t *)TX_ADDRESS, RX_ADR_WIDTH); // 设置TX节点地址,主要为了使能ACK
	SI24R1_write_reg(W_REGISTER + SETUP_RETR, 0x1A);								// 设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次 0x1A

	SI24R1_write_reg(W_REGISTER + EN_RXADDR, 0x01);									// 使能通道0的接收地址
	SI24R1_write_reg(W_REGISTER + EN_AA, 0x01);										// 使能通道0自动应答
	SI24R1_write_reg(W_REGISTER + RX_PW_P0, RX_PAYLO_WIDTH);						// 选择通道0的有效数据宽度
	SI24R1_Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t *)RX_ADDRESS, RX_ADR_WIDTH); // 写RX节点地址
	SI24R1_write_reg(W_REGISTER + RF_CH, 30);										// 设置RF通道为40hz(1-64Hz都可以)
	SI24R1_write_reg(W_REGISTER + RF_SETUP, 0x27);									// 设置TX发射参数,0db增益,2Mbps,低噪声增益关闭 （注意：低噪声增益关闭/开启直接影响通信,要开启都开启，要关闭都关闭0x0f）

	SI24R1set_Mode(IT_RX); // 默认为接收模式

	SI24R1_CE_HIGH;
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_TxPacket(uint8_t *txbuf)
 * 功  能：SI24R1发送一包数据
 * 参  数：txbuf：要发送数据地址
 * 返回值：无
 * 备  注：无
 *****************************************************************************/
void SI24R1_TxPacket(uint8_t *txbuf)
{
	SI24R1_CE_LOW;
	SI24R1_Write_Buf(W_REGISTER + TX_ADDR, (uint8_t *)TX_ADDRESS, TX_ADR_WIDTH);	// 写TX节点地址
	SI24R1_Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t *)TX_ADDRESS, RX_ADR_WIDTH); // 设置TX节点地址,主要为了使能ACK
	SI24R1_Write_Buf(W_RX_PAYLOAD, txbuf, TX_PAYLO_WIDTH);							// 写数据到TX_BUFF
	SI24R1_write_reg(W_REGISTER + CONFIG, 0x0e);									// 设置为发送模式,开启所有中断
	SI24R1_write_reg(W_REGISTER + STATUS, 0X7E);									// 清除所有中断,防止一进去发送模式就触发中断
	SI24R1_CE_HIGH;
	Delayus(10); // CE持续高电平10us
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_RxPacket(uint8_t *rxbuf)
 * 功  能：SI24R1接收一包数据
 * 参  数：rxbuf：接收数据存储地址
 * 返回值：无
 * 备  注：无
 *****************************************************************************/
void SI24R1_RxPacket(uint8_t *rxbuf)
{
	SI24R1_CE_LOW;
	SI24R1_Read_Buf(R_RX_PAYLOAD, rxbuf, TX_PAYLO_WIDTH); // 读取RX的有效数据
	SI24R1_write_reg(FLUSH_RX, 0xff);					  // 清除RX FIFO(注意：这句话很必要)
	SI24R1_CE_HIGH;
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_testConnection(void)
 * 功  能：检查SI24R1与MCU的SPI总线是否通信正常
 * 参  数：无
 * 返回值：1已连接 0未连接
 * 备  注：无
 *****************************************************************************/
uint8_t SI24R1_testConnection(void)
{
	uint8_t buf[5] = {0XA5, 0XA5, 0XA5, 0XA5, 0XA5};
	uint8_t i;
	SI24R1_Write_Buf(W_REGISTER + TX_ADDR, buf, 5); // 写入5个字节的地址.
	SI24R1_Read_Buf(TX_ADDR, buf, 5);				// 读出写入的地址
	for (i = 0; i < 5; i++)
		if (buf[i] != 0XA5)
			break;
	if (i != 5)
		return 0; // 检测24L01错误
	return 1;	  // 检测到24L01
}

/*****************************************************************************
 * 函  数：void SI24R1_Check(void)
 * 功  能：检测SI24R1是否连接
 * 参  数：无
 * 返回值：无
 * 备  注：无
 *****************************************************************************/
void SI24R1_Check(void)
{
	while (!SI24R1_testConnection())
	{
		Delayms(100);
		printf("\r SI24R1 no connect...\r\n");
		// RGB_LED_Red();//红灯常亮
	}
	printf("\r SI24R1 connect...\r\n");
}

/*****************************************************************************
* 函  数：void SI24R1_GetAddr(void)
* 功  能：给飞机获取上的SI24R1获取一个地址
* 参  数：无
* 返回值：无
* 备  注：此函数需要与遥控器的对频函数联合使用否者SI24R1通信不成功，
		  如果自己做的的遥控器可直接用固定地址
*****************************************************************************/
#if 0
void SI24R1_GetAddr(void)
{
	if(SI24R1addr > SI24R1AddrMax)//当 SI24R1addr大于10，就说明次时SI24R1还未初始化完成
	{
		srand(SysTick->VAL);//给随机数种子
//		printf("SysTick->VAL:%d\r\n",SysTick->VAL);
		SI24R1addr = rand()%SI24R1AddrMax;//随机获取SI24R1最后一位地址（地址:0~50）
		PID_WriteFlash();//保存此地址Flash
	}else if(SI24R1addr != TX_ADDRESS[TX_ADR_WIDTH-1])
	{
		TX_ADDRESS[TX_ADR_WIDTH-1] = SI24R1addr;
		RX_ADDRESS[TX_ADR_WIDTH-1] = SI24R1addr;
		SI24R1_Config();
//		printf("SI24R1Addr:%d\r\n",SI24R1addr);
	}
}
#endif
/*****************************************************************************
 * 函  数：void SI24R1_Test(void)
 * 功  能：SI24R1通信测试函数
 * 参  数：无
 * 返回值：无
 * 备  注：测试时用
 *****************************************************************************/
void SI24R1_Test(void)
{
	uint8_t t = 0;
	static uint8_t mode, key;
	mode = ' ';
	key = mode;
	for (t = 0; t < 32; t++)
	{
		key++;
		if (key > ('~'))
			key = ' ';
		SI24R1_TX_DATA[t] = key;
	}
	mode++;
	if (mode > '~')
		mode = ' ';
	SI24R1_TxPacket(SI24R1_TX_DATA);
}
#endif
