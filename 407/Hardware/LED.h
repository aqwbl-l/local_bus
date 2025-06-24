# ifndef LED_H
# define LED_H

# include "stm32f4xx.h"
# include "stdint.h"
# include "LED.h"
//PF9-RED, PF10-GREEN		低电平有效
void led_Init(void);
enum LED_type{LED_RED,LED_GREEN};
void LED_ON(enum LED_type led);
void LED_OFF(enum LED_type led);
void LED_TOGGLE(enum LED_type led);	

# endif
