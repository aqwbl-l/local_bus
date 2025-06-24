#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include "sys.h"

//使用3.3V供电
//PA0-D0 PA1-A0

void  air_Init(void)
{ 	
	
	//开启时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE );
	
	//初始化GPIO
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//输入上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//输入模拟
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 
	ADC_DeInit(ADC1);//复位ADC1
	
	//配置ADC通道
	ADC_InitTypeDef ADC_InitStructure; 
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;//关闭扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//使用单次转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//禁用硬件触发ADC
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//设置数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//通道数目
	ADC_Init(ADC1, &ADC_InitStructure);	 

	//使能ADC1
	ADC_Cmd(ADC1, ENABLE);
	
	//校准ADC
	ADC_ResetCalibration(ADC1); //复位 
	while(ADC_GetResetCalibrationStatus(ADC1));//等待复位
	ADC_StartCalibration(ADC1);	 //开启AD校准
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准
 
}				  
//获得ADC值
//ch:通道值 0~3
uint16_t Get_ADC(void)   
{
	ADC_RegularChannelConfig(ADC1,ADC_Channel_1, 1, ADC_SampleTime_55Cycles5 );		  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

uint16_t Get_ADC_Average(uint8_t times)
{
	uint32_t temp_val=0;
	uint8_t t;
	for(t=0;t<times;t++)
	{
		temp_val+= Get_ADC();
		delay_ms(5);
	}
	return temp_val/times;
} 	

 
/*
将0-5v的模拟量输入转换为 0-100 的显示值
*/
float air_read(void)
{
	uint16_t adcx;
	float voltage;
	adcx=Get_ADC_Average(3);
	voltage = adcx/409.5;

    return voltage;
}

float air_out(void)
{
	int i;
	float num = 0;
	float out;
	for(i=0;i<3;i++){ num += air_read();}  
	out = num/3.0;  
	return out;
}

//将传感器作为开关量使用
uint8_t air_key(void)
{
	uint8_t data = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
	if(!data)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}



