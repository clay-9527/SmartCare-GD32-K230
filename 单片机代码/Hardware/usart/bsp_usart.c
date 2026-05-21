#include "bsp_usart.h"
#include "stdio.h"
#include <string.h>
#include <stdarg.h>

uint8_t 	g_recv_buff[USART_RECEIVE_LENGTH];   // 接收缓冲区
uint16_t 	g_recv_length = 0;									 // 接收数据长度
uint8_t		g_recv_complete_flag = 0; 					 // 接收数据完成标志位

/************************************************
函数名称 ： usart_gpio_config
功    能 ： 串口配置GPIO
参    数 ： band_rate:波特率
返 回 值 ： 无
作    者 ： LC
*************************************************/
void usart_gpio_config(uint32_t band_rate)
{
	/* 开启时钟 */
	rcu_periph_clock_enable(BSP_USART_TX_RCU);   // 开启串口时钟
	rcu_periph_clock_enable(BSP_USART_RX_RCU);   // 开启端口时钟
	rcu_periph_clock_enable(BSP_USART_RCU);      // 开启端口时钟
	
	/* 配置GPIO复用功能 */
  gpio_af_set(BSP_USART_TX_PORT,BSP_USART_AF,BSP_USART_TX_PIN);	
	gpio_af_set(BSP_USART_RX_PORT,BSP_USART_AF,BSP_USART_RX_PIN);	
	
	/* 配置GPIO的模式 */
	/* 配置TX为复用模式 上拉模式 */
	gpio_mode_set(BSP_USART_TX_PORT,GPIO_MODE_AF,GPIO_PUPD_PULLUP,BSP_USART_TX_PIN);
	/* 配置RX为复用模式 上拉模式 */
	gpio_mode_set(BSP_USART_RX_PORT, GPIO_MODE_AF,GPIO_PUPD_PULLUP,BSP_USART_RX_PIN);
	
	/* 配置TX为推挽输出 50MHZ */
	gpio_output_options_set(BSP_USART_TX_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,BSP_USART_TX_PIN);
	/* 配置RX为推挽输出 50MHZ */
	gpio_output_options_set(BSP_USART_RX_PORT,GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_USART_RX_PIN);

	/* 配置串口的参数 */
	usart_deinit(BSP_USART);                                 // 复位串口
	usart_baudrate_set(BSP_USART,band_rate);                 // 设置波特率
	usart_parity_config(BSP_USART,USART_PM_NONE);            // 没有校验位
	usart_word_length_set(BSP_USART,USART_WL_8BIT);          // 8位数据位
	usart_stop_bit_set(BSP_USART,USART_STB_1BIT);     			 // 1位停止位

  /* 使能串口 */
	usart_enable(BSP_USART);                                 // 使能串口
	usart_transmit_config(BSP_USART,USART_TRANSMIT_ENABLE);  // 使能串口发送
	usart_receive_config(BSP_USART,USART_RECEIVE_ENABLE);    // 使能串口接收
	
	/* 中断配置 */
	nvic_irq_enable(BSP_USART_IRQ, 2, 2); 									 // 配置中断优先级
	usart_interrupt_enable(BSP_USART,USART_INT_RBNE);				 // 读数据缓冲区非空中断和溢出错误中断
	usart_interrupt_enable(BSP_USART,USART_INT_IDLE);				 // 空闲检测中断
}

/************************************************
函数名称 ： usart_send_data
功    能 ： 串口重发送一个字节
参    数 ： ucch：要发送的字节
返 回 值 ： 
作    者 ： LC
*************************************************/
void usart_send_data(uint8_t ucch)
{
	usart_data_transmit(BSP_USART,(uint8_t)ucch);							 // 发送数据
	while(RESET == usart_flag_get(BSP_USART,USART_FLAG_TBE));  // 等待发送数据缓冲区标志置位
}


/************************************************
函数名称 ： usart_send_String
功    能 ： 串口发送字符串
参    数 ： ucstr:要发送的字符串
返 回 值 ： 
作    者 ： LC
*************************************************/
void usart_send_string(uint8_t *ucstr)
{
	while(ucstr && *ucstr)        // 地址为空或者值为空跳出
	{
	  usart_send_data(*ucstr++);  // 发送单个字符
	}
}

/************************************************
函数名称 ： fputc
功    能 ： 串口重定向函数
参    数 ： 
返 回 值 ： 
作    者 ： LC
*************************************************/
//int fputc(int ch, FILE *f)
//{
//     usart_send_data(ch);
//     // 等待发送数据缓冲区标志置位
//     return ch;
//}
__align(8) char USART0_TxBuff[256];
void u0_printf(char* fmt,...)
{
    unsigned int i=0,length=0;
    
    va_list ap;
    va_start(ap,fmt);
    vsprintf(USART0_TxBuff,fmt,ap);
    va_end(ap);
    
    length=strlen((const char*)USART0_TxBuff);
    while(i<length)
    {
        usart_send_data(USART0_TxBuff[i]);
        i++;
    }
    while(usart_flag_get(USART0,USART_FLAG_TC)==RESET);
}

/************************************************
函数名称 ： BSP_USART_IRQHandler
功    能 ： 串口接收中断服务函数
参    数 ： 无
返 回 值 ： 无
作    者 ： LC
*************************************************/
void BSP_USART_IRQHandler(void)
{
	if(usart_interrupt_flag_get(BSP_USART,USART_INT_FLAG_RBNE) == SET)   // 接收缓冲区不为空
	{
		g_recv_buff[g_recv_length++] = usart_data_receive(BSP_USART);      // 把接收到的数据放到缓冲区中
	}
	
	if(usart_interrupt_flag_get(BSP_USART,USART_INT_FLAG_IDLE) == SET)   // 检测到帧中断
	{
		usart_data_receive(BSP_USART);                                     // 必须要读，读出来的值不能要
		g_recv_buff[g_recv_length] = '\0';																 // 数据接收完毕，数组结束标志
		g_recv_complete_flag = 1;                                          // 接收完成 
	}
	
}
