#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include "esp.h"
#include "LED.h"
#include "timer.h"
#include "stdint.h"
#include "stdbool.h"
/*    从103获取到的数据		*/
extern uint8_t g_receivedInt1;//温度
extern uint8_t g_receivedInt2;//湿度
extern uint8_t g_receivedInt3;//模式
extern float   g_receivedFloat;//浓度
/* 		间隔					*/		
uint16_t overtime;	//数据交换周期


/*    与ONENT交换的数据       */
int32_t humi,temp;
float air;
uint16_t cur_air_mode = 0;

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init(168);
	uart2_Init(115200);
	uart1_Init(115200);
	uart3_Init(115200);
	Timer2_Init();
	led_Init();
	ESP01S_Connect_wifi();
	delay_ms(2000);
	uint8_t onenet_status = ESP01S_Connect_onenet();
	while(1)
	{
		//连接onenet
		if(!onenet_status)
		{
			delay_ms(5000);
			ESP01S_Connect_onenet();
		}
		//控制模式切换
		cur_air_mode = ESP8266_air_m();
		if(cur_air_mode)
		{
			LED_ON(LED_GREEN);
			uart3_SendData(1);
		}
		else
		{
			LED_OFF(LED_GREEN);
			uart3_SendData(0);
		}
		
		LED_TOGGLE(LED_RED);
		delay_ms(1000);
		if(onenet_status && (overtime >= 60))//每隔5min向ONENET发送一次数据
		{
			send_temp((uint32_t)g_receivedInt1);
			send_humi((uint32_t)g_receivedInt2);
			send_air((float)g_receivedFloat);
			
			uart3_SendData(cur_air_mode);
			overtime =0;
		}
		
	}
}

//定时器2中断服务函数, 每隔一秒执行一次
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET) //溢出中断
	{
		overtime++;
	}
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  //清除中断标志位
}

