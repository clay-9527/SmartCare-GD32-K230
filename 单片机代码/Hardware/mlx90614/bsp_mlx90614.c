/********************************************************************************
  * 文 件 名: bsp_mlx90614.c
  * 版 本 号: 初版
  * 修改作者: LC
  * 修改日期: 2023年04月19日
  * 功能介绍:
  ******************************************************************************
  * 注意事项:
*********************************************************************************/

#include "bsp_mlx90614.h"
#include "bsp_usart.h"
#include "stdio.h"
#include "delay.h"


/******************************************************************
 * 函 数 名 称：MLX90614_GPIO_Init
 * 函 数 说 明：MLX90614的引脚初始化
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：MLX90614是5V，而且立创·梁山派开发板的引脚输出是3.3V，
 *             故设置引脚模式时，必须设置为开漏模式
******************************************************************/
void MLX90614_GPIO_Init(void)
{
  /* 使能时钟 */
    rcu_periph_clock_enable(RCU_SCL);
        rcu_periph_clock_enable(RCU_SDA);

        /* 配置SCL为输出模式 */
        gpio_mode_set(PORT_SCL,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLUP,GPIO_SCL);
        /* 配置为推挽输出 50MHZ */
        gpio_output_options_set(PORT_SCL,GPIO_OTYPE_OD,GPIO_OSPEED_50MHZ,GPIO_SCL);

        /* 配置SDA为输出模式 */
        gpio_mode_set(PORT_SDA,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLUP,GPIO_SDA);
        /* 配置为推挽输出 50MHZ */
        gpio_output_options_set(PORT_SDA,GPIO_OTYPE_OD,GPIO_OSPEED_50MHZ,GPIO_SDA);
}


/******************************************************************
 * 函 数 名 称：IIC_Start
 * 函 数 说 明：IIC起始时序
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
static void IIC_Start(void)
{
        SDA_OUT();

        SDA(1);
        delay_us(5);
        SCL(1);
        delay_us(5);

        SDA(0);
        delay_us(5);
        SCL(0);
        delay_us(5);

}
/******************************************************************
 * 函 数 名 称：IIC_Stop
 * 函 数 说 明：IIC停止信号
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
static void IIC_Stop(void)
{
        SDA_OUT();
        SCL(0);
        SDA(0);

        SCL(1);
        delay_us(5);
        SDA(1);
        delay_us(5);

}

/******************************************************************
 * 函 数 名 称：IIC_Send_Ack
 * 函 数 说 明：主机发送应答或者非应答信号
 * 函 数 形 参：0发送应答  1发送非应答
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
static void IIC_Send_Ack(unsigned char ack)
{
        SDA_OUT();
        SCL(0);
        SDA(0);
        delay_us(5);
        if(!ack) SDA(0);
        else         SDA(1);
        SCL(1);
        delay_us(5);
        SCL(0);
        SDA(1);
}


/******************************************************************
 * 函 数 名 称：I2C_WaitAck
 * 函 数 说 明：等待从机应答
 * 函 数 形 参：无
 * 函 数 返 回：0有应答  1超时无应答
 * 作       者：LC
 * 备       注：无
******************************************************************/
static unsigned char I2C_WaitAck(void)
{

        char ack = 0;
        unsigned char ack_flag = 10;
        SCL(0);
        SDA(1);
        SDA_IN();
        delay_us(5);
        SCL(1);
        delay_us(5);

        while( (SDA_GET()==1) && ( ack_flag ) )
        {
                ack_flag--;
                delay_us(5);
        }

        if( ack_flag <= 0 )
        {
                IIC_Stop();
                return 1;
        }
        else
        {
                SCL(0);
                SDA_OUT();
        }
        return ack;
}

