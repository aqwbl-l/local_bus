# include "LED.h"
//PF9-RED, PF10-GREEN		低电平有效
void led_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStruct);
	
	GPIO_SetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
}

//enum LED_type{LED_RED,LED_GREEN};
void LED_ON(enum LED_type led)
{
	if(led == LED_RED)
	{
		GPIO_WriteBit(GPIOF, GPIO_Pin_9,Bit_RESET);
	}
	else if(led == LED_GREEN)
	{
		GPIO_WriteBit(GPIOF, GPIO_Pin_10,Bit_RESET);
	}
}
void LED_OFF(enum LED_type led)
{
	if(led == LED_RED)
	{
		GPIO_WriteBit(GPIOF, GPIO_Pin_9,Bit_SET);
	}
	else if(led == LED_GREEN)
	{
		GPIO_WriteBit(GPIOF, GPIO_Pin_10,Bit_SET);
	}
}
void LED_TOGGLE(enum LED_type led)
{
	if(led == LED_RED)
	{
		GPIO_ToggleBits(GPIOF,GPIO_Pin_9);
	}
	else if(led == LED_GREEN)
	{
		GPIO_ToggleBits(GPIOF,GPIO_Pin_10);
	}
}
