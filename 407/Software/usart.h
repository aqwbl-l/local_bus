#ifndef __USART_H
#define __USART_H 			   

#include "stm32f4xx.h"
#include "stdio.h"
extern uint8_t receiveflag;
void uart2_Init(u32 bound);
void uart1_Init(u32 bound);
void uart3_Init(u32 bound);
void USART2_IRQHandler(void);
void USART1_IRQHandler(void);
void USART3_IRQHandler(void);
	
void uart2_SendData(uint8_t Data);
void uart3_SendData(uint8_t Data);


//与103交换数据，所需变量

#endif
