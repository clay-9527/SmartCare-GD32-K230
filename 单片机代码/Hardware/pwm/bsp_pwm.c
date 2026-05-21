#include "bsp_pwm.h"
#include "gd32f4xx.h"

/******************************************************************
 * 函 数 名 称：Servo_Init
 * 函 数 说 明：PWM配置
 * 函 数 形 参： pre定时器时钟预分频值    per周期
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：PWM频率=240 000 000 /( (pre+1) * (per+1) )
******************************************************************/

void Servo_Init(void)
{
        //定时器时钟
				rcu_periph_clock_enable(RCU_TIMER3);
        rcu_periph_clock_enable(RCU_TIMER2);        			    // 开启定时器时钟
        rcu_periph_clock_enable(RCU_GPIOB);										//引脚时钟
				rcu_periph_clock_enable(RCU_GPIOC);
        rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4); // 配置定时器时钟				
				
	      /* 配置AIN1*/
				gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6 | GPIO_PIN_7);
				gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
				gpio_af_set(GPIOB, GPIO_AF_2, GPIO_PIN_6 | GPIO_PIN_7);
	
        gpio_mode_set(GPIOC,GPIO_MODE_AF,GPIO_PUPD_NONE,GPIO_PIN_6| GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);
        gpio_output_options_set(GPIOC,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_6| GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);
        gpio_af_set(GPIOC,GPIO_AF_2,GPIO_PIN_6| GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);			
							
        // 复位定时器
        timer_deinit(TIMER3);	
        timer_deinit(TIMER2);

        /* 配置定时器参数 */
        // 频率f =系统时钟 / ( (prescaler+1) * (period+1) )
        // 频率f = 240,000,000/ (24000 * 200)  = 50hz
        // 周期T = 1/f = 1/50 = 0.02S = 20ms
				
        timer_parameter_struct timere_initpara;        					// 定义定时器结构体				
        timere_initpara.prescaler = 200-1;      							// 时钟预分频值
        timere_initpara.alignedmode = TIMER_COUNTER_EDGE;       // 边缘对齐
        timere_initpara.counterdirection = TIMER_COUNTER_UP;    // 向上计数
        timere_initpara.period = 200000-1;                         // 周期
        timere_initpara.clockdivision = TIMER_CKDIV_DIV1;       // 分频因子
        timere_initpara.repetitioncounter = 0;                  // 重复计数器 0-255
				timer_init(TIMER2,&timere_initpara);        						// 初始化定时器
				timer_init(TIMER3, &timere_initpara);
				
				
        timer_oc_parameter_struct timer_ocintpara;     //定时器比较输出结构体
        timer_ocintpara.ocpolarity = TIMER_OC_POLARITY_HIGH;         // 有效电平的极性
        timer_ocintpara.outputstate = TIMER_CCX_ENABLE;              // 配置比较输出模式状态 也就是使能PWM输出到端口
        timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;      // 通道互补输出极性为高电平
        timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;           // 通道互补输出状态失能
        timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;      // 信道输出的空闲状态为低
        timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;     // 信道互补输出的空闲状态为低
