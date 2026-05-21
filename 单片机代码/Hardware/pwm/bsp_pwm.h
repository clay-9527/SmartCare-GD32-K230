#ifndef _BSP_PWM_H
#define _BSP_PWM_H

#include "gd32f4xx.h"
#include "delay.h"



void Servo_Init(void);
void Set_Servo_Angle1(unsigned int angle);
void Set_Servo_Angle2(unsigned int angle);
void Set_Servo_Angle3(unsigned int angle);
void Set_Servo_Angle4(unsigned int angle);
void Set_Servo_Angle5(unsigned int angle);



#define SERVO1_MIN 80
#define SERVO1_MAX 280
#define SERVO2_MIN 80
#define SERVO2_MAX 280
#define SERVO3_MIN 80
#define SERVO3_MAX 280
#define SERVO4_MIN 80
#define SERVO4_MAX 280
#define SERVO5_MIN 150
#define SERVO5_MAX 250









unsigned int Get_Servo_Angle(void);
void TIM3_PWM_Init_AllChannel(void);





#endif

