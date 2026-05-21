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

#include "gd32f4xx.h"
#include "Lcd_Driver.h"
#include "delay.h"
//#define  delay_ms  delay_1ms





//---------------------------------function----------------------------------------------------//

/****************************************************************************
* 名    称：void LCD_GPIO_Init(void)
* 功    能：模拟SPI所用到的GPIO初始化
* 入口参数：无
* 出口参数：无
* 说    明：初始化模拟SPI所用的GPIO
****************************************************************************/
void LCD_GPIO_Init(void)
{
          /* 使能时钟 */      
        rcu_periph_clock_enable(RCU_LCD_SCL);
        rcu_periph_clock_enable(RCU_LCD_SDA);
        rcu_periph_clock_enable(RCU_LCD_CS);
        rcu_periph_clock_enable(RCU_LCD_DC);
        rcu_periph_clock_enable(RCU_LCD_RES);
        rcu_periph_clock_enable(RCU_LCD_BLK);
        
        
                 /* 配置SCL */
        gpio_mode_set(PORT_LCD_SCL,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLUP,GPIO_LCD_SCL);
        gpio_output_options_set(PORT_LCD_SCL,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_LCD_SCL);
        gpio_bit_write(PORT_LCD_SCL, GPIO_LCD_SCL, SET);
        
                 /* 配置SDA */
        gpio_mode_set(PORT_LCD_SDA,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLUP,GPIO_LCD_SDA);
        gpio_output_options_set(PORT_LCD_SDA,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_LCD_SDA);
        gpio_bit_write(PORT_LCD_SDA, GPIO_LCD_SDA, SET);
        
                 /* 配置DC */
        gpio_mode_set(PORT_LCD_DC,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLUP,GPIO_LCD_DC);
        gpio_output_options_set(PORT_LCD_DC,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_LCD_DC);
        gpio_bit_write(PORT_LCD_DC, GPIO_LCD_DC, SET);

                 /* 配置CS */
        gpio_mode_set(PORT_LCD_CS,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLUP,GPIO_LCD_CS);
        gpio_output_options_set(PORT_LCD_CS,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_LCD_CS);
        gpio_bit_write(PORT_LCD_CS, GPIO_LCD_CS, SET);
        
                 /* 配置RES */
        gpio_mode_set(PORT_LCD_RES,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLUP,GPIO_LCD_RES);
        gpio_output_options_set(PORT_LCD_RES,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_LCD_RES);
        gpio_bit_write(PORT_LCD_RES, GPIO_LCD_RES, SET);

                 /* 配置BLK */
        gpio_mode_set(PORT_LCD_BLK, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_LCD_BLK);
        gpio_output_options_set(PORT_LCD_BLK, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_LCD_BLK);
        gpio_bit_write(PORT_LCD_BLK, GPIO_LCD_BLK, SET);

}

/****************************************************************************
* 名    称：void  SPIv_WriteData(u8 Data)
* 功    能：模拟SPI写一个字节数据底层函数
* 入口参数：Data
* 出口参数：无
* 说    明：模拟SPI读写一个字节数据底层函数
****************************************************************************/
void  SPIv_WriteData(u8 Data)
{
	unsigned char i=0;
	for(i=8;i>0;i--)
	{
		if(Data&0x80)	
	  LCD_SDA_SET; //输出数据
      else LCD_SDA_CLR;
	   
      LCD_SCL_CLR;       
      LCD_SCL_SET;
      Data<<=1; 
	}
}

/****************************************************************************
* 名    称：u8 SPI_WriteByte(u8 Byte)
* 功    能：硬件SPI读写一个字节数据底层函数
* 入口参数：SPIx,Byte
* 出口参数：返回总线收到的数据
* 说    明：硬件SPI读写一个字节数据底层函数
****************************************************************************/
u8 SPI_WriteByte(u8 Byte)
{
    while(RESET == spi_i2s_flag_get(PORT_SPI, SPI_FLAG_TBE));
    spi_i2s_data_transmit(PORT_SPI, Byte);
    while(RESET == spi_i2s_flag_get(PORT_SPI, SPI_FLAG_RBNE));
    spi_i2s_data_receive(PORT_SPI);
} 

