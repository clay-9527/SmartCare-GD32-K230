#include "esp01s.h"
#include "stdio.h"
#include "bsp_usart.h"


unsigned char WIFI_RX_BUFF[WIFI_RX_LEN_MAX];     
unsigned char WIFI_RX_FLAG = 0;
unsigned char WIFI_RX_LEN = 0;

/************************************************************
 * 函数名称：WIFI_USART_Init
 * 函数说明：串口1初始化
 * 型    参：bund=串口波特率
 * 返 回 值：无
 * 备    注：无
*************************************************************/
void WIFI_USART_Init(unsigned int bund)
{
	/* 使能 WIFI_USART 的时钟 */
	rcu_periph_clock_enable(RCU_WIFI_USART);
	/* 使能时钟 */
	rcu_periph_clock_enable(RCU_WIFI_TX);
	rcu_periph_clock_enable(RCU_WIFI_RX);
	/*	配置引脚为复用功能 */
	gpio_af_set(PORT_WIFI_TX, BSP_WIFI_AF, GPIO_WIFI_TX);
	
	/*	配置引脚为复用功能 */
	gpio_af_set(PORT_WIFI_RX, BSP_WIFI_AF, GPIO_WIFI_RX);
	
	/*	配置TX引脚为复用上拉模式 */
	gpio_mode_set(PORT_WIFI_TX, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_WIFI_TX);
	
	/*	配置RX引脚为复用上拉模式 */
	gpio_mode_set(PORT_WIFI_RX, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_WIFI_RX);
	
	/*	配置PA2引脚为为输出模式 */
	gpio_output_options_set(PORT_WIFI_TX, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_WIFI_TX);
	
	/*	配置PA3引脚为为输出模式 */
	gpio_output_options_set(PORT_WIFI_RX, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_WIFI_RX);
	
	/*	设置WIFI_USART的波特率为115200 */
	usart_baudrate_set(WIFI_USART, bund);
	
	/*	设置WIFI_USART的校验位为无 */
	usart_parity_config(WIFI_USART, USART_PM_NONE);
	
	/*	设置WIFI_USART的数据位为8位 */
	usart_word_length_set(WIFI_USART, USART_WL_8BIT);
	
	/*	设置WIFI_USART的停止位为1位 */
	usart_stop_bit_set(WIFI_USART, USART_STB_1BIT);
	
	/*	使能串口1 */
	usart_enable(WIFI_USART);
	
	/*	使能WIFI_USART传输 */
	usart_transmit_config(WIFI_USART, USART_TRANSMIT_ENABLE);
	
	/*	使能WIFI_USART接收 */
	usart_receive_config(WIFI_USART, USART_RECEIVE_ENABLE);
	
	/*	使能WIFI_USART接收中断标志位 */
	usart_interrupt_enable(WIFI_USART, USART_INT_RBNE);   
	
  /*	使能WIFI_USART空闲中断标志位 */
	usart_interrupt_enable(WIFI_USART, USART_INT_IDLE); // DLE 线检测中断

	/* 配置中断优先级 */
	nvic_irq_enable(WIFI_USART_IRQ, 2, 2); // 配置中断优先级
}
/* WIFI_USART发送单个字符 */
void WIFI_USART_Send_Bit(unsigned char ch)
{
	//发送字符
	usart_data_transmit(WIFI_USART, ch);
	// 等待发送数据缓冲区标志自动置位
	while(RESET == usart_flag_get(WIFI_USART, USART_FLAG_TBE) );
}  

/* WIFI_USART发送字符串 */
void WIFI_USART_send_String(unsigned char *str)
{
	while( str && *str ) // 地址为空或者值为空跳出
	{	
		WIFI_USART_Send_Bit(*str++);
	}	
}

/************************************************
函数名称 ： fputc
功    能 ： 串口重定向函数
参    数 ： 
返 回 值 ： 
作    者 ： LC
*************************************************/
int fputc(int ch, FILE *f)
{
     WIFI_USART_Send_Bit(ch);
     // 等待发送数据缓冲区标志置位
     return ch;
}

