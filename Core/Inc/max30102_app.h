/**
  ******************************************************************************
  * @file   user_app.h
  * @brief  �û�Ӧ�ó��򲿷ֵĴ���
  * 
  ******************************************************************************
  */
#ifndef __USER_APP_H__
#define __USER_APP_H__
//
#include "stdio.h"  //����ʵ��Printf��ӡ���

//MAX30102�����ļ�
#include "max30102.h"

//MAX30102缓冲区长度定义
#define MAX30102_BUFF_LENGTH 200
#define MAX30102_ENABLE

//MAX30102全局变量声明
extern int32_t n_ir_buffer_length;     // 数据缓冲区长度
extern int32_t n_heart_rate;           // 心率值
extern int32_t n_sp02;                 // 血氧饱和度值

//MAX30102缓冲区长度定义
#define MAX30102_BUFF_LENGTH 200



enum{
	eUPDATE_TIME = 0,					/* ϵͳʱ��	*/		
	eUPDATE_FIVEKEY = 1,			/* ���򰴼� */
	eUPDATE_CHIPINFO = 2,			/* ϵͳ��Ϣ */
	eUPDATE_SIX_AXIS = 3,			/* �����˶�	*/
	eUPDATE_WIFI_RSSI = 4,		/* �ź�ǿ��	*/	
	eUPDATE_APP_INFO = 5,			/* APPҳ����Ϣ */		
	eUPDATE_HEART_RATE = 6,		/* ������� */		
	eUPDATE_NIXIE_SHOW = 7		/* �������ʾ */		
};
//����ʹ�ܱ�ֵ
typedef struct
{
	uint32_t UPDATE_TIME_EN:1;					//ϵͳʱ������ʹ��
	uint32_t UPDATE_FIVEKEY_EN:1;				//���򰴼�����ʹ��				
	uint32_t UPDATE_CHIPINFO_EN:1;			//ϵͳ��Ϣ����ʹ��				
	uint32_t UPDATE_SIX_AXIS_EN:1;			//�����˶�����ʹ��
	uint32_t UPDATE_WIFI_RSSI_EN:1;			//WiFi��������ʹ��		
	uint32_t UPDATE_APP_TASK_EN:1;			//APPҳ������ʹ��		
	uint32_t UPDATE_HEART_RATE_EN:1;		//������Ѫ�����Ͷ�����ʹ��
	uint32_t UPDATE_NIXIE_SHOW_EN:1;		//�������ʾ����ʹ��
	uint32_t   :24;
}gTask_MarkEN;

//����״̬��ֵ
typedef struct
{
	uint32_t AP3216PS:1;			//AP3216��PS״̬��ֵ�����ڱ�ʶ�����Ƿ�ӽ�
	uint32_t Sport:1;					//�˶�״̬
	uint32_t ADCC:1;					//ADCת�����
	uint32_t TouchPress:1;		//1���������� 0�������ͷ�
	uint32_t FiveKeyPress:1;	//1����������� 0�������ͷ�
	uint32_t ExtITR:1;				//1����翪�ش��� 0���ͷ�
	uint32_t ExtFIRE:1;				//1�������ⴥ��	0���ͷ� 
	uint32_t ExtPIR:1;				//1�����͵紥��	0���ͷ� 
	uint32_t Max30102:1;			//1��max30102�������	0���ͷ� 
	uint32_t   :23;
}gTask_BitDef;

// Retrieve year info
#define OS_YEAR     ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
                                    + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))

// Retrieve month info
#define OS_MONTH    (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6) \
                                : __DATE__ [2] == 'b' ? 2 \
                                : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
                                : __DATE__ [2] == 'y' ? 5 \
                                : __DATE__ [2] == 'l' ? 7 \
                                : __DATE__ [2] == 'g' ? 8 \
                                : __DATE__ [2] == 'p' ? 9 \
                                : __DATE__ [2] == 't' ? 10 \
                                : __DATE__ [2] == 'v' ? 11 : 12)

// Retrieve day info
#define OS_DAY      ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 \
                                + (__DATE__ [5] - '0'))

// Retrieve hour info
#define OS_HOUR     ((__TIME__ [0] - '0') * 10 + (__TIME__ [1] - '0'))