//				timer_channel_output_config(TIMER2,TIMER_CH_0,&timer_ocintpara); /* 配置定时器输出功能 */
				// 配置TIM1通道3和4
				timer_channel_output_config(TIMER3, TIMER_CH_0, &timer_ocintpara);
				timer_channel_output_config(TIMER3, TIMER_CH_1, &timer_ocintpara);
				
				// 配置TIM2通道1-4
				timer_channel_output_config(TIMER2, TIMER_CH_0, &timer_ocintpara);
				timer_channel_output_config(TIMER2, TIMER_CH_1, &timer_ocintpara);
				timer_channel_output_config(TIMER2, TIMER_CH_2, &timer_ocintpara);
				timer_channel_output_config(TIMER2, TIMER_CH_3, &timer_ocintpara);
			

        // 配置定时器通道输出比较模式
        timer_channel_output_mode_config(TIMER2,TIMER_CH_0,TIMER_OC_MODE_PWM1);
        timer_channel_output_mode_config(TIMER2,TIMER_CH_1,TIMER_OC_MODE_PWM1);				
        timer_channel_output_mode_config(TIMER2,TIMER_CH_2,TIMER_OC_MODE_PWM1);				
        timer_channel_output_mode_config(TIMER2,TIMER_CH_3,TIMER_OC_MODE_PWM1);	
				
        timer_channel_output_mode_config(TIMER3,TIMER_CH_0,TIMER_OC_MODE_PWM1);
        timer_channel_output_mode_config(TIMER3,TIMER_CH_1,TIMER_OC_MODE_PWM1);

        // 配置定时器通道输出影子寄存器
        timer_channel_output_shadow_config(TIMER2,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);
        timer_channel_output_shadow_config(TIMER2,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);				
        timer_channel_output_shadow_config(TIMER2,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);
        timer_channel_output_shadow_config(TIMER2,TIMER_CH_3,TIMER_OC_SHADOW_DISABLE);
				
        timer_channel_output_shadow_config(TIMER3,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);
        timer_channel_output_shadow_config(TIMER3,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);

        /* 只有高级定时器使用 */
        timer_auto_reload_shadow_enable(TIMER3);
        timer_auto_reload_shadow_enable(TIMER2);
        timer_primary_output_config(TIMER3, ENABLE);				
        timer_primary_output_config(TIMER2, ENABLE);

        /* 使能定时器 */
        timer_enable(TIMER3);				
        timer_enable(TIMER2);
}



/******************************************************************
 * 函 数 名 称：Set_Servo_Angle
 * 函 数 说 明：设置角度
 * 函 数 形 参：angle=要设置的角度，范围0-180
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void Set_Servo_Angle1(unsigned int angle)
{
        unsigned int ServoAngle1 = 0;	
        //保存设置的角度
        ServoAngle1 =500 + angle * (2000 / 270);	
				if(ServoAngle1 < 500) ServoAngle1 = 500;
				else if(ServoAngle1 > 2500) ServoAngle1 = 2500;	
        timer_channel_output_pulse_value_config(TIMER2,TIMER_CH_0,ServoAngle1);
}

void Set_Servo_Angle2(unsigned int angle)
{
        unsigned int ServoAngle2 = 0;	
        //保存设置的角度
        ServoAngle2 =500 + angle * (2000 / 270);	
				if(ServoAngle2 < 500) ServoAngle2 = 500;
				else if(ServoAngle2 > 2500) ServoAngle2 = 2500;	
        timer_channel_output_pulse_value_config(TIMER2,TIMER_CH_1,ServoAngle2);
}

void Set_Servo_Angle3(unsigned int angle)
{
        unsigned int ServoAngle3 = 0;	
        //保存设置的角度
        ServoAngle3 =500 + angle * (2000 / 270);	
				if(ServoAngle3 < 500) ServoAngle3 = 500;
				else if(ServoAngle3 > 2500) ServoAngle3 = 2500;	
        timer_channel_output_pulse_value_config(TIMER2,TIMER_CH_2,ServoAngle3);
}

void Set_Servo_Angle4(unsigned int angle)
{
        unsigned int ServoAngle4 = 0;	
        //保存设置的角度
        ServoAngle4 =500 + angle * (2000 / 270);	
				if(ServoAngle4 < 500) ServoAngle4 = 500;
				else if(ServoAngle4 > 2500) ServoAngle4 = 2500;	
        timer_channel_output_pulse_value_config(TIMER3,TIMER_CH_0,ServoAngle4);
}

void Set_Servo_Angle5(unsigned int angle)
{
        unsigned int ServoAngle5 = 0;	
        //保存设置的角度
        ServoAngle5 =500 + angle * (2000 / 270);	
				if(ServoAngle5 < 500) ServoAngle5 = 500;
				else if(ServoAngle5 > 2500) ServoAngle5 = 2500;	
        timer_channel_output_pulse_value_config(TIMER3,TIMER_CH_1,ServoAngle5);
}