/******************************************************************
 * 函 数 名 称：WIFI_USART_IRQHandler
 * 函 数 说 明：连接WIFI的串口中断服务函数
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void WIFI_USART_IRQHandler(void)
{
	if(usart_interrupt_flag_get(WIFI_USART,USART_INT_FLAG_RBNE) != RESET) // 接收缓冲区不为空
	{
        //接收数据
		WIFI_RX_BUFF[ WIFI_RX_LEN ] = usart_data_receive(WIFI_USART);
        
		//接收长度限制
        WIFI_RX_LEN = ( WIFI_RX_LEN + 1 ) % WIFI_RX_LEN_MAX;
	}
	if(usart_interrupt_flag_get(WIFI_USART,USART_INT_FLAG_IDLE) == SET) // 检测到空闲中断
	{
		usart_data_receive(WIFI_USART); // 必须要读，读出来的值不能要
		WIFI_RX_BUFF[WIFI_RX_LEN] = '\0'; //字符串结尾补 '\0'
		WIFI_RX_FLAG = SET;            // 接收完成
	}
}

//清除串口接收的数据
/******************************************************************
 * 函 数 名 称：Clear_WIFI_RX_BUFF
 * 函 数 说 明：清除WIFI发过来的数据
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void Clear_WIFI_RX_BUFF(void)
{
	unsigned char i = WIFI_RX_LEN_MAX - 1;
	while(i)        
	{
		WIFI_RX_BUFF[i--] = 0;
	}
	WIFI_RX_LEN = 0;
	WIFI_RX_FLAG = 0;
}

/******************************************************************
 * 函 数 名 称：WIFI_Send_Cmd
 * 函 数 说 明：向WIFI模块发送指令，并查看WIFI模块是否返回想要的数据
 * 函 数 形 参：cmd=发送的AT指令	ack=想要的应答		waitms=等待应答的时间		cnt=等待应答多少次
 * 函 数 返 回：1=得到了想要的应答		0=没有得到想要的应答
 * 作       者：LC
 * 备       注：无
******************************************************************/
char WIFI_Send_Cmd(char *cmd,char *ack,unsigned int waitms,unsigned char cnt)
{	
	WIFI_USART_send_String((unsigned char*)cmd);//1.发送AT指令
	while(cnt--)
	{
        //时间间隔
		delay_1ms(waitms);
		//串口中断接收wifi应答
		if( WIFI_RX_FLAG == 1 )
		{
			WIFI_RX_FLAG = 0;
			WIFI_RX_LEN = 0;
            //查找是否有想要的数据
			if( strstr((char*)WIFI_RX_BUFF, ack) != NULL )
			{
				return 1;
			}
            //清除接收的数据
			memset( WIFI_RX_BUFF, 0, sizeof(WIFI_RX_BUFF) );
		}
	}
	WIFI_RX_FLAG = 0;
	WIFI_RX_LEN = 0;
	return 0;
}

/******************************************************************
 * 函 数 名 称：WIFI_ESP01S_Init
 * 函 数 说 明：WIFI模块ESP01S初始化
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：ESP01S的默认波特率是115200
******************************************************************/
void WIFI_ESP01S_Init(void)
{
	WIFI_USART_Init(115200);//默认波特率为115200
}

