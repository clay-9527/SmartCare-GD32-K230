#include "gd32f4xx.h"
#include "delay.h"
#include <stdio.h>
#include "main.h"
#include "bsp_led.h"
#include "sys.h"
#include "bsp_usart.h"
#include "bsp_basic_timer.h"
#include "bsp_pwm.h"
#include "stdlib.h"
#include "string.h"
#include "bsp_sht30.h"
#include "bsp_mlx90614.h"
#include "esp01s.h"
#include "GUI.h"
#include "Lcd_Driver.h"
#include "bsp_usart3.h"
#include "bsp_key.h"

#define data "AT+MQTTPUB=0,\"%s",\"{\\\"id\\\": \\\"1234\\\"\\,\\\"version\\\": \\\"1.0\\\"\\,\\\"params\\\": {\\\"tempm\\\": {\\\"value\\\": 35.5}\\,\\\"temp\\\": {\\\"value\\\": 23.5 }\\,\\\"humi\\\": {\\\"value\\\": 28.5}}}\",1,0\r\n"
void onenet_send_data(void);

uint8_t KeyNum;        // 按键编号


uint8_t action1 = 0;
uint8_t action2 = 0;
uint8_t action3 = 0;
uint8_t action4 = 0;
uint8_t action5 = 0;

extern uint8_t      g_recv_buff[];   // 接收缓冲区
static uint16_t 		servo_angle = 0;
extern uint16_t     g_recv_length;   // 接收数据长度
extern uint8_t      g_recv_complete_flag;  // 接收完成标志



float Angle1 = 200;
float Angle2 = 200;
float Angle3 = 200;
float Angle4 = 200;
float Angle5 = 250;


// 标准化后的全局变量声明
float TargetAngle1 = 200;
float TargetAngle2 = 200;
float TargetAngle3 = 200;
float TargetAngle4 = 200;
float TargetAngle5 = 250;

float step = 1.0f;

// 自动模式标志位
uint8_t auto_mode = 0;

// 保持原有舵机控制
uint8_t increase_flag1 = 0, decrease_flag1 = 0;
uint8_t increase_flag2 = 0, decrease_flag2 = 0;
uint8_t increase_flag3 = 0, decrease_flag3 = 0;
uint8_t increase_flag4 = 0, decrease_flag4 = 0;
uint8_t increase_flag5 = 0, decrease_flag5 = 0;
uint8_t increase_flag6 = 0, decrease_flag6 = 0;



// 动作流程状态
uint8_t servo_task_step = 0;

// 等待计数
uint16_t wait_count = 0;

// 记录舵机初始位置
float OriginAngle1 = 200;
float OriginAngle2 = 200;
float OriginAngle3 = 200;
float OriginAngle4 = 200;
float OriginAngle5 = 250;

// 判断是否到达目标角度
uint8_t Servo_Arrived(void)
{
    if(Angle1 == TargetAngle1 &&
       Angle2 == TargetAngle2 &&
       Angle3 == TargetAngle3 &&
       Angle4 == TargetAngle4 &&
       Angle5 == TargetAngle5)
    {
        return 1;
    }
    return 0;
}



