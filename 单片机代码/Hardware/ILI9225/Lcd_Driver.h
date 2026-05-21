
 /********************************************************************************
   * 测试硬件：立创·梁山派开发板GD32F470ZGT6    使用主频200Mhz    晶振25Mhz
   * 版 本 号: V1.0
   * 修改作者: LC
   * 修改日期: 2023年06月12日
   * 功能介绍:      
   ******************************************************************************
   * 梁山派软硬件资料与相关扩展板软硬件资料官网全部开源  
   * 开发板官网：www.lckfb.com   
   * 技术支持常驻论坛，任何技术问题欢迎随时交流学习  
   * 立创论坛：club.szlcsc.com   
   * 其余模块移植手册：【立创·梁山派开发板】模块移植手册
   * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
   * 不靠卖板赚钱，以培养中国工程师为己任
 *********************************************************************************/

#ifndef _LCD_DRIVER_H_
#define _LCD_DRIVER_H_

#include "gd32f4xx.h"
#ifndef u8
#define u8 uint8_t
#endif

#ifndef u16
#define u16 uint16_t
#endif

#ifndef u32
#define u32 uint32_t
#endif
//-----------------------------SPI 总线配置--------------------------------------//
#define USE_HARDWARE_SPI     1  //1:Enable Hardware SPI;0:USE Soft SPI

//-------------------------屏幕物理像素设置--------------------------------------//
#define LCD_X_SIZE	        176
#define LCD_Y_SIZE	        220

/////////////////////////////////////用户配置区///////////////////////////////////	 
//支持横竖屏快速定义切换
#define USE_HORIZONTAL  		1	//定义是否使用横屏 		0,不使用.1,使用.

#ifdef USE_HORIZONTAL//如果定义了横屏 
#define X_MAX_PIXEL	        LCD_Y_SIZE
#define Y_MAX_PIXEL	        LCD_X_SIZE
#else
#define X_MAX_PIXEL	        LCD_X_SIZE
#define Y_MAX_PIXEL	        LCD_Y_SIZE
#endif
//////////////////////////////////////////////////////////////////////////////////
	 

typedef struct  
{										    
	u16 width;			//LCD 宽度
	u16 height;			//LCD 高度
	u16 id;				//LCD ID
	u8  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	u16	 wramcmd;		//开始写gram指令
	u16  setxcmd;		//设置x坐标指令
	u16  setycmd;		//设置y坐标指令	 
}_lcd_dev; 




#define RED  	0xf800
#define GREEN	0x07e0
#define BLUE 	0x001f
#define WHITE	0xffff
#define BLACK	0x0000
#define YELLOW  0xFFE0
#define GRAY0   0xEF7D   	//灰色0 3165 00110 001011 00101
#define GRAY1   0x8410      	//灰色1      00000 000000 00000
#define GRAY2   0x4208      	//灰色2  1111111111011111



/******************************************************************************
接口定义在Lcd_Driver.h内定义，请根据接线修改并修改相应IO初始化LCD_GPIO_Init()
*******************************************************************************/
//-----------------LCD端口移植---------------- 
//VCC - 3.3V
//SCL - PB13 SPI1_SCK
//SDA - PB15 SPI1_MOSI
//RES - PD0(可以接入复位)
//DC  - PC6
//CS  - PB12        SPI1_NSS
//BLK - PC7- TIMER7_CH0
#define RCU_LCD_SCL     RCU_GPIOB//SCL=CLK=LCK
#define PORT_LCD_SCL    GPIOB
#define GPIO_LCD_SCL    GPIO_PIN_13

#define RCU_LCD_SDA     RCU_GPIOB//SDA=SDI
#define PORT_LCD_SDA    GPIOB
#define GPIO_LCD_SDA    GPIO_PIN_15

#define RCU_LCD_CS      RCU_GPIOB//NSS
#define PORT_LCD_CS     GPIOB
#define GPIO_LCD_CS     GPIO_PIN_12

#define RCU_LCD_DC      RCU_GPIOB //DC=RS
#define PORT_LCD_DC     GPIOB
#define GPIO_LCD_DC     GPIO_PIN_4

#define RCU_LCD_RES     RCU_GPIOD//RES=RST
#define PORT_LCD_RES    GPIOD
#define GPIO_LCD_RES    GPIO_PIN_0

#define RCU_LCD_BLK     RCU_GPIOD//BLK
#define PORT_LCD_BLK    GPIOD
#define GPIO_LCD_BLK    GPIO_PIN_3

#define RCU_SPI_HARDWARE RCU_SPI1
#define PORT_SPI         SPI1
#define LINE_AF_SPI      GPIO_AF_5



//#define LCD_CS_SET(x) LCD_CTRL->ODR=(LCD_CTRL->ODR&~LCD_CS)|(x ? LCD_CS:0)

//液晶控制口置1操作语句宏定义
#define	LCD_CS_SET  	gpio_bit_write(PORT_LCD_CS, GPIO_LCD_CS, SET)
#define	LCD_RS_SET  	gpio_bit_write(PORT_LCD_DC, GPIO_LCD_DC, SET) 
#define	LCD_SDA_SET  	gpio_bit_write(PORT_LCD_SDA, GPIO_LCD_SDA, SET)
#define	LCD_SCL_SET  	gpio_bit_write(PORT_LCD_SCL, GPIO_LCD_SCL, SET)
#define	LCD_RST_SET  	gpio_bit_write(PORT_LCD_RES, GPIO_LCD_RES, SET)
#define	LCD_LED_SET  	gpio_bit_write(PORT_LCD_BLK, GPIO_LCD_BLK, SET)

//液晶控制口置0操作语句宏定义
#define	LCD_CS_CLR  	gpio_bit_write(PORT_LCD_CS, GPIO_LCD_CS, RESET)//CS 
#define	LCD_RS_CLR  	gpio_bit_write(PORT_LCD_DC, GPIO_LCD_DC, RESET)//DC
#define	LCD_SDA_CLR  	gpio_bit_write(PORT_LCD_SDA, GPIO_LCD_SDA, RESET)//SDA=MOSI    
#define	LCD_SCL_CLR  	gpio_bit_write(PORT_LCD_SCL, GPIO_LCD_SCL, RESET)//SCL=SCLK
#define	LCD_RST_CLR  	gpio_bit_write(PORT_LCD_RES, GPIO_LCD_RES, RESET)//RES
#define	LCD_LED_CLR  	gpio_bit_write(PORT_LCD_BLK, GPIO_LCD_BLK, RESET)





void LCD_GPIO_Init(void);
void Lcd_WriteIndex(u8 Index);
void Lcd_WriteData(u8 Data);
void Lcd_WriteReg(u8 Index,u8 Data);
u16 Lcd_ReadReg(u8 LCD_Reg);
void Lcd_Reset(void);
void Lcd_Init(void);
void Lcd_Clear(u16 Color);
void Lcd_SetXY(u16 x,u16 y);
void Gui_DrawPoint(u16 x,u16 y,u16 Data);
unsigned int Lcd_ReadPoint(u16 x,u16 y);
void Lcd_SetRegion(u8 x_start,u8 y_start,u8 x_end,u8 y_end);
void Lcd_WriteData_16Bit(u16 Data);

#endif
