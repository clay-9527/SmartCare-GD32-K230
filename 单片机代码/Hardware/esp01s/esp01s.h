#ifndef _ESP01S_H
#define _ESP01S_H

#include "gd32f4xx.h"
#include "systick.h"
#include "string.h"

/****************************   串口配置    ****************************/
#define RCU_WIFI_TX         RCU_GPIOD   // 串口TX的端口时钟
#define RCU_WIFI_RX         RCU_GPIOD  // 串口RX的端口时钟
#define RCU_WIFI_USART      RCU_USART1  // 串口1的时钟

#define PORT_WIFI_TX        GPIOD		// 串口TX的端口
#define PORT_WIFI_RX        GPIOD		// 串口RX的端口
#define GPIO_WIFI_TX        GPIO_PIN_5  // 串口TX的引脚
#define GPIO_WIFI_RX        GPIO_PIN_6  // 串口RX的引脚
#define BSP_WIFI_AF 		GPIO_AF_7   // 串口1的复用功能

#define WIFI_USART 			     USART1      		// 串口1
#define WIFI_USART_IRQ     	     USART1_IRQn 		// 串口1中断
#define WIFI_USART_IRQHandler    USART1_IRQHandler	// 串口1中断服务函数

//#define accesstoken "aejqudxnrrl4vyqs"
//#define projectkey "dfYotJBXHo"
//#define clid        "thingscloud"
//#define thip         "sh-3-mqtt.iot-api.com"
//#define pub "attributes"
//#define sub "attributes/push"
//#define datas  "{\\\"temperature\\\":28.4}" 
/****************************   STA模式    ****************************/
#define  WIFISSID       "therui"               //wifi热点名称
#define  WIFIPASS       "1234567860"          //wifi热点密码
//#define  IP             "203.107.45.14"     //阿里云服务器IP或域名
#define  URL            "mqtts.heclouds.com"   //服务器IP或域名
#define  PORT           1883			    //连接端口号，MQTT默认1883

////阿里云三元组
//#define ProductKey 		"gxzzl8DRtO7"						//产品密匙
//#define DeviceName 		"esp8266_01s"  							//设备名称
//#define DeviceSecret	"7a3a5864de89f23cd980a0733abee379"	//设备密匙

//AT指令的
//#define AND "&"
#define ClientId "test01"		//客户端ID
#define UserName "kZb47eBW38"					//用户名
#define PassWord "version=2018-10-31&res=products%2FkZb47eBW38%2Fdevices%2Ftest01&et=2062937993&method=md5&sign=gsVJ%2F986BDv9tlehxSTniQ%3D%3D"
                     
//订阅发布的主题
//#define SYS "/sys/"
//#define LINK "/"
//#define TOP "/thing/event/property/"
//#define POST "post"
//#define ESET  "set"
//#define PublishMessageTopPost 	(SYS ProductKey LINK DeviceName TOP POST)
//#define PublishMessageTopSet 	(SYS ProductKey LINK DeviceName TOP ESET)

#define PublishTopic 	"$sys/kZb47eBW38/test01/thing/property/post"
#define SubscribeTopic 	"$sys/kZb47eBW38/test01/thing/property/post/reply"

#define WIFI_RX_LEN_MAX  256
extern unsigned char WIFI_RX_BUFF[WIFI_RX_LEN_MAX];     
extern unsigned char WIFI_RX_FLAG;
extern unsigned char WIFI_RX_LEN;

//上传数据结构体
typedef struct
{
	char keyname[50];   //键读取
	char value[20];     //读取到的值，类型字符串
}JSON_PUBLISH;

void WIFI_ESP01S_Init(void);            //WIFI模块初始化
uint8_t Get_Device_connection_status(void);//获取连接状态
void WIFI_MODE_STA_Aliyun_Init(void);   //连接阿里云初始化
void Get_Aliyun_json_data(JSON_PUBLISH *data);
void Clear_Aliyun_json_data(JSON_PUBLISH *data);
void Publish_MQTT_message(JSON_PUBLISH *data,uint8_t data_num,uint8_t LED_Switch,uint8_t brightness,uint8_t powerstate);
void Publish_MQTT_Message(char* name,float val);
void Publish_MQTT_thingscloud_Message(void);
void WIFI_MODE_STA_ThingsCloud_Init(void);
char WIFI_Send_Cmd(char *cmd,char *ack,unsigned int waitms,unsigned char cnt);
#endif