int main(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);  // 设置NVIC优先级分组
    systick_config();                  // 系统滴答定时器初始化
	
    DelayInit();
		usart_gpio_config(115200U);         // 串口0初始化
		uart_gpio_config3(115200U);
		Servo_Init();
		key_gpio_config();
		SHT30_GPIO_Init();
		MLX90614_GPIO_Init();
		WIFI_ESP01S_Init();
		WIFI_MODE_STA_Aliyun_Init();                   //WiFi链接模式
		onenet_send_data();	
		Lcd_Init();
		LCD_LED_SET;//通过IO控制背光亮	
		Lcd_Clear(GRAY0);	
		DisplayButtonUp(40,8,180,30); // 左边框位置，上边框位置 ，右边框位置，下边框位置
		Gui_DrawFont_GBK16(60,12,RED,GRAY0,"智能监测系统");			// 在坐标(x,y)处显示颜色文字"xxxxxx"，背景为颜色
		
		DisplayButtonUp(8,45,90,65);    //左 上 右 下				
		Gui_DrawFont_GBK16(12,49,BLACK,GRAY0,"体温：");   //起始左 下
		DisplayButtonUp(98,45,217,65); 				
		Gui_DrawFont_GBK16(100,47,BLACK,GRAY0,"体温阈值：36-38");
		DisplayButtonUp(8,75,90,96); 
		Gui_DrawFont_GBK16(12,79,BLACK,GRAY0,"湿度：");
		DisplayButtonUp(98,75,217,96); 
		Gui_DrawFont_GBK16(100,77,BLACK,GRAY0,"湿度阈值：40-60");
		DisplayButtonUp(8,105,90,125); 	
		Gui_DrawFont_GBK16(12,109,BLACK,GRAY0,"温度：");
		DisplayButtonUp(98,105,217,125); 
		Gui_DrawFont_GBK16(100,107,BLACK,GRAY0,"温度阈值：26-30");


    while(1) 
    {   
			
			
		
        /*读取温度湿度信息*/
        SHT30_Read(0xe000);   
	
				show_number(50, 49, 240, 320, 16, BLACK,GRAY0, MLX90614_Read(0X5A, 0X07), 1);// 参数: x坐标, y坐标, 显示区域宽度, 显示区域高度, 字体大小, 前景色, 背景色, 数值, 小数位数									        
				show_number(50, 79, 240, 320, 16, BLACK,GRAY0, Humidity, 1);
				show_number(50, 109, 240, 320, 16, BLACK,GRAY0,Temperature , 1);	

				
				printf("AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\": \\\"123\\\"\\,\\\"version\\\": \\\"1.0\\\"\\,\\\"params\\\": {\\\"tempm\\\": {\\\"value\\\": %.2f}\\,\\\"temp\\\": {\\\"value\\\": %.2f}\\,\\\"humi\\\": {\\\"value\\\": %.2f}}}\",1,0\r\n",PublishTopic,MLX90614_Read(0X5A, 0X07),Temperature,Humidity);
 				printf("AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\": \\\"123\\\"\\,\\\"version\\\": \\\"1.0\\\"\\,\\\"params\\\": {\\\"eat\\\": {\\\"value\\\":%d}\\,\\\"sleep\\\": {\\\"value\\\":%d}\\,\\\"happy\\\": {\\\"value\\\":%d}\\,\\\"ruce\\\": {\\\"value\\\":%d}\\,\\\"water\\\": {\\\"value\\\":%d}}}\",1,0\r\n",PublishTopic,action1,action2,action3,action4,action5);

			    KeyNum = Key_GetNum();
					if(KeyNum == 1)
					{
					// 清上传状态
					action1 = 0;
					action2 = 0;
					action3 = 0;
					action4 = 0;
					action5 = 0;
					// 清接收缓存（防止旧数据触发）
					memset(g_recv_buff1, 0, g_recv_length1);
					g_recv_length1 = 0;
					g_recv_complete_flag1 = 0;
          u0_printf("abc: %d\r\n", action1);
						KeyNum = 0;
						
          }	
			
			if(g_recv_complete_flag1==1)					
			{
				g_recv_complete_flag1 = 0;  
				uint8_t K230 = g_recv_buff1[0];
			  u0_printf("recv: %d\r\n", K230); 

				
				switch(K230)
				{
					case 0:  break;		//A
					case 1: action1=1; break;		//B
					case 2: action2=2; break;		//C
					case 3: action3=3; break;		//D
					case 4: action4=4; break;		//E
					case 5:
    action5 = 5;

    // 保存当前初始位置
    OriginAngle1 = Angle1;
    OriginAngle2 = Angle2;
    OriginAngle3 = Angle3;
    OriginAngle4 = Angle4;
    OriginAngle5 = Angle5;

    // 第一步：前四个舵机到固定位置1，第五个保持不动
    TargetAngle1 = 200;
    TargetAngle2 = 141;
    TargetAngle3 = 279;
    TargetAngle4 = 150;
    TargetAngle5 = Angle5;

    servo_task_step = 1;
    auto_mode = 1;
    wait_count = 0;
    break;
				}
						memset(g_recv_buff1,0,g_recv_length1); 
						g_recv_length1 = 0;  
			}
					
				

			/*自动模式角度插值计算*/
				if(auto_mode == 1)
{
    delay_ms(20);

    // 舵机缓慢靠近目标角度
    if(Angle1 < TargetAngle1) Angle1 += step;
    if(Angle1 > TargetAngle1) Angle1 -= step;

    if(Angle2 < TargetAngle2) Angle2 += step;
    if(Angle2 > TargetAngle2) Angle2 -= step;

    if(Angle3 < TargetAngle3) Angle3 += step;
    if(Angle3 > TargetAngle3) Angle3 -= step;

    if(Angle4 < TargetAngle4) Angle4 += step;
    if(Angle4 > TargetAngle4) Angle4 -= step;

    if(Angle5 < TargetAngle5) Angle5 += step;
    if(Angle5 > TargetAngle5) Angle5 -= step;

    // 动作流程控制
    if(Servo_Arrived())
    {
        switch(servo_task_step)
        {
            case 1:
                // 第二步：第五个舵机到固定位置
                TargetAngle5 = 165;   // 这里改成你第五个舵机要到的位置
                servo_task_step = 2;
                break;

            case 2:
                // 第三步：前四个舵机到固定位置2
                TargetAngle1 = 250;
                TargetAngle2 = 155;
                TargetAngle3 = 210;
                TargetAngle4 = 215;
                TargetAngle5 = Angle5;
                servo_task_step = 3;
                break;

            case 3:
                // 第四步：等待几秒
                wait_count++;

                if(wait_count >= 100)   // 250 × 20ms = 4秒
                {
                    wait_count = 0;

                    // 第五步：前四个舵机返回固定位置1
                    TargetAngle1 = 200;
                    TargetAngle2 = 141;
                    TargetAngle3 = 279;
                    TargetAngle4 = 150;
                    TargetAngle5 = Angle5;

                    servo_task_step = 4;
                }
                break;

            case 4:
                // 第六步：第五个舵机恢复到原状态
                TargetAngle5 = OriginAngle5;
                servo_task_step = 5;
                break;

            case 5:
                // 第七步：前四个舵机恢复到原位置
                TargetAngle1 = OriginAngle1;
                TargetAngle2 = OriginAngle2;
                TargetAngle3 = OriginAngle3;
                TargetAngle4 = OriginAngle4;
                TargetAngle5 = OriginAngle5;

                servo_task_step = 6;
                break;

            case 6:
                // 全部动作完成
                auto_mode = 0;
                servo_task_step = 0;
                break;
        }
    }
}
							
			/*保持舵机控制*/
				Set_Servo_Angle1(Angle1);
				Set_Servo_Angle2(Angle2);	
				Set_Servo_Angle3(Angle3);	
				Set_Servo_Angle4(Angle4);
				Set_Servo_Angle5(Angle5);	



 
    }
}

void onenet_send_data(void)
{
	uint16_t num = 1;
	printf("AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\": \\\"123\\\"\\,\\\"version\\\": \\\"1.0\\\"\\,\\\"params\\\": {\\\"tempm\\\": {\\\"value\\\": %f}\\,\\\"temp\\\": {\\\"value\\\": %f }\\,\\\"humi\\\": {\\\"value\\\": %f}}}\",1,0\r\n",PublishTopic,MLX90614_Read(0X5A, 0X07),Temperature,Humidity);
}