/******************************************************************
 * 函 数 名 称：WIFI_MODE_STA_Aliyun_Init
 * 函 数 说 明：配置WIFI模块连接阿里云物联网平台
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void WIFI_MODE_STA_Aliyun_Init(void)
{
	char AT_CMD[250]={0};
	
	RST:
    //测试指令AT
	if(WIFI_Send_Cmd("AT\r\n","OK",100,3)==0)
    {
//		u0_printf("AT指令发送失败，复位重启\r\n");
        //wifi连接不上，重启
		WIFI_Send_Cmd("AT+RST\r\n","ready",1000,1);  
		goto RST;
	}
    else
//     u0_printf("测试指令AT发送成功\r\n");
	//配置WIFI STA
	if(WIFI_Send_Cmd("AT+CWMODE=1\r\n","OK",300,3)==0)
    {
//		u0_printf("WIFI模式配置失败,复位重启\r\n");
        //wifi连接不上，重启
		WIFI_Send_Cmd("AT+RST\r\n","ready",1000,1);  
		goto RST;
	}
    else
//       u0_printf("WiFi模式配置成功\r\n");
	//设置时区  NTSP服务器  用于调整客户端自身所在系统的时间，达到同步时间的目的	
//	if(WIFI_Send_Cmd("AT+CIPSNTPCFG=1,8,\"ntp1.alliyun.com\"\r\n","OK",100,3)==0)
//    {
//		u0_printf("连接NTSP服务器失败,复位重启\r\n");
//        //wifi连接不上，重启
//		WIFI_Send_Cmd("AT+RST\r\n","ready",1000,1);  
//		goto RST;
//	}
//    else
//        u0_printf("连接NTSP服务器成功\r\n");
    
	//连接wifi 账号&密码
	sprintf(AT_CMD,"AT+CWJAP=\"%s\",\"%s\"\r\n",WIFISSID,WIFIPASS);
    
	if( WIFI_Send_Cmd(AT_CMD,"OK",3000,3) == 0 )
	{
//		u0_printf("WIFI名称或密码有错,复位重启\r\n");
        //wifi连接不上，重启
		WIFI_Send_Cmd("AT+RST\r\n","ready",1000,1);  
		goto RST;
	}
    else
//        u0_printf("WiFi连接成功\r\n");
//    CON:
	//清0数组，备用
	memset(AT_CMD,0,sizeof(AT_CMD));  
	sprintf(AT_CMD,"AT+MQTTUSERCFG=0,1,\"test01\",\"%s\",\"%s\",0,0,\"\"\r\n", UserName, PassWord);
	if(WIFI_Send_Cmd(AT_CMD,"OK",2000,3) == 0)
    {
//       u0_printf("UserName,PassWord配置错误，软件复位重连\r\n");
		WIFI_Send_Cmd("AT+RST\r\n","ready",1000,2);  //wifi连接不上，重启  1000延时1S    2链接次数
		__set_FAULTMASK(1); //STM32程序软件复位
		NVIC_SystemReset();
    }
    else
//       u0_printf("MQTT用户配置成功\r\n");
	
	//设置连接客户端ID
	memset(AT_CMD,0,sizeof(AT_CMD));  //清0数组，备用
	sprintf(AT_CMD,"AT+MQTTCLIENTID=0,\"%s\"\r\n",ClientId);
	if(WIFI_Send_Cmd(AT_CMD,"OK",1000,3)==0)
    {
//      u0_printf("ClientId 配置错误，软件复位重连\r\n");
		WIFI_Send_Cmd("AT+RST\r\n","ready",1000,2);  //wifi连接不上，重启  1000延时1S    2链接次数
		__set_FAULTMASK(1); //STM32程序软件复位
		NVIC_SystemReset();
    }
    else
//     u0_printf("ClientID配置成功\r\n");
	
	//连接到MQTT代理（阿里云平台）
	memset(AT_CMD,0,sizeof(AT_CMD));
	sprintf(AT_CMD,"AT+MQTTCONN=0,\"%s\",1883,1\r\n",URL);
	if(WIFI_Send_Cmd(AT_CMD,"OK",2000,3)==0)
	{
//		u0_printf("连接aliyu失败,复位GD32重连\r\n");
		WIFI_Send_Cmd("AT+RST\r\n","ready",1000,2);  //wifi连接不上，重启  1000延时1S    2链接次数
		__set_FAULTMASK(1); //STM32程序软件复位
		NVIC_SystemReset();
	}
	else
//       u0_printf("连接阿里云成功\r\n");
	//订阅主题
	memset(AT_CMD,0,sizeof(AT_CMD));
	sprintf(AT_CMD, "AT+MQTTSUB=0,\"%s\",1\r\n", SubscribeTopic);
	WIFI_Send_Cmd(AT_CMD,"OK",1000,3);
//	u0_printf("订阅主题消息成功\r\n");
	Clear_WIFI_RX_BUFF();//清除串口接收缓存     
}

/*云平台下发数据*/

/*点击LED开关

+MQTTSUBRECV:0,"/sys/a1PJRLOWo3p/TEST/thing/service/property/set",100,{"method":"thing.service.property.set","id":"367399823","params":{"LED_Switch":1},"version":"1.0.0"}
*/

