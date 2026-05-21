#ifndef _BSP_MLX90614_H_
#define _BSP_MLX90614_H_

#include "gd32f4xx.h"

//端口移植
#define RCU_SDA RCU_GPIOC
#define PORT_SDA GPIOC
#define GPIO_SDA GPIO_PIN_9

#define RCU_SCL RCU_GPIOA
#define PORT_SCL GPIOA
#define GPIO_SCL GPIO_PIN_8

//设置SDA输出模式
#define SDA_OUT()        gpio_mode_set(PORT_SDA,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLUP,GPIO_SDA)
//设置SDA输入模式
#define SDA_IN()        gpio_mode_set(PORT_SDA,GPIO_MODE_INPUT,GPIO_PUPD_PULLUP,GPIO_SDA)
//获取SDA引脚的电平变化
#define SDA_GET()        gpio_input_bit_get(PORT_SDA,GPIO_SDA)
//SDA与SCL输出
#define SDA(x)          gpio_bit_write(PORT_SDA,GPIO_SDA, (x?SET:RESET))
#define SCL(x)          gpio_bit_write(PORT_SCL,GPIO_SCL, (x?SET:RESET))


void MLX90614_GPIO_Init(void);
float MLX90614_Read(unsigned char SlaveAddr, unsigned char RegAddr);
#endif