// Retrieve minute info
#define OS_MINUTE   ((__TIME__ [3] - '0') * 10 + (__TIME__ [4] - '0'))

// Retrieve second info
#define OS_SECOND   ((__TIME__ [6] - '0') * 10 + (__TIME__ [7] - '0'))
//
#define VREFINT_CAL ((uint16_t*) ((uint32_t) 0x0BFA07A5)) //�ڲ��ο���ѹԴ����ֵ
#define TS_CAL1 		((uint16_t*) ((uint32_t) 0x0BFA0710)) //�ڲ��¶Ⱦ���ֵ
//
#define RTC_BKP0RL_VAULE			0x1A1B
//ϵͳ����
#define BEEP_ENABLE()						HAL_GPIO_WritePin(RUN_BEEP_GPIO_Port, RUN_BEEP_Pin, GPIO_PIN_SET)					/* ʹ�ܷ��������� */
#define BEEP_DISABLE()					HAL_GPIO_WritePin(RUN_BEEP_GPIO_Port, RUN_BEEP_Pin, GPIO_PIN_RESET)				/* ��ֹ���������� */
//ϵͳ����ָʾ������չָʾ��LD1
#define RUN_LED_ENABLE()				HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_SET)		 			/* ʹ������ָʾ�� */
#define RUN_LED_DISABLE()	  		HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_RESET)				/* ��ֹ����ָʾ�� */
#define RUN_LED_TURN_STATE()	  HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin)											/* ��ֹ����ָʾ�� */
//��չ��ָʾ��LD2
#define EXT_LED2_ENABLE()				HAL_GPIO_WritePin(EXT_LED2_GPIO_Port, EXT_LED2_Pin, GPIO_PIN_SET)		 			/* ʹ���ⲿָʾ��2 */
#define EXT_LED2_DISABLE()	  	HAL_GPIO_WritePin(EXT_LED2_GPIO_Port, EXT_LED2_Pin, GPIO_PIN_RESET)				/* ��ֹ�ⲿָʾ��2 */
//��չ��ָʾ��LD3
#define EXT_LED3_ENABLE()				HAL_GPIO_WritePin(EXT_LED3_GPIO_Port, EXT_LED3_Pin, GPIO_PIN_SET)		 			/* ʹ���ⲿָʾ��3 */
#define EXT_LED3_DISABLE()	  	HAL_GPIO_WritePin(EXT_LED3_GPIO_Port, EXT_LED3_Pin, GPIO_PIN_RESET)				/* ��ֹ�ⲿָʾ��3 */
//��չ�����
#define EXT_FAN_ENABLE()				HAL_GPIO_WritePin(EXT_FAN_GPIO_Port, EXT_FAN_Pin, GPIO_PIN_SET)		 				/* ʹ����չ����� */
#define EXT_FAN_DISABLE()	  		HAL_GPIO_WritePin(EXT_FAN_GPIO_Port, EXT_FAN_Pin, GPIO_PIN_RESET)					/* ��ֹ��չ����� */
//��չ���𶯵��
#define EXT_MOTOR_ENABLE()			HAL_GPIO_WritePin(EXT_MOTOR_GPIO_Port, EXT_MOTOR_Pin, GPIO_PIN_SET)		 		/* ʹ���𶯵�� */
#define EXT_MOTOR_DISABLE()	  	HAL_GPIO_WritePin(EXT_MOTOR_GPIO_Port, EXT_MOTOR_Pin, GPIO_PIN_RESET)			/* ��ֹ�𶯵�� */
//
void ESP8266_RSSI_Task(void);
void Update_AppPageInfo(void);
void Update_Backlight(uint8_t pDutyRatio);
void Update_ChipInfo(void);
void Update_FiveKey_Value(void);
void Update_System_Time(void);
void System_Time_init(void);
//
void mpu_init_dmp(void);
void Update_EulerAngle(void);
void Update_HeartRateInfo(void);
void Update_NixieDisplay(void);
 void mpu_get_max30102_data(void);
//�����ϵͳ��������
#define OS_TASKLISTCNT	8  
extern void (* g_OSTsakList[OS_TASKLISTCNT])(void);
#endif /* __USER_APP_H__ */

