#include "bsp_led.h"

/************************************************
函数名称 ： led_gpio_config
功    能 ： led灯gpio引脚配置
参    数 ： 无
返 回 值 ： 无
作    者 ： LC
*************************************************/
void led_gpio_config(void)
{
  /* 使能时钟 */
	rcu_periph_clock_enable(RCU_LED2);
	/* 配置为输出模式 浮空模式 */
	gpio_mode_set(PORT_LED2,GPIO_MODE_OUTPUT,GPIO_PUPD_NONE,PIN_LED2);
	/* 配置为推挽输出 50MHZ */
	gpio_output_options_set(PORT_LED2,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,PIN_LED2);
}
