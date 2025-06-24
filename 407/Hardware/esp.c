#include "esp.h"
# include <string.h>
#include <stdlib.h>
#include <ctype.h>


char Data_RX_BUF[RX_BUF_MAX_LEN];//接收缓冲区
uint16_t FramLength;
uint16_t FramFinishFlag;

//////////我的接收缓冲区和接收标志位////
extern char MY_RX_BUF[RX_BUF_MAX_LEN];//
uint8_t RxFlag;


// 初始化端口
// PA2-ESP_RX, PA3-ESP_TX
void Esp_Init(void)
{
    uart2_Init(115200);
}

// 对ESP8266模块发送AT指令
//  cmd 待发送的指令
//  ack1,ack2;期待的响应，为NULL表不需响应，两者为或逻辑关系
//  time 等待响应时间
// 返回1发送成功， 0失败
bool ESP8266_Send_AT_Cmd(char *cmd, char *ack1, char *ack2, u32 time)
{
    FramLength = 0; // 重置接收数据包
    ESP8266_USART("%s\r\n", cmd);
    if (ack1 == 0 && ack2 == 0) // 不需要响应
    {
        return true;
    }
    delay_ms(time); // 延时
    delay_ms(1000);
	//memset(Data_RX_BUF,0,RX_BUF_MAX_LEN);
    Data_RX_BUF[FramLength] = '\0';//清空接收缓冲区

    printf("%s", Data_RX_BUF);
    if (ack1 != 0 && ack2 != 0)
    {
        return ((bool)strstr(Data_RX_BUF, ack1) ||
                (bool)strstr(Data_RX_BUF, ack2));
    }
    else if (ack1 != 0) // strstr(s1,s2);检测s2是否为s1的一部分，是返回该位置，否则返回false，它强制转换为bool类型了
        return ((bool)strstr(Data_RX_BUF, ack1));

    else
        return ((bool)strstr(Data_RX_BUF, ack2));
}
// 发送恢复出厂默认设置指令将模块恢复成出厂设置
void ESP8266_AT_Test(void)
{
    char count = 0;
    delay_ms(1000);
    while (count < 10)
    {
        if (ESP8266_Send_AT_Cmd("AT+RESTORE", "OK", NULL, 500))
        {
            printf("OK\r\n");
            return;
        }
        ++count;
    }
}

// 选择ESP8266的工作模式
//  enumMode 模式类型
// 成功返回true，失败返回false
bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode)
{
    switch (enumMode)
    {
    case STA:
        return ESP8266_Send_AT_Cmd("AT+CWMODE=1", "OK", "no change", 2500);

    case AP:
        return ESP8266_Send_AT_Cmd("AT+CWMODE=2", "OK", "no change", 2500);

    case STA_AP:
        return ESP8266_Send_AT_Cmd("AT+CWMODE=3", "OK", "no change", 2500);

    default:
        return false;
    }
}
// ESP8266连接外部的WIFI
// pSSID WiFi帐号
// pPassWord WiFi密码
// 设置成功返回true 反之false
bool ESP8266_JoinAP(char *pSSID, char *pPassWord)
{
    char cCmd[120];
	sprintf(cCmd,"AT+CWQAP");
	ESP8266_Send_AT_Cmd(cCmd,NULL, NULL,1000);
	delay_ms(2500);
    sprintf(cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord);
    return ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 5000);
}


