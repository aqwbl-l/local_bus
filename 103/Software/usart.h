#ifndef __USART_H
#define __USART_H 			   

#include "stm32f10x.h"
#include "stdio.h"
extern uint8_t receiveflag;
void uart1_Init(u32 bound);
void USART1_SendByte(uint8_t byte);

// 发送单个字节
void usart_SendData(uint16_t Data);
void SendDataPacket(uint8_t uint8_1, uint8_t uint8_2, uint8_t uint8_3, float float1) ;
void USART1_IRQHandler(void);
	
uint8_t usart_GetFlag(void);
uint16_t usart_ReceiveData(void);
#endif