/*滑动亮度条
+MQTTSUBRECV:0,"/sys/a1PJRLOWo3p/TEST/thing/service/property/set",101,{"method":"thing.service.property.set","id":"812539841","params":{"brightness":75},"version":"1.0.0"}
*/
void Get_Aliyun_json_data(JSON_PUBLISH *data)//接收数据拆解
{
	char *buff=0;
    uint8_t timenum=0;
    //串口中断接收WIFI应答
    if( WIFI_RX_FLAG == SET )
    {
		//u0_printf("\r\n接收中--\r\n");
        WIFI_RX_FLAG = 0;
        WIFI_RX_LEN = 0;
        
        
        //有设备连接了热点
        if( strstr((char*)WIFI_RX_BUFF, "params\":") != NULL )
        {
			//获取功能名称
          //  u0_printf("%s",WIFI_RX_BUFF);
         //   u0_printf("解析json数据\r\n");
            
			buff = strstr((char*)WIFI_RX_BUFF, "params\":");
			buff += strlen("params\":{\"");
			strcpy(data->keyname,strtok(buff,"\""));
			//u0_printf("data->keyname = %s\r\n",data->keyname);
            
			//获取功能值
			buff = strstr((char*)WIFI_RX_BUFF, "params\":" );
			buff += strlen("params\":{\"")+strlen(data->keyname)+2;
			strcpy(data->value, strtok(buff,","));
           // u0_printf("data->value = %s\r\n",data->value);
            
            //获取功能名称
			buff = strstr((char*)WIFI_RX_BUFF, "params\":" );
			timenum=strlen("params\":{\"")+strlen(data->keyname)+2+strlen(data->value)+2;
            buff += timenum;
			strcpy(data->keyname, strtok(buff,"\""));
           // u0_printf("data->keyname = %s\r\n",data->keyname);
            
			//获取功能值
			buff = strstr((char*)WIFI_RX_BUFF, "params\":" );
            buff +=timenum+strlen(data->keyname)+2;
			strcpy(data->value, strtok(buff,"}"));
           // u0_printf("data->value = %s\r\n",data->value);
		}
	}
}

//清除JSON数据
void Clear_Aliyun_json_data(JSON_PUBLISH *data)
{
	uint16_t i = 0;
	while( data->keyname[i] != 0 )
	{
		data->keyname[i++] = '\0';
	}
	i= 0;
	while( data->value[i] != 0 )
	{
		data->keyname[i++] = '\0';
	}
}           

//发布主题 ，上发多个数据
void Publish_MQTT_message(JSON_PUBLISH *data,uint8_t data_num,uint8_t LED_Switch,uint8_t brightness,uint8_t powerstate)  
{
	char AT_CMD[384]={0};
	char params[256]={0},i,*sp;
//	int len=31;
	
	sp=params;
	
	sprintf(data[data_num-3].value,"%d",LED_Switch);   //把传感器的值赋值给json结构体的value
	sprintf(data[data_num-2].value,"%d",brightness);
	sprintf(data[data_num-1].value,"%d",powerstate);
	
	//          4
	for(i=0;i<data_num;i++)
	{        // 3
		if(i<(data_num-1))
		{   
			sprintf(sp,"%s%s%s",data[i].keyname,data[i].value,"\\,");
			while(*sp!=0){
                sp++;
            } //防止覆盖
		}
		else
			sprintf(sp,"%s%s",data[i].keyname,data[i].value);
	}
	sprintf(AT_CMD,"AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{%s}}\",1,0\r\n",PublishTopic,params);
	if(WIFI_Send_Cmd(AT_CMD,"OK",1000,1)==1){
       // u0_printf("发布主题消息成功\r\n");
    }
    else{
       // u0_printf("发布主题消息失败\r\n");
    }    
}

//发布主题 ，上发1个数据
/**
keyname:结构体成员位号;
keyvalue:对应数值
**/
void Publish_MQTT_Message(char *name,float val)  
{
    char *evalute,high[]="high",middle[]="middle",low[]="low";
	char AT_CMD[384]={0};
	char params[256]={0},*sp;
    JSON_PUBLISH json_data={0};
    JSON_PUBLISH *p=&json_data;

	sp=params;
    
	sprintf(p->keyname,"\\\"%s\\\":",name);
	sprintf(p->value,"%.2f",val);   //把传感器的值赋值给json结构体的value

    sprintf(sp,"%s%s",p->keyname,p->value);

    if(val>=12)
        evalute=high;
    else if(val<=9)
        evalute=low;
    else
        evalute=middle;
    
    
	sprintf(AT_CMD,"AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{%s\\,\\\"collect\\\":false}}\",1,0\r\n",PublishTopic,params);
    if(WIFI_Send_Cmd(AT_CMD,"OK",1000,1)==1){
       // u0_printf("发布主题消息成功\r\n");
    }
    else{
      //  u0_printf("发布主题消息失败\r\n");
    }    
//    sprintf(AT_CMD,"AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{%s\\,\\\"collect\\\":0\\,\\\"evaluate\\\":\\\"%s\\\"}}\",1,0\r\n",PublishMessageTopPost,params,evalute);
//	if(WIFI_Send_Cmd(AT_CMD,"OK",1000,1)==1){
//        u0_printf("发布主题消息成功\r\n");
//    }
//    else{
//        u0_printf("发布主题消息失败\r\n");
//    }    
}

