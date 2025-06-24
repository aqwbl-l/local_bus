# include "stm32f10x.h"                  // Device header
# include "delay.h"
# include "sys.h"
# include "usart.h"
# include "timer.h"
# include "air.h"
# include "motor.h"
# include "DHT11.h"
# include "stdbool.h"

//数据发送周期
uint8_t  overtime = 0;
uint8_t temp = 0,humi = 0;
uint8_t air_mode = 0;//1:根据模拟量调整，0:根据开关量调整
float air;
//从407接收到的模式参数
uint16_t receive_air_mode;
int main(void)
{
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_4);
	
	uart1_Init(115200);
	motor_Init();
	air_Init();
	DHT11_Init();
	DHT11_Rst();
	Timer2_Init();
    while(1)
	{
		//接收处理
		if(usart_GetFlag())
		{
			air_mode = usart_ReceiveData();
		}
		air_mode = 0;
		if(overtime==120)
		{
			air = air_out();
			DHT11_Read_Data(&temp,&humi);	//读取温湿度数据
			SendDataPacket(temp,humi,0,air);
			overtime=0;
		}
		if(air_mode==0)
		{
			if(air_key())
			{
				motor_Control(100);
			}
			else
			{
				motor_Control(0);
			}
		}
		if(air_mode==1)
		{
			motor_Control((int)air_out());
		}
		
		
	}

}

// TIM2中断服务函数
void TIM2_IRQHandler(void) 
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) 
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);// 清楚中断标志位
		overtime++;
    }	
}


