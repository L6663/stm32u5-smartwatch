/*
 * esp8266.h
 *
 *  Created on: 2024年6月14日
 *      Author: Benny
 */

#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

#define MQTT_QOS			"1"
//#define MQTT_ENABLE_TLS		"1"

#define MQTT_SUB_QOS		"1"
#define MQTT_PUB_QOS		"0"

extern char  MQTT_SUB_TOPIC[128];
extern char  MQTT_PUB_TOPIC[128];
extern char  MQTT_ENABLE_TLS[2];

typedef struct
{
		int TimeStart;
    int TimeInter;
} tsTimeType;

void send_mqtt_data(char* send_data, char *response_command);
int send_command(char *command, int cmd_len, char *response,  char *response_command, int timeout);
char* analysis_cmd(char* cmd, char* delim, int count);
int esp8266_mqtt_init(void);

#endif /* INC_ESP8266_H_ */
