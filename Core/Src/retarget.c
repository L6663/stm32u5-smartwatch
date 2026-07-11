/**
  ******************************************************************************
  * @file    retarget.c
  * @brief   Disable semihosting and redirect C library I/O to USART1
  ******************************************************************************
  */

#include "main.h"
#include "usart.h"

#include <stdint.h>
#include <stddef.h>
#include <rt_sys.h>

/*
 * ARM Compiler 6：
 * 禁止C库调用半主机。
 *
 * 如果仍有未重定向的半主机函数，链接阶段会直接报错，
 * 而不是让程序运行后卡在 BKPT #0xAB。
 */
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6000000)
__asm(".global __use_no_semihosting\n");
__asm(".global __ARM_use_no_argv\n");
#endif

/**
  * @brief 判断USART1是否已经完成初始化
  */
static int Retarget_UART_Ready(void)
{
    return (huart1.Instance == USART1) ? 1 : 0;
}

/**
  * @brief 打开标准输入输出设备
  *
  * 对stdin、stdout和stderr统一返回句柄0。
  */
FILEHANDLE _sys_open(const char *name, int openmode)
{
    (void)name;
    (void)openmode;

    return (FILEHANDLE)0;
}

/**
  * @brief 关闭设备
  */
int _sys_close(FILEHANDLE fh)
{
    (void)fh;

    return 0;
}

/**
  * @brief 标准输出重定向到USART1
  *
  * 返回值是“未成功写出的字节数”：
  * 返回0表示全部发送成功。
  */
int _sys_write(FILEHANDLE fh,
               const unsigned char *buffer,
               unsigned int length,
               int mode)
{
    HAL_StatusTypeDef status;

    (void)fh;
    (void)mode;

    if ((buffer == NULL) || (length == 0U))
    {
        return 0;
    }

    /*
     * USART1还未初始化时直接丢弃输出。
     * 绝不能再次调用半主机。
     */
    if (Retarget_UART_Ready() == 0)
    {
        return (int)length;
    }

    /*
     * HAL_UART_Transmit的长度是uint16_t。
     * 当前日志均远小于65535字节。
     */
    status = HAL_UART_Transmit(
        &huart1,
        (uint8_t *)buffer,
        (uint16_t)length,
        1000U
    );

    if (status == HAL_OK)
    {
        return 0;
    }

    return (int)length;
}

/**
  * @brief 标准输入接口
  *
  * 当前工程不使用scanf/getchar，所以直接返回未读取。
  */
int _sys_read(FILEHANDLE fh,
              unsigned char *buffer,
              unsigned int length,
              int mode)
{
    (void)fh;
    (void)buffer;
    (void)mode;

    return (int)length;
}

/**
  * @brief 标准输出设备属于终端
  */
int _sys_istty(FILEHANDLE fh)
{
    (void)fh;

    return 1;
}

/**
  * @brief 不支持文件定位
  */
int _sys_seek(FILEHANDLE fh, long position)
{
    (void)fh;
    (void)position;

    return -1;
}

/**
  * @brief 不支持读取文件长度
  */
long _sys_flen(FILEHANDLE fh)
{
    (void)fh;

    return 0L;
}

/**
  * @brief C库单字符输出接口
  */
void _ttywrch(int character)
{
    uint8_t data;

    if (Retarget_UART_Ready() == 0)
    {
        return;
    }

    data = (uint8_t)character;

    (void)HAL_UART_Transmit(
        &huart1,
        &data,
        1U,
        100U
    );
}

/**
  * @brief 不使用命令行参数
  */
char *_sys_command_string(char *command, int length)
{
    (void)command;
    (void)length;

    return NULL;
}

/**
  * @brief 防止程序退出时调用半主机
  */
void _sys_exit(int return_code)
{
    (void)return_code;

    __disable_irq();

    while (1)
    {
        __NOP();
    }
}