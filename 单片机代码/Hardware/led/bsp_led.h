#ifndef _BSP_LED_H
#define _BSP_LED_H

#include "gd32f4xx.h"
#include "delay.h"


#define RCU_LED2  	RCU_GPIOD   // GPIOD的时钟
#define PORT_LED2 	GPIOD				// GPIOD的端口
#define PIN_LED2 		GPIO_PIN_7  // GPIOD的引脚

void led_gpio_config(void); 	  // led gpio引脚配置

#endif /* BSP_LED_H */