// ESP8266发送字符串
// enumEnUnvarnishTx是否使能透传模式
// pStr字符串
// ulStrLength字符串长度
// ucId 连接号
// 设置成功返回true， 反之false
bool ESP8266_SendString(FunctionalState enumEnUnvarnishTx, char *pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId)
{
    char cStr[20];
    bool bRet = false;
    if (enumEnUnvarnishTx)
    {
        ESP8266_USART("%s", pStr);
        bRet = true;
    }
    else
    {
        if (ucId < 5)
            sprintf(cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2);
        else
            sprintf(cStr, "AT+CIPSEND=%d", ulStrLength + 2);
        ESP8266_Send_AT_Cmd(cStr, "> ", 0, 1000);
        bRet = ESP8266_Send_AT_Cmd(pStr, "SEND OK", 0, 1000);
    }
    return bRet;
}
// ESP8266 检测连接状态
// 返回0：获取状态失败
// 返回2：获得ip
// 返回3：建立连接
// 返回4：失去连接
u8 ESP8266_Get_LinkStatus(void)
{
    if (ESP8266_Send_AT_Cmd("AT+CIPSTATUS", "OK", 0, 500))
    {
        if (strstr(Data_RX_BUF, "STATUS:2\r\n"))
            return 2;
        else if (strstr(Data_RX_BUF, "STATUS:3\r\n"))
            return 3;
        else if (strstr(Data_RX_BUF, "STATUS:4\r\n"))
            return 4;
    }
    return 0;
}
// 将整数转换为字符串
static char *itoa(int value, char *string, int radix)
{
    int i, d;
    int flag = 0;
    char *ptr = string;
    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }
    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }
    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';
        /* Make the value positive. */
        value *= -1;
    }
    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;
        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }
    /* Null terminate the string. */
    *ptr = 0;
    return string;
} /* NCL_Itoa */
void USART_printf(USART_TypeDef *USARTx, char *Data, ...)
{
    const char *s;
    int d;
    char buf[16];
    va_list ap;
    va_start(ap, Data);
    while (*Data != 0) // 判断数据是否到达结束符
    {
        if (*Data == 0x5c) //'\'
        {
            switch (*++Data)
            {
            case 'r': // 回车符
                USART_SendData(USARTx, 0x0d);
                Data++;
                break;
            case 'n': // 换行符
                USART_SendData(USARTx, 0x0a);
                Data++;
                break;
            default:
                Data++;
                break;
            }
        }
        else if (*Data == '%')
        {
            switch (*++Data)
            {
            case 's': // 字符串
                s = va_arg(ap, const char *);
                for (; *s; s++)
                {
                    USART_SendData(USARTx, *s);
                    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
                        ;
                }
                Data++;
                break;
            case 'd':
                // 十进制
                d = va_arg(ap, int);
                itoa(d, buf, 10);
                for (s = buf; *s; s++)
                {
                    USART_SendData(USARTx, *s);
                    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
                        ;
                }
                Data++;
                break;
            default:
                Data++;
                break;
            }
        }
        else
            USART_SendData(USARTx, *Data++);
        while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
            ;
    }
}



/////////////////// 与ONENET通信
// 配置用户信息（自动重试100次）
bool ESP01S_MQTTSETUSER(char *DEID, char *PRID, char *TOKEN)
{
// AT+MQTTUSERCFG=0,1,"设备ID","产品ID","Token",0,0,""
#define MAX_RETRY 100      // 最大重试次数
#define RETRY_DELAY 3000 // 重试间隔 1 秒
    char cCmd[256];
	
	//先发送一次断开连接指令，防止已经连接ONENET，造成连接出错
	sprintf(cCmd,"AT+MQTTCLEAN=0");
	ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 500);
	delay_ms(5000);
	
    uint8_t retry = 0;
    while (retry < MAX_RETRY)
    {
        sprintf(cCmd, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"", DEID, PRID, TOKEN);
        if (ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 500))
        {
            return true; // 配置成功
        }
		sprintf(cCmd,"AT+MQTTCLEAN=0");
		ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 500);
        delay_ms(RETRY_DELAY); // 等待后重试
        retry++;
    }
    return false; // 重试失败
}
// 连接 MQTT服务器（自动重试）
bool ESP01S_MQTTCONN(void)
{
// AT+MQTTCONN=0,"mqtts.heclouds.com",1883,1
    char cCmd[128];
    uint8_t retry = 0;
    sprintf(cCmd, "AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1");
    if (ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 2000))
    {
		return true; // 连接成功
	}
    return false; // 所有重试均失败
}

// 订阅消息，向云发送消息后，云端给一个回复
bool ESP01S_MQTTSUB1(char *PRID, char *DEID)
{
    // AT+MQTTSUB=0,"$sys/产品ID/设备ID/thing/property/post/reply",1
    char cCmd[256];
    sprintf(cCmd, "AT+MQTTSUB=0,\"$sys/%s/%s/thing/property/post/reply\",1", PRID, DEID);
    return ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 500);
}
// 订阅消息，当云端物理特性改变后，会向设备发送一个消息
bool ESP01S_MQTTSUB2(char *PRID, char *DEID)
{
    // AT+MQTTSUB=0,"$sys/产品ID/设备ID/thing/property/set",1
    char cCmd[256];
    sprintf(cCmd, "AT+MQTTSUB=0,\"$sys/%s/%s/thing/property/set\",1", PRID, DEID);
    return ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 500);
}
// 向云端发送消息，发送后收到一个">"，之后发送话题消息，需要加"\r\n"
bool ESP01S_MQTT_SEND_ME(char *PRID, char *DEID, uint8_t len)
{
    // AT+MQTTPUBRAW=0,"$sys/产品ID/设备ID/thing/property/post",118,0,0
    char cCmd[256];
    sprintf(cCmd, "AT+MQTTPUBRAW=0,\"$sys/%s/%s/thing/property/post\",%d,1,0", PRID, DEID,len);
    ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 500);
    return (strstr(Data_RX_BUF, ">"));
}