/****************************************************************************
* 名    称：SPI2_Init(void)
* 功    能：SPI2硬件配置初始化
* 入口参数：无
* 出口参数：无
* 说    明：SPI2硬件配置初始化
****************************************************************************/
void SPI2_Init(void)	
{
    spi_parameter_struct spi_init_struct;
      /* 开启各引脚时钟 */
    rcu_periph_clock_enable(RCU_LCD_SCL);
    rcu_periph_clock_enable(RCU_LCD_SDA);
    rcu_periph_clock_enable(RCU_LCD_CS);
    rcu_periph_clock_enable(RCU_LCD_DC);
    rcu_periph_clock_enable(RCU_LCD_RES);
    rcu_periph_clock_enable(RCU_LCD_BLK);
      /* 使能SPI */
    rcu_periph_clock_enable(RCU_SPI_HARDWARE); 

      /* 配置 SPI的SCK GPIO */
    gpio_af_set(PORT_LCD_SCL, LINE_AF_SPI, GPIO_LCD_SCL);
    gpio_mode_set(PORT_LCD_SCL, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_LCD_SCL);
    gpio_output_options_set(PORT_LCD_SCL, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_LCD_SCL);
    gpio_bit_set(PORT_LCD_SCL,GPIO_LCD_SCL);
      /* 配置 SPI的MOSI  GPIO */
    gpio_af_set(PORT_LCD_SDA, LINE_AF_SPI, GPIO_LCD_SDA);
    gpio_mode_set(PORT_LCD_SDA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_LCD_SDA);
    gpio_output_options_set(PORT_LCD_SDA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_LCD_SDA);
    gpio_bit_set(PORT_LCD_SDA, GPIO_LCD_SDA);         

       /* 配置DC */
    gpio_mode_set(PORT_LCD_DC,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLUP,GPIO_LCD_DC);
    gpio_output_options_set(PORT_LCD_DC,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_LCD_DC);
    gpio_bit_write(PORT_LCD_DC, GPIO_LCD_DC, SET);
       /* 配置RES */
    gpio_mode_set(PORT_LCD_RES,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLUP,GPIO_LCD_RES);
    gpio_output_options_set(PORT_LCD_RES,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_LCD_RES);
    gpio_bit_write(PORT_LCD_RES, GPIO_LCD_RES, SET);
       /* 配置BLK */
    gpio_mode_set(PORT_LCD_BLK, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_LCD_BLK);
    gpio_output_options_set(PORT_LCD_BLK, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_LCD_BLK);
    gpio_bit_write(PORT_LCD_BLK, GPIO_LCD_BLK, SET);
       /* 配置CS */
    gpio_mode_set(PORT_LCD_CS,GPIO_MODE_OUTPUT,GPIO_PUPD_NONE,GPIO_LCD_CS);
    gpio_output_options_set(PORT_LCD_CS,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_LCD_CS);
    gpio_bit_write(PORT_LCD_CS, GPIO_LCD_CS, SET);

    /* 配置 SPI 参数 */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;  // 传输模式全双工
    spi_init_struct.device_mode          = SPI_MASTER;   // 配置为主机
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT; // 8位数据
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;  // 软件cs
    spi_init_struct.prescale             = SPI_PSC_2;//2分频
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(PORT_SPI, &spi_init_struct);

    /* 使能 SPI */
    spi_enable(PORT_SPI);
}

/****************************************************************************
* 名    称：Lcd_WriteIndex(u8 Index)
* 功    能：向液晶屏写一个8位指令
* 入口参数：Index   寄存器地址
* 出口参数：无
* 说    明：调用前需先选中控制器，内部函数
****************************************************************************/
void Lcd_WriteIndex(u8 Index)
{
   LCD_CS_CLR;
   LCD_RS_CLR;
#if USE_HARDWARE_SPI   
   SPI_WriteByte(Index);
#else
   SPIv_WriteData(Index);
#endif 
   LCD_CS_SET;
}

/****************************************************************************
* 名    称：Lcd_WriteData(u8 Data)
* 功    能：向液晶屏写一个8位数据
* 入口参数：dat     寄存器数据
* 出口参数：无
* 说    明：向控制器指定地址写入数据，内部函数
****************************************************************************/
void Lcd_WriteData(u8 Data)
{
   LCD_CS_CLR;
   LCD_RS_SET;
#if USE_HARDWARE_SPI   
   SPI_WriteByte(Data);
#else
   SPIv_WriteData(Data);
#endif 
   LCD_CS_SET;
}

