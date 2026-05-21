#ifndef _BSP_BASIC_TIMER_H
#define _BSP_BASIC_TIMER_H

#include "gd32f4xx.h"
#include "delay.h"

#define BSP_TIMER_RCU  				RCU_TIMER5            // 定时器时钟
#define BSP_TIMER      				TIMER5                // 定时器
#define BSP_TIMER_IRQ  				TIMER5_DAC_IRQn       // 定时器中断
#define BSP_TIMER_IRQHANDLER  TIMER5_DAC_IRQHandler // 定时器中断服务函数


//#define BSP_TIMER_RCU  				RCU_TIMER2					// 定时器时钟
//#define BSP_TIMER      				TIMER2							// 定时器
//#define BSP_TIMER_IRQ  			  TIMER2_IRQn					// 定时器中断
//#define BSP_TIMER_IRQHANDLER  TIMER2_IRQHandler		// 定时器中断服务函数


void basic_timer_config(uint16_t pre,uint16_t per); // 基本定时器配置

#endif  /* BSP_BASIC_TIMER_H */
