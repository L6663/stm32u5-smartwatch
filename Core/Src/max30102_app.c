/**
 ******************************************************************************
 * @file   user_app.c
 * @brief  �û�Ӧ�ó��򲿷ֵĴ���
 *
 ******************************************************************************
 */
#include "string.h"
#include "math.h"
//
#include "max30102_app.h"
/***************************����Ѫ��������***************************/
float n_temperature;
//#define MAX30102_ENABLE  // 已在头文件中定义
//#define MAX30102_BUFF_LENGTH 200 // 已在头文件中定义
//
#ifdef MAX30102_ENABLE
#define MAX_BRIGHTNESS 255
//
uint32_t aun_ir_buffer[MAX30102_BUFF_LENGTH];  // IR LED sensor data
int32_t n_ir_buffer_length;					   // data length
uint32_t aun_red_buffer[MAX30102_BUFF_LENGTH]; // Red LED sensor data
int32_t n_sp02;								   // SPO2 value
int8_t ch_spo2_valid;						   // indicator to show if the SP02 calculation is valid
int32_t n_heart_rate;						   // heart rate value
float n_temperature;						   // ����¶�
int8_t ch_hr_valid;							   // indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;
//
extern void maxim_heart_rate_and_oxygen_saturation(uint32_t *pun_ir_buffer, int32_t n_ir_buffer_length, uint32_t *pun_red_buffer, int32_t *pn_spo2, int8_t *pch_spo2_valid,
												   int32_t *pn_heart_rate, int8_t *pch_hr_valid);
#endif

#ifdef MAX30102_ENABLE
/*
**********************************************************************
* @fun     :mpu_get_max30102_data
* @brief   :��ȡmax30102���ݽ��д������㣬�õ�������Ѫ������
* @param   :None
* @return  :None
**********************************************************************
*/
void mpu_get_max30102_data(void)
{
	int16_t i;
	uch_dummy = 0x00;
	// dumping the first 100 sets of samples in the memory and shift the last 400 sets of samples to the top
	for (i = 100; i < MAX30102_BUFF_LENGTH; i++)
	{
		aun_red_buffer[i - 100] = aun_red_buffer[i];
		aun_ir_buffer[i - 100] = aun_ir_buffer[i];
	}
	// �ڼ�������ǰȡ100������
	for (i = 100; i < MAX30102_BUFF_LENGTH; i++)
	{
		while ((uch_dummy & 0xC0) == 0x00)
			maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_dummy);
		maxim_max30102_read_fifo((aun_red_buffer + i), (aun_ir_buffer + i));
		uch_dummy = 0x00;
#ifdef DEBUG_MODE
		// send samples and calculation result to terminal program through UART
		printf("%i,%i\n\r", aun_red_buffer[i], aun_ir_buffer[i]);
#endif // DEBUG
	}
	maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
	// Read the _chip_ temperature in degrees Celsius
	int8_t integer_temperature;
	uint8_t fractional_temperature;

	maxim_max30102_read_temperature(&integer_temperature, &fractional_temperature);
	n_temperature = integer_temperature + ((float)fractional_temperature) / 16.0;

	// send samples and calculation result to terminal program through UART
	if (ch_hr_valid || ch_spo2_valid)
	{
		printf(" HR=%d, ", n_heart_rate / 4); // ������100sps,max30102����4����ƽ��
		printf("SpO2=%d\n\r", n_sp02);
	}
}
/*
**********************************************************************
* @fun     :Update_HeartRateInfo
* @brief   :����������Ѫ�����Ͷȡ������¶�
* @remark  :���ʴ�������I2C�����봥������ͬһ�����ߣ���ȡ�������ݵ�ʱ
*           ����������mainѭ������˲����������ɼ������ϴ�һ�Ρ���ȡ��
*						�������ݿ���ͨ��printf��ӡ��PC�ˣ���PC�Ͻ��л���Ư�ƵĴ���
*						�����ʲ�����ʾ������
**********************************************************************
*/
void Update_HeartRateInfo(void)
{
	uint16_t pVariableX = 0;
	static uint16_t sample_index = 0;
	uint32_t timeout;
	
	// 每次调用只采集一个样本，累积到缓冲区
	for (pVariableX = 0; pVariableX < 10; pVariableX++)  // 每次采集10个样本
	{
		timeout = 0;
		// 带超时的等待数据
		while ((uch_dummy & 0xC0) == 0x00)
		{
			maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_dummy);
			timeout++;
			if (timeout > 100000) {
				printf("MAX30102 timeout!\r\n");
				return;  // 超时退出，避免卡死
			}
		}
		maxim_max30102_read_fifo((aun_red_buffer + sample_index), (aun_ir_buffer + sample_index));
		aun_red_buffer[sample_index] &= 0x03FF;
		aun_ir_buffer[sample_index] &= 0x03FF;
		uch_dummy = 0x00;
		sample_index++;
		
		// 缓冲区满时计算心率血氧
		if (sample_index >= MAX30102_BUFF_LENGTH)
		{
			sample_index = 0;
			// calculate heart rate and SpO2
			maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, MAX30102_BUFF_LENGTH, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
			
			// 验证数据有效性
			if (!ch_hr_valid) {
				n_heart_rate = 0;
			}
			if (!ch_spo2_valid) {
				n_sp02 = 0;
			}
		}
	}
}
#endif