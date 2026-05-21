#ifndef _BSP_UART3_H
#define _BSP_UART3_H

#include "gd32f4xx.h"
#include "systick.h"

#define BSP_UART_TX_RCU  RCU_GPIOA   // 串口TX的端口时钟
#define BSP_UART_RX_RCU  RCU_GPIOA   // 串口RX的端口时钟
#define BSP_UART_RCU     RCU_UART3  // 串口3的时钟

#define BSP_UART_TX_PORT GPIOA				// 串口TX的端口
#define BSP_UART_RX_PORT GPIOA				// 串口RX的端口
#define BSP_UART_AF 			GPIO_AF_8   // 串口3的复用功能
#define BSP_UART_TX_PIN  GPIO_PIN_0  // 串口TX的引脚
#define BSP_UART_RX_PIN  GPIO_PIN_1 // 串口RX的引脚

#define BSP_UART 						UART3      								// 串口3
#define BSP_UART_IRQ     		UART3_IRQn 								// 串口3中断
#define BSP_UART_IRQHandler  UART3_IRQHandler					// 串口3中断服务函数



/* 串口缓冲区的数据长度 */
#define UART_RECEIVE_LENGTH  4096

extern uint8_t  g_recv_buff1[UART_RECEIVE_LENGTH]; // 接收缓冲区
extern uint16_t g_recv_length1;										 // 接收数据长度
extern uint8_t  g_recv_complete_flag1; 						 // 接收完成标志位

void uart_gpio_config3(uint32_t band_rate);  			 // 配置串口
void uart_send_data(uint8_t ucch);          			 // 发送一个字符
void uart_send_string(uint8_t *ucstr);      			 // 发送一个字符串

#endif
