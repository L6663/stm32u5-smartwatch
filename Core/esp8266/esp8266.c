/*
 * esp8266.c
 *
 *  Created on: 2024年6月14日
 *      Author: Benny
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "esp8266.h"
#include "usart.h"
#include "main.h"

extern uint8_t  ESP8266STATE;
extern UART_HandleTypeDef huart5;
extern char recv_at[256];
extern uint8_t recv_finish;
extern uint32_t count;

static tsTimeType timeESP;
static void setTime(tsTimeType *timeType, uint32_t timeInter);
static uint8_t compareTime(tsTimeType *timeType);


int send_command(char *command, int cmd_len, char *response,  char *response_command, int timeout)
{
	memset(recv_at, 0, 256);
	printf("send: %s\n", command);
	HAL_UART_Transmit(&huart5, (const uint8_t *)command, cmd_len, HAL_MAX_DELAY);
	setTime(&timeESP, timeout);
	while(1) {
		if (compareTime(&timeESP)) {
			return 1;
		}
		if (recv_finish == 1) {
			recv_finish = 0;
			if((strstr(recv_at, response) != NULL)) {
				if (response_command != NULL) {
					memcpy(response_command, recv_at, count);
				}
				return 0;
			} else {
				continue;
			}
		}
	}
}


char* analysis_cmd(char* cmd, char* delim, int count)
{
	int i = 0;
    char *token = strtok(cmd, delim);
	for (i = 0; i < count; i++) {
		token = strtok(NULL, delim);
	}

	return token;
}

void send_mqtt_data(char* send_data, char *response_command)
{
	  char send_at[256] = {0};
	  sprintf(send_at, "AT+MQTTPUB=0,\"%s\",\"%s\",%s,0\r\n", MQTT_PUB_TOPIC, send_data, MQTT_PUB_QOS);
	  send_command(send_at, strlen(send_at), "OK", response_command, 500);
}

int Esp8266CheckState(void)
{
	int ret = 0;
	char send_at[128] = {0};

	strcpy(send_at, "ATE0\r\n");
	send_command(send_at, strlen(send_at), "OK", NULL, 500);

	strcpy(send_at, "AT\r\n");
	ret = send_command(send_at, strlen(send_at), "OK", NULL, 500);

	return ret;
}

/**
 * 函数功能： 设置软件定时器的定时值
 * 参数：
 *     @tsTimeType *TimeType ： 软件定时器的结构体
 *     @uint32_t TimeInter ；   软件定时器的时间（单位：ms）
 * 返回值：
 *      无
*/
static void setTime(tsTimeType *TimeType, uint32_t TimeInter)
{
    TimeType->TimeStart = HAL_GetTick();    //获取系统运行时间
    TimeType->TimeInter = TimeInter;      //间隔时间
}

/**
 * 函数功能： 比较软件定时启的值
 * 参数：
 *      @tsTimeType *timeType： 软件定时器结构体
 * 返回值：
 *      返回1，代表超时
*/
static uint8_t  compareTime(tsTimeType *TimeType)  //比较时间
{
    return ((HAL_GetTick()-TimeType->TimeStart) >= TimeType->TimeInter);  //返回1代表超时
}