/****************************************************************************
* 名    称：void LCD_WriteReg(u8 Index,u16 Data)
* 功    能：写寄存器数据
* 入口参数：Index,Data
* 出口参数：无
* 说    明：本函数为组合函数，向Index地址的寄存器写入Data值
****************************************************************************/
void LCD_WriteReg(u8 Index,u16 Data)
{
	Lcd_WriteIndex(Index);
  	Lcd_WriteData_16Bit(Data);
}

/****************************************************************************
* 名    称：void Lcd_WriteData_16Bit(u16 Data)
* 功    能：向液晶屏写一个16位数据
* 入口参数：Data
* 出口参数：无
* 说    明：向控制器指定地址写入一个16位数据
****************************************************************************/
void Lcd_WriteData_16Bit(u16 Data)
{	
	Lcd_WriteData(Data>>8);
	Lcd_WriteData(Data);	
}

/****************************************************************************
* 名    称：void Lcd_Reset(void)
* 功    能：液晶硬复位函数
* 入口参数：无
* 出口参数：无
* 说    明：液晶初始化前需执行一次复位操作
****************************************************************************/
void Lcd_Reset(void)
{
	LCD_RST_CLR;
	delay_ms(100);
	LCD_RST_SET;
	delay_ms(50);
}
/****************************************************************************
* 名    称：void Lcd_Init(void)
* 功    能：液晶初始化函数
* 入口参数：无
* 出口参数：无
* 说    明：液晶初始化_ILI9225_176X220
****************************************************************************/
void Lcd_Init(void)
{	
#if USE_HARDWARE_SPI //使用硬件SPI
	SPI2_Init();
#else	
	LCD_GPIO_Init();//使用模拟SPI
#endif
	Lcd_Reset(); //Reset before LCD Init.

	//LCD Init For 2.2inch LCD Panel with ILI9225.	
	LCD_WriteReg(0x10, 0x0000); // Set SAP,DSTB,STB
	LCD_WriteReg(0x11, 0x0000); // Set APON,PON,AON,VCI1EN,VC
	LCD_WriteReg(0x12, 0x0000); // Set BT,DC1,DC2,DC3
	LCD_WriteReg(0x13, 0x0000); // Set GVDD
	LCD_WriteReg(0x14, 0x0000); // Set VCOMH/VCOML voltage
	delay_ms(40); // Delay 20 ms
	
	// Please follow this power on sequence
	LCD_WriteReg(0x11, 0x0018); // Set APON,PON,AON,VCI1EN,VC
	LCD_WriteReg(0x12, 0x1121); // Set BT,DC1,DC2,DC3
	LCD_WriteReg(0x13, 0x0063); // Set GVDD
	LCD_WriteReg(0x14, 0x3961); // Set VCOMH/VCOML voltage
	LCD_WriteReg(0x10, 0x0800); // Set SAP,DSTB,STB
	delay_ms(10); // Delay 10 ms
	LCD_WriteReg(0x11, 0x1038); // Set APON,PON,AON,VCI1EN,VC
	delay_ms(30); // Delay 30 ms
	
	
	LCD_WriteReg(0x02, 0x0100); // set 1 line inversion

#if USE_HORIZONTAL//如果定义了横屏
	//R01H:SM=0,GS=0,SS=0 (for details,See the datasheet of ILI9225)
	LCD_WriteReg(0x01, 0x001C); // set the display line number and display direction
	//R03H:BGR=1,ID0=1,ID1=1,AM=1 (for details,See the datasheet of ILI9225)
	LCD_WriteReg(0x03, 0x1038); // set GRAM write direction .
#else//竖屏
	//R01H:SM=0,GS=0,SS=1 (for details,See the datasheet of ILI9225)
	LCD_WriteReg(0x01, 0x011C); // set the display line number and display direction 
	//R03H:BGR=1,ID0=1,ID1=1,AM=0 (for details,See the datasheet of ILI9225)
	LCD_WriteReg(0x03, 0x1030); // set GRAM write direction.
#endif

	LCD_WriteReg(0x07, 0x0000); // Display off
	LCD_WriteReg(0x08, 0x0808); // set the back porch and front porch
	LCD_WriteReg(0x0B, 0x1100); // set the clocks number per line
	LCD_WriteReg(0x0C, 0x0000); // CPU interface
	LCD_WriteReg(0x0F, 0x0501); // Set Osc
	LCD_WriteReg(0x15, 0x0020); // Set VCI recycling
	LCD_WriteReg(0x20, 0x0000); // RAM Address
	LCD_WriteReg(0x21, 0x0000); // RAM Address
	
	//------------------------ Set GRAM area --------------------------------//
	LCD_WriteReg(0x30, 0x0000); 
	LCD_WriteReg(0x31, 0x00DB); 
	LCD_WriteReg(0x32, 0x0000); 
	LCD_WriteReg(0x33, 0x0000); 
	LCD_WriteReg(0x34, 0x00DB); 
	LCD_WriteReg(0x35, 0x0000); 
	LCD_WriteReg(0x36, 0x00AF); 
	LCD_WriteReg(0x37, 0x0000); 
	LCD_WriteReg(0x38, 0x00DB); 
	LCD_WriteReg(0x39, 0x0000); 
	
	
	// ---------- Adjust the Gamma 2.2 Curve -------------------//
	LCD_WriteReg(0x50, 0x0603); 
	LCD_WriteReg(0x51, 0x080D); 
	LCD_WriteReg(0x52, 0x0D0C); 
	LCD_WriteReg(0x53, 0x0205); 
	LCD_WriteReg(0x54, 0x040A); 
	LCD_WriteReg(0x55, 0x0703); 
	LCD_WriteReg(0x56, 0x0300); 
	LCD_WriteReg(0x57, 0x0400); 
	LCD_WriteReg(0x58, 0x0B00); 
	LCD_WriteReg(0x59, 0x0017); 
	
	
	
	LCD_WriteReg(0x0F, 0x0701); // Vertical RAM Address Position
	LCD_WriteReg(0x07, 0x0012); // Vertical RAM Address Position
	delay_ms(50); // Delay 50 ms
	LCD_WriteReg(0x07, 0x1017); // Vertical RAM Address Position  
	
}