/******************************************************************
 * 函 数 名 称：Send_Byte
 * 函 数 说 明：写入一个字节
 * 函 数 形 参：dat要写人的数据
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
static void Send_Byte(uint8_t dat)
{
        int i = 0;
        SDA_OUT();
        SCL(0);//拉低时钟开始数据传输

        for( i = 0; i < 8; i++ )
        {
                SDA( (dat & 0x80) >> 7 );
                __nop();
                SCL(1);
                delay_us(5);
                SCL(0);
                delay_us(5);
                dat<<=1;
        }
}

/******************************************************************
 * 函 数 名 称：Read_Byte
 * 函 数 说 明：IIC读时序
 * 函 数 形 参：无
 * 函 数 返 回：读到的数据
 * 作       者：LC
 * 备       注：无
******************************************************************/
static unsigned char Read_Byte(void)
{
        unsigned char i,receive=0;
        SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
        {
        SCL(0);
        delay_us(5);
        SCL(1);
        delay_us(5);
        receive<<=1;
        if( SDA_GET() )
        {
            receive|=1;
        }
        delay_us(5);
    }
        SCL(0);
  return receive;
}


/******************************************************************
 * 函 数 名 称：PEC_Calculation
 * 函 数 说 明：PEC校验
 * 函 数 形 参：pec要校验的数据地址   len校验的长度
 * 函 数 返 回：校验后的值
 * 作       者：LC
 * 备       注：无
******************************************************************/
static unsigned char PEC_Calculation(unsigned char *dat , unsigned char len)
{
    unsigned char i;
    unsigned char crc=0;
    while( len-- )
    {
        crc ^= *dat++;
        for( i=0 ; i<8 ; i++ )
        {
            if( crc&0x80 )
            {
                crc = (crc<<1)^0x07;
            }
            else
            {
                crc = (crc<<1);
            }
        }
    }
    return crc;
}

/************************************************************
 * 函数名称：MLX90615_Read
 * 函数说明：读取MLX90615的温度
 * 型    参：SlaveAddr = 器件地址  RegAddr = 要操作的寄存器地址
 * 返 回 值：温度值
 * 备    注：   SlaveAddr = 0X5A默认器件地址
 *              RegAddr   = 0X07读取被测量物体温度
 *              RegAddr   = 0X06读取环境温度
*************************************************************/
#define CRC_VERIFY_ENABLE 1
float MLX90614_Read(unsigned char SlaveAddr, unsigned char RegAddr)
{
        int i = 0;
        unsigned char buff[3]={0};        //保存温度高低位与校验码
        unsigned char arr[6]={0};        //校验数据使用
        uint16_t temp = 0;                        //高低位整合数据保存
        float T=0.0;                                //换算出的实际温度

        IIC_Start();
        Send_Byte((SlaveAddr<<1)|0);//写命令
        I2C_WaitAck();                 //等待响应


        Send_Byte(RegAddr);//写入要操作的寄存器地址
        I2C_WaitAck();

    do{
        delay_ms(1);
        IIC_Start();        //重新开始IIC
        Send_Byte((SlaveAddr<<1)|1);        //读命令
    }while( I2C_WaitAck() );

        buff[0] = Read_Byte();                        //保存温度数据的低8位
        IIC_Send_Ack(0);                                //主机发送应答
        buff[1] = Read_Byte();                        //保存温度数据的高8位
        IIC_Send_Ack(0);                                //主机发送应答
        buff[2] = Read_Byte();                        //保存校验码
        IIC_Send_Ack(1);                                //主机发送应答
        IIC_Stop();        //停止时序


//使用校验
#if CRC_VERIFY_ENABLE
    arr[0] = (SlaveAddr<<1);    //器件地址+写
    arr[1] = RegAddr;           //命令
    arr[2] = (SlaveAddr<<1)+1;  //器件地址+读
    arr[3] = buff[0];           //数据低8位
    arr[4] = buff[1];           //数据高8位

    if( PEC_Calculation(arr, 5) == buff[2] )//如果校验正确
    {
        temp = (short)(buff[1]<<8) | buff[0];//整合高低位
        T = (temp * 0.02) - 273.15 ;         //带入公式换算出实际温度
    }
    else
    {
        printf("ERROR CODE 4\r\n");
    }
#endif

//不使用校验
#if !CRC_VERIFY_ENABLE
    temp = (uint16_t)(buff[1]<<8) | buff[0];
    T = (temp*0.02)-273.15 ;
#endif

        return T+7;
}
