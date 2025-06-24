#include "usart.h"
#include "esp.h"
//UART1 PA9,PA10			PC
//UART2 PA2-TX, PA3-RX		ESP01S
//UART3 PC10-TX,PC11-RX		103

// 定义全局变量存储解析后的数据
uint8_t g_receivedInt1 = 0;
uint8_t g_receivedInt2 = 0;
uint8_t g_receivedInt3 = 0;
float   g_receivedFloat = 0.0f;

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
void uart2_Init(u32 bound) 
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    //串口2对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); 
	
    // 配置TX引脚(PA2)为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    
    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 配置USART2
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    
    // 使能中断
    USART_ITConfig(USART2, USART_IT_RXNE | USART_IT_IDLE, ENABLE);
    
    // 使能USART2
    USART_Cmd(USART2, ENABLE);
}

// 串口2中断处理函数（ESP01S接收数据）
void USART2_IRQHandler(void)
{   
    u8 ucCh;
    if(USART_GetITStatus( USART2, USART_IT_RXNE ) != RESET )
    {
        ucCh  = USART_ReceiveData( USART2 );
		USART_SendData(USART1,ucCh);//将接收到的数据通过uasrt1打印
        if(FramLength < ( RX_BUF_MAX_LEN - 1 ) ) 
        {
            Data_RX_BUF[FramLength ++ ]  = ucCh;   
        }
    }
    if( USART_GetITStatus( USART2, USART_IT_IDLE ) == SET )//如果总线空闲
    {
        FramFinishFlag = 1;
        ucCh = USART_ReceiveData( USART2 ); //由软件序列清除中断标志位（先读USART_SR,然后读USART_DR）
    }   
}

void uart1_Init(u32 bound)//串口1  引脚为PA9-RX  PA10-TX 
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);; 

	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); 
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_9; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      //响应优先级0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //USART_IRQn通道使能
    NVIC_Init(&NVIC_InitStructure); //初始化NVIC
    //USART1 配置
    USART_InitStructure.USART_BaudRate = bound;//波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//数据长度
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//停止位1
    USART_InitStructure.USART_Parity = USART_Parity_No;//校验位无
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件流控制无
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //使能串口的接收和发送功能
    USART_Init(USART1, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART1, USART_IT_RXNE|USART_IT_IDLE, ENABLE);//配置了接收中断和总线空闲中断

    USART_Cmd(USART1, ENABLE);      //串口外设使能    
}
void USART1_IRQHandler( void )
{   
    u8 ucCh;
    if(USART_GetITStatus( USART1, USART_IT_RXNE ) != RESET )
    {
        ucCh  = USART_ReceiveData( USART1 );
        if(FramLength < ( RX_BUF_MAX_LEN - 1 ) ) 
        {
            //留最后一位做结束位
            Data_RX_BUF[FramLength ++ ]  = ucCh;   
        }                      
    }  
}

void uart3_Init(u32 bound)//串口3  PC10-TX,PC11-RX
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能指定端口时钟
	
	//串口3对应引脚复用映射
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_USART3); 
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_USART3); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure);	//初始化GPIO

	
	//Usart3 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         
	NVIC_Init(&NVIC_InitStructure); 

	
	//USART3配置
	USART_InitStructure.USART_BaudRate = bound;	//设置串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;	//无奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART3, &USART_InitStructure); //配置USART参数
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//配置了接收中断和总线空闲中断
	
	USART_Cmd(USART3, ENABLE);//使能USART

}



// USART3中断服务程序


// 接收状态机变量
static uint8_t g_rxState = 0;  // 0:等待帧头AA, 1:等待帧头55, 2~8:接收数据
static uint8_t g_rxBuffer[9];  // 数据包缓冲区

/**
 * USART3中断服务函数 - 直接在中断中处理所有接收和解析逻辑
 */
void USART3_IRQHandler(void) {
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        uint8_t byte = USART_ReceiveData(USART3);  // 读取接收到的字节
        
        switch (g_rxState) {
            case 0:  // 等待帧头AA
                if (byte == 0xAA) {
                    g_rxBuffer[0] = byte;
                    g_rxState = 1;  // 进入等待帧头55状态
                }
                break;
                
            case 1:  // 等待帧头55
                if (byte == 0x55) {
                    g_rxBuffer[1] = byte;
                    g_rxState = 2;  // 开始接收数据
                } else {
                    g_rxState = 0;  // 帧头错误，重置
                }
                break;
                
            case 2: case 3: case 4: case 5: case 6: case 7: case 8:
                g_rxBuffer[g_rxState] = byte;
                
                if (g_rxState == 8) {  // 接收完最后一个字节
                    // 验证帧头并解析数据
                    if (g_rxBuffer[0] == 0xAA && g_rxBuffer[1] == 0x55) {
                        // 提取三个uint8_t整数
                        g_receivedInt1 = g_rxBuffer[2];
                        g_receivedInt2 = g_rxBuffer[3];
                        g_receivedInt3 = g_rxBuffer[4];
                        
                        // 提取浮点数（小端模式）
                        uint8_t* floatBytes = (uint8_t*)&g_receivedFloat;
                        floatBytes[0] = g_rxBuffer[5];
                        floatBytes[1] = g_rxBuffer[6];
                        floatBytes[2] = g_rxBuffer[7];
                        floatBytes[3] = g_rxBuffer[8];
                    }
                    
                    g_rxState = 0;  // 重置状态机
                } else {
                    g_rxState++;  // 继续接收下一个字节
                }
                break;
                
            default:
                g_rxState = 0;  // 异常状态，重置
                break;
        }
        
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);  // 清除中断标志
    }
}

int re(void)
{
	return 0;
}
void uart2_SendData(uint8_t Data)
{
	USART_SendData(USART2,Data);//发送一个字节数据
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==0);//等待发送数据寄存器空
}

void uart3_SendData(uint8_t Data)
{
	USART_SendData(USART3,Data);//发送一个字节数据
	while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==0);//等待发送数据寄存器空
}