// 连接wifi
void ESP01S_Connect_wifi(void)
{
	printf("开始连接wifi\r\n");
    ESP8266_AT_Test(); // 恢复出厂默认模式
    ESP8266_Net_Mode_Choose(STA);
    while (!ESP8266_JoinAP(User_ESP8266_SSID, User_ESP8266_PWD))
        ;
	delay_ms(100);
	if(ESP8266_Get_LinkStatus() == 2)//检查是是否连接WIFI,未连接则自动重连
	{
		printf("已经连接wifi\r\n");
	}
	
}
// 连接onenet
bool ESP01S_Connect_onenet(void)
{

	printf("开始连接Onenet\r\n");
    ESP01S_MQTTSETUSER(User_ONENET_DEID, User_ONENET_PRID, User_ONENET_TOKEN);
    delay_ms(3000);
    ESP01S_MQTTCONN();
	delay_ms(1000);
	ESP01S_MQTTSUB1(User_ONENET_PRID, User_ONENET_DEID);
	delay_ms(1000);
    bool re = ESP01S_MQTTSUB2(User_ONENET_PRID, User_ONENET_DEID);
	delay_ms(1000);
    printf("已连接到Onenet\r\n");
	return re ;
}

// 向onenet发送消息,虽然usart3不会打印发送内容，但onenet，能接收到改变
bool ESP01S_SendMessage(char* param, uint16_t value)
{
	char cCmd[64];
	sprintf(cCmd, "{\"id\":\"123\",\"params\":{\"%s\":{\"value\":%d}}}",param, value);  
    ESP01S_MQTT_SEND_ME(User_ONENET_PRID, User_ONENET_DEID,strlen(cCmd));
	delay_ms(500);
    if (strstr(Data_RX_BUF, ">") != NULL)
    {
		ESP8266_USART("%s", cCmd);
    }
	if(strstr(Data_RX_BUF, "OK") != NULL)
	{
		return 1;
	}
	else
		return 0;
}
bool ESP01S_SendMessage_f(char* param, float value)
{
	char cCmd[64];
	sprintf(cCmd, "{\"id\":\"123\",\"params\":{\"%s\":{\"value\":%.2f}}}",param, value);  
    ESP01S_MQTT_SEND_ME(User_ONENET_PRID, User_ONENET_DEID,strlen(cCmd));
	delay_ms(500);
    if (strstr(Data_RX_BUF, ">") != NULL)
    {
		ESP8266_USART("%s", cCmd);
    }
	if(strstr(Data_RX_BUF, "OK") != NULL)
	{
		return 1;
	}
	else
		return 0;
}
//发送消息
uint8_t ESP01S_SEND_MESSAGE_TO_ONENET_f(char* param, float value)
{
	ESP01S_SendMessage_f(param, value);
	delay_ms(100);
	ESP01S_SendMessage_f(param, value);
}
uint8_t ESP01S_SEND_MESSAGE_TO_ONENET_i(char* param, uint32_t value)
{
	ESP01S_SendMessage(param, value);
	delay_ms(100);
	ESP01S_SendMessage(param, value);
}


// 从ONENET MQTT消息中提取最后一项的数字值（支持0-9999）
int extract_last_number(const char *mqtt_msg) {
    // 查找JSON部分的起始位置
    const char *json_start = strchr(mqtt_msg, '{');
    if (json_start == NULL) return 0;
    // 查找params部分的起始位置
    const char *params_start = strstr(json_start, "\"params\":{");
    if (params_start == NULL) return 0;
    params_start += strlen("\"params\":{");   
    // 查找最后一个冒号的位置（用于定位数字）
    const char *last_colon = strrchr(params_start, ':');
    if (last_colon == NULL) return 0;    
    // 跳过可能的空格
    const char *num_start = last_colon + 1;
    while (*num_start && isspace(*num_start)) {
        num_start++;
    }    
    // 查找数字的结束位置（第一个非数字字符）
    const char *num_end = num_start;
    while (*num_end && isdigit(*num_end)) {
        num_end++;
    }    
    // 检查是否找到有效的数字
    if (num_end == num_start) return 0;    
    // 提取数字字符串并转换为整数
    char num_str[16] = {0};
    memcpy(num_str, num_start, num_end - num_start);   
    int result = atoi(num_str);    
    // 添加范围检查（0-9999）
    if (result < 0) return 0;
    if (result > 9999) return 9999;    
    return result;
}

//接收消息扫描
uint16_t ESP8266_air_m(void)
{
	printf("%s",Data_RX_BUF);
	if(strstr(Data_RX_BUF,"air_mode"))
	{
		uint16_t value = extract_last_number(Data_RX_BUF);
		
		return value;
	}
	delay_ms(200);
	
	// 清空接收缓冲区
    memset(Data_RX_BUF, 0, RX_BUF_MAX_LEN);
    FramLength = 0;  // 重置帧长度计数器
	
	return 0;
}

