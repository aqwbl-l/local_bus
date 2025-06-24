#include "usart.h"

uint8_t receiveflag = 0;
uint16_t receiveData = 0;
#pragma import(__use_no_semihosting)             
//标准库需要支持的函数
struct __FILE 
{
	int handle;
};

FILE __stdout;       
//定义_sys_exit()以避免工作在半主机状态
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数
//这个需要根据MCU和我们希望printf从哪个串口输出来确认 __WAIT_TODO__
int fputc(int ch, FILE *f)
{
	//注意：USART_FLAG_TXE是检查发送缓冲区是否为空，这个要在发送前检查，检查这个提议提高发送效率，但是在休眠的时候可能导致最后一个字符丢失
	//USART_FLAG_TC是检查发送完成标志，这个在发送后检查，这个不会出现睡眠丢失字符问题，但是效率低（发送过程中发送缓冲区已经为空了，可以接收下一个数据了，但是因为要等待发送完成，所以效率低）
	//不要两个一起用，一起用效率最低
	//循环等待直到发送缓冲区为空(TX Empty)此时可以发送数据到缓冲区
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
  {}
	USART_SendData(USART1, (uint8_t)ch);
  /* 循环等待直到发送结束*/
  //while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){}
	return ch;
}
void uart1_Init(u32 bound)//串口1  引脚为PA9-tx  PA10-rx
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//使能指定端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//初始化GPIO
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//初始化GPIO
	
	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         
	NVIC_Init(&NVIC_InitStructure); 

	
	//USART1配置
	USART_InitStructure.USART_BaudRate = bound;	//设置串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;	//无奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStructure); //配置USART参数
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//配置了接收中断和总线空闲中断
	
	USART_Cmd(USART1, ENABLE);//使能USART
}

// 发送单个字节
void USART1_SendByte(uint8_t byte) {
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, byte);
}

// 发送数据包（修改后：发送3个uint8_t和1个float）
void SendDataPacket(uint8_t uint8_1, uint8_t uint8_2, uint8_t uint8_3, float float1) 
{
    // 帧头
    USART1_SendByte(0xAA);
    USART1_SendByte(0x55);
    
    // 发送3个uint8_t数据（每个uint8_t占1字节）
    USART1_SendByte(uint8_1);
    USART1_SendByte(uint8_2);
    USART1_SendByte(uint8_3);
    
    // 发送浮点数（float占4字节）
    uint8_t* p = (uint8_t*)&float1;
    USART1_SendByte(p[0]);
    USART1_SendByte(p[1]);
    USART1_SendByte(p[2]);
    USART1_SendByte(p[3]);
}

//接收
uint8_t usart_GetFlag(void)
{
	if(receiveflag==1)
	{
		receiveflag = 0;
		return 1;
	}
	return 0;	
}
uint16_t usart_ReceiveData(void)
{
	return receiveData;
}

// 串口1中断处理函数（接收数据）
void USART1_IRQHandler(void)
{ 
	if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);//清除中断标志位
		receiveflag = 1;
		receiveData = USART_ReceiveData(USART1);	
	}   
}


