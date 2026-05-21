/********************************************************************************
  * 文 件 名: bsp_key.c
  * 版 本 号: 初版
  * 修改作者: LC
  * 修改日期: 2022年04月15日
  * 功能介绍:          
  ******************************************************************************
  * 注意事项:
*********************************************************************************/

#include "bsp_key.h"
#include "sys.h"
#include "bsp_led.h"
#include "stdio.h"
#include "delay.h"

/************************************************
函数名称 ： key_gpio_config
功    能 ： keygpio引脚配置
参    数 ： 无
返 回 值 ： 无
作    者 ： LC
*************************************************/
void key_gpio_config(void)
{
	/* 开启时钟 */
	rcu_periph_clock_enable(BSP_KEY_RCU);
	/* 配置GPIO的模式 */
	gpio_mode_set(BSP_KEY_PORT,GPIO_MODE_INPUT,GPIO_PUPD_PULLDOWN,BSP_KEY_PIN);
}


uint8_t Key_GetNum(void)     //uint8_t 是返回值  
{
	uint8_t KeyNum = 0;    //按键键码默认为0  没有按键按下就返回0   KeyNum这个KeyNum是局部变量
	if (gpio_input_bit_get(BSP_KEY_PORT,BSP_KEY_PIN) == SET)
	{
		delay_ms(20);      //消除按下的抖动
		while (gpio_input_bit_get(BSP_KEY_PORT,BSP_KEY_PIN) == SET);
		delay_ms(20);      //消除松手的抖动
		KeyNum = 1;
	}
}