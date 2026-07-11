/**
  ******************************************************************************
  * @file   bsp_ft6336.c
  * @brief  2.8寸屏ft6336驱动文件,i2c接口
  * 
  ******************************************************************************
  */
#include "bsp_ft6336.h"
#include "i2c.h"
#include "gpio.h"
#include "cmsis_os2.h"

#include <stdbool.h>

/* I2C1 is shared with SHT20 and MAX30102. */
extern osMutexId_t i2c1MutexHandle;

#define FT6336_I2C_TIMEOUT_MS       20U
#define FT6336_MUTEX_TIMEOUT_MS      5U

volatile uint32_t g_ft6336_i2c_error_count = 0U;
volatile uint32_t g_ft6336_mutex_timeout_count = 0U;

static bool FT6336_LockBus(void)
{
    /* Allow pre-kernel diagnostic access while keeping runtime access serialized. */
    if (i2c1MutexHandle == NULL)
    {
        return true;
    }

    if (osMutexAcquire(i2c1MutexHandle, FT6336_MUTEX_TIMEOUT_MS) != osOK)
    {
        g_ft6336_mutex_timeout_count++;
        return false;
    }

    return true;
}

static void FT6336_UnlockBus(void)
{
    if (i2c1MutexHandle != NULL)
    {
        (void)osMutexRelease(i2c1MutexHandle);
    }
}

static bool FT6336_ReadRegisters(
    uint8_t start_address,
    uint8_t *data,
    uint16_t length)
{
    HAL_StatusTypeDef status;

    if ((data == NULL) || (length == 0U) || !FT6336_LockBus())
    {
        return false;
    }

    status = HAL_I2C_Mem_Read(
        &hi2c1,
        FT6336_ADDR_WRITE,
        start_address,
        I2C_MEMADD_SIZE_8BIT,
        data,
        length,
        FT6336_I2C_TIMEOUT_MS
    );

    FT6336_UnlockBus();

    if (status != HAL_OK)
    {
        g_ft6336_i2c_error_count++;
        return false;
    }

    return true;
}

static bool FT6336_WriteRegister(uint8_t address, uint8_t data)
{
    HAL_StatusTypeDef status;

    if (!FT6336_LockBus())
    {
        return false;
    }

    status = HAL_I2C_Mem_Write(
        &hi2c1,
        FT6336_ADDR_WRITE,
        address,
        I2C_MEMADD_SIZE_8BIT,
        &data,
        1U,
        FT6336_I2C_TIMEOUT_MS
    );

    FT6336_UnlockBus();

    if (status != HAL_OK)
    {
        g_ft6336_i2c_error_count++;
        return false;
    }

    return true;
}

//event_notify_cb touch_notify;
FT6336_TouchPointType tp_dev =
{
	.touch_count = 0,
	
	.tp[0].status = release,
  .tp[0].x = 0, 
  .tp[0].y = 0,
	
	.tp[1].status = release,
  .tp[1].x = 0, 
  .tp[1].y = 0,
	
	.press_status = 0,
    
	.init = FT6336_init,	/* 初始化触摸屏控制器 */
	.scan = FT6336_scan_task,	/* 扫描触摸屏 */
};

