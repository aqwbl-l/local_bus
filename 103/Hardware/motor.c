#include "stm32f10x.h"                  // Device header

/**
 * @brief  初始化电机控制相关引脚和PWM输出
 * @param  无
 * @retval 无
 */
void motor_Init(void)
{
    // 开启GPIOA和TIM2的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // 配置GPIO - 方向控制引脚 (AIN1:PA4, AIN2:PA5)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 初始状态：电机停止
    GPIO_ResetBits(GPIOA, GPIO_Pin_4 | GPIO_Pin_5);
    
    // 配置GPIO - PWM输出引脚 (PA2)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 初始化TIM2 - 配置时基单元
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    // 计算公式: PWM频率 = 72MHz / ((PSC+1) * (ARR+1))
    // 当前配置: 72MHz / (32 * 100) = 22.5kHz
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;     // ARR自动重装值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 32 - 1;   // PSC预分频系数
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
    
    // 初始化TIM2输出比较通道3
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;       // PWM模式1
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 输出高电平有效
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 使能输出
    TIM_OCInitStructure.TIM_Pulse = 0;                     // 初始占空比为0%
    TIM_OC3Init(TIM2, &TIM_OCInitStructure);
    
    // 使能TIM2
    TIM_Cmd(TIM2, ENABLE);
}

/**
 * @brief  控制电机转速
 * @param  speed: 电机速度 (0-100%,对应TIM2的CCR值)
 * @retval 无
 */
void motor_Control(uint8_t speed)
{
    // 限制速度范围
    if (speed > 100) {
        speed = 100;
    }
    
    // 设置固定方向（正转）
    GPIO_SetBits(GPIOA, GPIO_Pin_5);
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    
    // 设置PWM占空比控制速度
    TIM_SetCompare3(TIM2, speed);
}