/*************************************************
函数名：LCD_Set_XY
功能：设置lcd显示起始点
入口参数：xy坐标
返回值：无
*************************************************/
void Lcd_SetXY(u16 Xpos, u16 Ypos)
{	
#if USE_HORIZONTAL//如果定义了横屏  	    	
	LCD_WriteReg(0x21,Xpos);
	LCD_WriteReg(0x20,Ypos);
#else//竖屏	
	LCD_WriteReg(0x20,Xpos);
	LCD_WriteReg(0x21,Ypos);
#endif
	Lcd_WriteIndex(0x22);		
} 
/*************************************************
函数名：LCD_Set_Region
功能：设置lcd显示区域，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
*************************************************/
//设置显示窗口
void Lcd_SetRegion(u8 xStar, u8 yStar,u8 xEnd,u8 yEnd)
{
#if USE_HORIZONTAL//如果定义了横屏	
	LCD_WriteReg(0x38,xEnd);
	LCD_WriteReg(0x39,xStar);
	LCD_WriteReg(0x36,yEnd);
	LCD_WriteReg(0x37,yStar);
	LCD_WriteReg(0x21,xStar);
	LCD_WriteReg(0x20,yStar);
#else//竖屏	
	LCD_WriteReg(0x36,xEnd);
	LCD_WriteReg(0x37,xStar);
	LCD_WriteReg(0x38,yEnd);
	LCD_WriteReg(0x39,yStar);
	LCD_WriteReg(0x20,xStar);
	LCD_WriteReg(0x21,yStar);
#endif
	Lcd_WriteIndex(0x22);	
}

	
/*************************************************
函数名：LCD_DrawPoint
功能：画一个点
入口参数：xy坐标和颜色数据
返回值：无
*************************************************/
void Gui_DrawPoint(u16 x,u16 y,u16 Data)
{
	Lcd_SetXY(x,y);
	Lcd_WriteData_16Bit(Data);

}    

/*************************************************
函数名：Lcd_Clear
功能：全屏清屏函数
入口参数：填充颜色COLOR
返回值：无
*************************************************/
void Lcd_Clear(u16 Color)               
{	
   unsigned int i,m;
   Lcd_SetRegion(0,0,X_MAX_PIXEL-1,Y_MAX_PIXEL-1);
   for(i=0;i<X_MAX_PIXEL;i++)
    for(m=0;m<Y_MAX_PIXEL;m++)
    {	
	  	Lcd_WriteData_16Bit(Color);
    }   
}

