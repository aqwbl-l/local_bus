#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include <stdio.h>


//DHT11引脚宏定义
#define DHT11_GPIO_PORT  GPIOA
#define DHT11_GPIO_PIN   GPIO_Pin_3
#define DHT11_GPIO_CLK   RCC_APB2Periph_GPIOA

//输出状态定义
#define OUT 1
#define IN  0

//控制DHT11引脚输出高低电平
#define DHT11_Low  GPIO_ResetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN)
#define DHT11_High GPIO_SetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN)


uint8_t DHT11_Init(void);//初始化DHT11
uint8_t DHT11_Read_Data(uint8_t *temp,uint8_t *humi) ;//读取温湿度数据
void DHT11_Rst(void);//复位DHT11 

#endif