/*
**********************************************************************
* @fun     :FT6336_readByte 
* @brief   :读取从机数据
* @param   :addr:寄存器地址
* @return  :data:返回的数据
**********************************************************************
*/
uint8_t FT6336_readByte(uint8_t addr)
{
    uint8_t data = 0U;

    (void)FT6336_ReadRegisters(addr, &data, 1U);

    return data;
}
/*
**********************************************************************
* @fun     :FT6336_writeByte 
* @brief   :对从机写数据
* @param   :addr:寄存器地址，data:写入的数据
* @return  :None 
**********************************************************************
*/
void FT6336_writeByte(uint8_t addr, uint8_t data)
{
    (void)FT6336_WriteRegister(addr, data);
}
/*
**********************************************************************
* @fun     :FT6336_irq_fuc 
* @brief   :中断函数调用，生产按压事件
* @param   :None
* @return  :None 
**********************************************************************
*/
void FT6336_irq_fuc(void)
{
    tp_dev.press_status = 1;
}
/*
**********************************************************************
* @fun     :FT6336_scan_task 
* @brief   :触摸事件，扫描触摸点
* @param   :None
* @return  :None 
**********************************************************************
*/
void FT6336_scan_task(void)
{
    if (tp_dev.press_status) // irq done
    {
        FT6336_scan();
        if (tp_dev.tp[0].status == 1)
        {                                  //最多支持两个触点
					printf("touch down x %d  y %d\r\n", tp_dev.tp[0].x, tp_dev.tp[0].y);
        }
        tp_dev.press_status = 0;
    }
}
/*
**********************************************************************
* @fun     :FT6336_init 
* @brief   :FT6336初始化函数
* @param   :None
* @return  :None 
**********************************************************************
*/
void FT6336_init(void)
{
    // Int Pin Configuration
		HAL_GPIO_WritePin(TP_RST_GPIO_Port, TP_RST_Pin, GPIO_PIN_RESET);
		HAL_Delay(10);
		HAL_GPIO_WritePin(TP_RST_GPIO_Port, TP_RST_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
}
/*
**********************************************************************
* @fun     :FT6336状态读取函数
**********************************************************************
*/
uint8_t FT6336_read_device_mode(void)
{
    return (FT6336_readByte(FT6336_ADDR_DEVICE_MODE) & 0x70) >> 4;
}
//
void FT6336_write_device_mode(DEVICE_MODE_Enum mode)
{
    FT6336_writeByte(FT6336_ADDR_DEVICE_MODE, (mode & 0x07) << 4);
}
//
uint8_t FT6336_read_gesture_id(void)
{
    return FT6336_readByte(FT6336_ADDR_GESTURE_ID);
}
//
uint8_t FT6336_read_td_status(void)
{
    return FT6336_readByte(FT6336_ADDR_TD_STATUS);
}
//
uint8_t FT6336_read_touch_number(void)
{
    return FT6336_readByte(FT6336_ADDR_TD_STATUS) & 0x0F;
}
/*
**********************************************************************
* @fun     :FT6336_read_touch1_x 
* @brief   :读取touch1的数据
* @param   :None
* @return  :None 
**********************************************************************
*/
uint16_t FT6336_read_touch1_x(void)
{
    uint8_t read_buf[2];
    read_buf[0] = FT6336_readByte(FT6336_ADDR_TOUCH1_X);
    read_buf[1] = FT6336_readByte(FT6336_ADDR_TOUCH1_X + 1);
    return ((read_buf[0] & 0x0f) << 8) | read_buf[1];
}
//
uint16_t FT6336_read_touch1_y(void)
{
    uint8_t read_buf[2];
    read_buf[0] = FT6336_readByte(FT6336_ADDR_TOUCH1_Y);
    read_buf[1] = FT6336_readByte(FT6336_ADDR_TOUCH1_Y + 1);
    return ((read_buf[0] & 0x0f) << 8) | read_buf[1];
}
//
uint8_t FT6336_read_touch1_event(void)
{
    return FT6336_readByte(FT6336_ADDR_TOUCH1_EVENT) >> 6;
}
//
uint8_t FT6336_read_touch1_id(void)
{
    return FT6336_readByte(FT6336_ADDR_TOUCH1_ID) >> 4;
}
//
uint8_t FT6336_read_touch1_weight(void)
{
    return FT6336_readByte(FT6336_ADDR_TOUCH1_WEIGHT);
}
//
uint8_t FT6336_read_touch1_misc(void)
{
    return FT6336_readByte(FT6336_ADDR_TOUCH1_MISC) >> 4;
}
/*
**********************************************************************
* @fun     :FT6336_read_touch2_x 
* @brief   :读取touch2的数据
* @param   :None
* @return  :None 
**********************************************************************
*/
uint16_t FT6336_read_touch2_x(void)
{
    uint8_t read_buf[2];
    read_buf[0] = FT6336_readByte(FT6336_ADDR_TOUCH2_X);
    read_buf[1] = FT6336_readByte(FT6336_ADDR_TOUCH2_X + 1);
    return ((read_buf[0] & 0x0f) << 8) | read_buf[1];
}
//
uint16_t FT6336_read_touch2_y(void)
{
    uint8_t read_buf[2];
    read_buf[0] = FT6336_readByte(FT6336_ADDR_TOUCH2_Y);
    read_buf[1] = FT6336_readByte(FT6336_ADDR_TOUCH2_Y + 1);
    return ((read_buf[0] & 0x0f) << 8) | read_buf[1];
}
//
uint8_t FT6336_read_touch2_event(void)
{
    return FT6336_readByte(FT6336_ADDR_TOUCH2_EVENT) >> 6;
}
//
uint8_t FT6336_read_touch2_id(void)
{
    return FT6336_readByte(FT6336_ADDR_TOUCH2_ID) >> 4;
}
//
uint8_t FT6336_read_touch2_weight(void)
{
    return FT6336_readByte(FT6336_ADDR_TOUCH2_WEIGHT);
}
//
uint8_t FT6336_read_touch2_misc(void)
{
    return FT6336_readByte(FT6336_ADDR_TOUCH2_MISC) >> 4;
}
/*
**********************************************************************
* @fun     :Mode Parameter Register
**********************************************************************
*/
uint8_t FT6336_read_touch_threshold(void)
{
    return FT6336_readByte(FT6336_ADDR_THRESHOLD);
}
//
uint8_t FT6336_read_filter_coefficient(void)
{
    return FT6336_readByte(FT6336_ADDR_FILTER_COE);
}
//
uint8_t FT6336_read_ctrl_mode(void)
{
    return FT6336_readByte(FT6336_ADDR_CTRL);
}
//
void FT6336_write_ctrl_mode(CTRL_MODE_Enum mode)
{
    FT6336_writeByte(FT6336_ADDR_CTRL, mode);
}
//
uint8_t FT6336_read_time_period_enter_monitor(void)
{
    return FT6336_readByte(FT6336_ADDR_TIME_ENTER_MONITOR);
}
//
uint8_t FT6336_read_active_rate(void)
{
    return FT6336_readByte(FT6336_ADDR_ACTIVE_MODE_RATE);
}
//
uint8_t FT6336_read_monitor_rate(void)
{
    return FT6336_readByte(FT6336_ADDR_MONITOR_MODE_RATE);
}
/*
**********************************************************************
* @fun     :Gesture Parameters
**********************************************************************
*/
uint8_t FT6336_read_radian_value(void)
{
    return FT6336_readByte(FT6336_ADDR_RADIAN_VALUE);
}
//
void FT6336_write_radian_value(uint8_t val)
{
    FT6336_writeByte(FT6336_ADDR_RADIAN_VALUE, val);
}
//
uint8_t FT6336_read_offset_left_right(void)
{
    return FT6336_readByte(FT6336_ADDR_OFFSET_LEFT_RIGHT);
}
//
void FT6336_write_offset_left_right(uint8_t val)
{
    FT6336_writeByte(FT6336_ADDR_OFFSET_LEFT_RIGHT, val);
}
//
uint8_t FT6336_read_offset_up_down(void)
{
    return FT6336_readByte(FT6336_ADDR_OFFSET_UP_DOWN);
}
//
void FT6336_write_offset_up_down(uint8_t val)
{
    FT6336_writeByte(FT6336_ADDR_OFFSET_UP_DOWN, val);
}
//
uint8_t FT6336_read_distance_left_right(void)
{
    return FT6336_readByte(FT6336_ADDR_DISTANCE_LEFT_RIGHT);
}
//
void FT6336_write_distance_left_right(uint8_t val)
{
    FT6336_writeByte(FT6336_ADDR_DISTANCE_LEFT_RIGHT, val);
}
//
uint8_t FT6336_read_distance_up_down(void)
{
    return FT6336_readByte(FT6336_ADDR_DISTANCE_UP_DOWN);
}
//
void FT6336_write_distance_up_down(uint8_t val)
{
    FT6336_writeByte(FT6336_ADDR_DISTANCE_UP_DOWN, val);
}
//
uint8_t FT6336_read_distance_zoom(void)
{
    return FT6336_readByte(FT6336_ADDR_DISTANCE_ZOOM);
}
//
void FT6336_write_distance_zoom(uint8_t val)
{
    FT6336_writeByte(FT6336_ADDR_DISTANCE_ZOOM, val);
}
/*
**********************************************************************
* @fun     :System Information
**********************************************************************
*/
uint16_t FT6336_read_library_version(void)
{
    uint8_t read_buf[2];
    read_buf[0] = FT6336_readByte(FT6336_ADDR_LIBRARY_VERSION_H);
    read_buf[1] = FT6336_readByte(FT6336_ADDR_LIBRARY_VERSION_L);
    return ((read_buf[0] & 0x0f) << 8) | read_buf[1];
}
//
uint8_t FT6336_read_chip_id(void)
{
    return FT6336_readByte(FT6336_ADDR_CHIP_ID);
}
//
uint8_t FT6336_read_g_mode(void)
{
    return FT6336_readByte(FT6336_ADDR_G_MODE);
}
//
void FT6336_write_g_mode(G_MODE_Enum mode)
{
    FT6336_writeByte(FT6336_ADDR_G_MODE, mode);
}
//
uint8_t FT6336_read_pwrmode(void)
{
    return FT6336_readByte(FT6336_ADDR_POWER_MODE);
}
//
uint8_t FT6336_read_firmware_id(void)
{
    return FT6336_readByte(FT6336_ADDR_FIRMARE_ID);
}
//
uint8_t FT6336_read_focaltech_id(void)
{
    return FT6336_readByte(FT6336_ADDR_FOCALTECH_ID);
}
//
uint8_t FT6336_read_release_code_id(void)
{
    return FT6336_readByte(FT6336_ADDR_RELEASE_CODE_ID);
}
//
uint8_t FT6336_read_state(void)
{
    return FT6336_readByte(FT6336_ADDR_STATE);
}
/*
**********************************************************************
* @fun     :FT6336_scan 
* @brief   :FT6336触摸点读取
* @param   :None
* @return  :None
**********************************************************************
*/
void FT6336_scan(void)
{
    uint8_t touch_data[7];
    uint8_t id1;

    /*
     * Read TD_STATUS and the complete first touch point in one I2C transaction.
     * This reduces bus occupancy and prevents coordinate bytes from belonging to
     * different controller samples.
     */
    if (!FT6336_ReadRegisters(
            FT6336_ADDR_TD_STATUS,
            touch_data,
            sizeof(touch_data)))
    {
        tp_dev.touch_count = 0U;
        tp_dev.tp[0].status = release;
        tp_dev.tp[1].status = release;
        return;
    }

    tp_dev.touch_count = touch_data[0] & 0x0FU;

    if (tp_dev.touch_count == 0U)
    {
        tp_dev.tp[0].status = release;
        tp_dev.tp[1].status = release;
        return;
    }

    id1 = touch_data[3] >> 4U;

    if (id1 > 1U)
    {
        tp_dev.touch_count = 0U;
        tp_dev.tp[0].status = release;
        tp_dev.tp[1].status = release;
        return;
    }

    tp_dev.tp[0].status =
        (tp_dev.tp[0].status == release) ? touch : stream;

    tp_dev.tp[0].x =
        (uint16_t)(
            (((uint16_t)(touch_data[1] & 0x0FU)) << 8U) |
            touch_data[2]
        );

    tp_dev.tp[0].y =
        (uint16_t)(
            (((uint16_t)(touch_data[3] & 0x0FU)) << 8U) |
            touch_data[4]
        );

    tp_dev.tp[1].status = release;
}