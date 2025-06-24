#ifndef __ESP_H
#define __ESP_H
#include "stm32f4xx.h"
#include "stdio.h"
#include <stdarg.h>
#include <stdbool.h>
#include "delay.h"
#include <string.h>
#include "usart.h"

//ESP8266模式选择
typedef enum
{
    STA,
    AP,
    STA_AP  
}ENUM_Net_ModeTypeDef;
//网络传输层协议，枚举类型
typedef enum{
     enumTCP,
     enumUDP,
} ENUM_NetPro_TypeDef;
//连接号，指定为该连接号可以防止其他计算机访问同一端口而发生错误
typedef enum{
    Multiple_ID_0 = 0,
    Multiple_ID_1 = 1,
    Multiple_ID_2 = 2,
    Multiple_ID_3 = 3,
    Multiple_ID_4 = 4,
    Single_ID_0 = 5,
} ENUM_ID_NO_TypeDef;

//启用匿名联合体
#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

#define RX_BUF_MAX_LEN 1024       //接收缓冲区最大长度
extern char Data_RX_BUF[RX_BUF_MAX_LEN];//接收缓冲区
extern uint16_t FramLength;
extern uint16_t FramFinishFlag;


#define ESP8266_USART(fmt, ...)  USART_printf (USART2, fmt, ##__VA_ARGS__);


/*----------ESP8266-----------*/
void Esp_Init(void);	//初始化
void ESP8266_AT_Test(void);
bool ESP8266_Send_AT_Cmd(char *cmd,char *ack1,char *ack2,u32 time);//发送指令
bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode);//工作模式选择
bool ESP8266_JoinAP( char * pSSID, char * pPassWord);//连接外部WIFI
u8 ESP8266_Get_LinkStatus(void);//检测连接状态
static char *itoa( int value, char *string, int radix );
void USART_printf( USART_TypeDef * USARTx, char * Data, ... );//类似printf

#define User_ESP8266_SSID     "test"          //wifi名
#define User_ESP8266_PWD      "12345678"      //wifi密码

//连接onenet
#define User_ONENET_PRID	"32CiUB9Rq4"	//产品ID	prid
#define User_ONENET_DEID	"407"			//设备ID	deid
#define User_ONENET_PWD		"N3NTZ0NBdDlrZm1wMHNyUTJzVGV2MjlPOHlZc25vNWY="		
#define User_ONENET_TOKEN	"version=2018-10-31&res=products%2F32CiUB9Rq4%2Fdevices%2F407&et=1956499200&method=sha1&sign=0ZVJoj3wHmXTbP4ORjcolkavYu4%3D"

void ESP01S_Connect_wifi(void);
bool ESP01S_Connect_onenet(void);
bool ESP01S_SendMessage_f(char* param, float value);
bool ESP01S_SendMessage_i(char* param, uint32_t value);


//待上传ONENET数据结构体
struct WAITSEND{
	int32_t humi;//湿度
	int32_t temp; // 温度
	int32_t air; // 可燃气体浓度
	int32_t air_mode;// 控制模式
};


uint8_t ESP01S_SEND_MESSAGE_TO_ONENET_f(char* param, float value);
uint8_t ESP01S_SEND_MESSAGE_TO_ONENET_i(char* param, uint32_t value);
//定义一些宏函数，便于发送消息
# define send_humi(value) ESP01S_SEND_MESSAGE_TO_ONENET_i("humi",value)
# define send_temp(value) ESP01S_SEND_MESSAGE_TO_ONENET_i("temp",value)
# define send_air(value) ESP01S_SEND_MESSAGE_TO_ONENET_f("air",value)
# define send_air_mode(value) ESP01S_SEND_MESSAGE_TO_ONENET_i("air_mode",value)

//解析接收到的数据
int extract_last_number(const char *mqtt_msg) ;

//扫描接收
uint16_t ESP8266_air_m(void);
#endif
