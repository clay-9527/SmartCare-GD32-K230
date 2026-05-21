#ifndef _BSP_KEY_H
#define _BSP_KEY_H

#include "gd32f4xx.h"
#include "systick.h"

#define BSP_KEY_RCU   RCU_GPIOA   // 按键端口时钟
#define BSP_KEY_PORT  GPIOA       // 按键端口
#define BSP_KEY_PIN   GPIO_PIN_2  // 按键引脚
 
void key_gpio_config(void);       // key gpio引脚配置
uint8_t Key_GetNum(void);
#endif  /* BSP_KEY_H */