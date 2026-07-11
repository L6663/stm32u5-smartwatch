/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os2.h"
#include "gpdma.h"
#include "i2c.h"
#include "icache.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_freertos.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/*
 * 启动阶段标志。
 *
 * 可以在Keil Watch窗口观察：
 * g_boot_stage
 */
volatile uint32_t g_boot_stage = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */

static void Main_Debug_Print(const char *text);
static void Main_Debug_Printf(const char *format, ...);
static void RTC_BackupDomain_Prepare(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief USART1发送普通字符串
  */
static void Main_Debug_Print(const char *text)
{
    size_t length;

    if (text == NULL)
    {
        return;
    }

    length = strlen(text);

    if (length == 0U)
    {
        return;
    }

    (void)HAL_UART_Transmit(
        &huart1,
        (uint8_t *)text,
        (uint16_t)length,
        1000U
    );
}

/**
  * @brief USART1格式化输出
  */
static void Main_Debug_Printf(const char *format, ...)
{
    char buffer[256];
    va_list args;
    int length;

    if (format == NULL)
    {
        return;
    }

    va_start(args, format);

    length = vsnprintf(
        buffer,
        sizeof(buffer),
        format,
        args
    );

    va_end(args);

    if (length <= 0)
    {
        return;
    }

    if (length >= (int)sizeof(buffer))
    {
        length = (int)sizeof(buffer) - 1;
    }

    (void)HAL_UART_Transmit(
        &huart1,
        (uint8_t *)buffer,
        (uint16_t)length,
        1000U
    );
}

/**
  * @brief 清理以前残留的LSE/RTC备份域配置
  *
  * 只有检测到以下情况才会复位备份域：
  *
  * 1. RTC当前不是LSI时钟；
  * 2. LSE仍处于开启状态。
  *
  * 正常切换到LSI以后，后续普通复位不会重复清空RTC。
  */
static void RTC_BackupDomain_Prepare(void)
{
    uint32_t rtc_clock_source;
    uint32_t lse_state;
    RCC_OscInitTypeDef osc_config = {0};

    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_PWR_EnableBkUpAccess();

    rtc_clock_source =
        READ_BIT(RCC->BDCR, RCC_BDCR_RTCSEL);

    lse_state =
        READ_BIT(RCC->BDCR, RCC_BDCR_LSEON);

    if ((rtc_clock_source != RCC_RTCCLKSOURCE_LSI) ||
        (lse_state != 0U))
    {
        Main_Debug_Print(
            "[BOOT] Reset old RTC/LSE backup configuration\r\n"
        );

        /*
         * 清除旧RTC时钟选择、旧LSE开关状态。
         */
        __HAL_RCC_BACKUPRESET_FORCE();

        __DSB();
        __ISB();

        __HAL_RCC_BACKUPRESET_RELEASE();

        __DSB();
        __ISB();

        /*
         * 备份域复位可能同时关闭LSI，
         * 所以重新启动LSI。
         */
        osc_config.OscillatorType =
            RCC_OSCILLATORTYPE_LSI;

        osc_config.LSIState =
            RCC_LSI_ON;

        osc_config.LSIDiv =
            RCC_LSI_DIV1;

        osc_config.PLL.PLLState =
            RCC_PLL_NONE;

        if (HAL_RCC_OscConfig(&osc_config) != HAL_OK)
        {
            g_boot_stage = 91U;
            Error_Handler();
        }
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration------------------------------------------------------*/

    /*
     * 初始化HAL、Flash接口和HAL时基。
     */
    HAL_Init();

    g_boot_stage = 1U;

    /*
     * 关键设计：
     *
     * 在SystemClock_Config之前，先利用复位后的默认MSI时钟
     * 初始化GPIO和USART1。
     *
     * 因此普通复位后，只要进入main，就会立即输出BOOT 00。
     */
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    g_boot_stage = 2U;

    Main_Debug_Print("\r\n");
    Main_Debug_Print(
        "========================================\r\n"
    );
    Main_Debug_Print(
        "[BOOT 00] main entered before clock config\r\n"
    );

    /*
     * 配置12MHz HSE和160MHz PLL。
     */
    g_boot_stage = 20U;

    SystemClock_Config();

    g_boot_stage = 3U;

    /*
     * 系统时钟从默认MSI切到160MHz后，
     * USART1波特率寄存器需要重新计算。
     */
    MX_USART1_UART_Init();

    Main_Debug_Print(
        "[BOOT 01] SystemClock_Config OK\r\n"
    );

    Main_Debug_Printf(
        "[BOOT CLOCK] SYSCLK=%lu HCLK=%lu "
        "PCLK1=%lu PCLK2=%lu Hz\r\n",
        (unsigned long)HAL_RCC_GetSysClockFreq(),
        (unsigned long)HAL_RCC_GetHCLKFreq(),
        (unsigned long)HAL_RCC_GetPCLK1Freq(),
        (unsigned long)HAL_RCC_GetPCLK2Freq()
    );

    /*
     * 电源配置。
     */
    g_boot_stage = 4U;

    SystemPower_Config();

    Main_Debug_Print(
        "[BOOT 02] SystemPower_Config OK\r\n"
    );

    /*
     * GPDMA必须在SPI初始化之前完成。
     */
    g_boot_stage = 5U;

    MX_GPDMA1_Init();

    Main_Debug_Print(
        "[BOOT 03] GPDMA1 OK\r\n"
    );

    /*
     * I2C1。
     */
    g_boot_stage = 6U;

    MX_I2C1_Init();

    Main_Debug_Print(
        "[BOOT 04] I2C1 OK\r\n"
    );

    /*
     * SPI1及其TX DMA。
     */
    g_boot_stage = 7U;

    MX_SPI1_Init();

    Main_Debug_Print(
        "[BOOT 05] SPI1 OK\r\n"
    );

    /*
     * 指令缓存。
     */
    g_boot_stage = 8U;

    MX_ICACHE_Init();

    Main_Debug_Print(
        "[BOOT 06] ICACHE OK\r\n"
    );

    /*
     * TIM2背光PWM。
     */
    g_boot_stage = 9U;

    MX_TIM2_Init();

    Main_Debug_Print(
        "[BOOT 07] TIM2 OK\r\n"
    );

    /*
     * 清除旧LSE残留，并确保RTC使用LSI。
     */
    g_boot_stage = 10U;

    RTC_BackupDomain_Prepare();

    Main_Debug_Print(
        "[BOOT 08] RTC backup domain ready\r\n"
    );

    /*
     * RTC。
     */
    g_boot_stage = 11U;

    MX_RTC_Init();

    Main_Debug_Print(
        "[BOOT 09] RTC OK\r\n"
    );

    /* USER CODE BEGIN 2 */

    /* USER CODE END 2 */

    /*
     * 初始化CMSIS-RTOS2内核。
     */
    g_boot_stage = 12U;

    Main_Debug_Print(
        "[BOOT 10] Kernel initialize\r\n"
    );

    osKernelInitialize();

    /*
     * 创建FreeRTOS任务、队列、互斥量等。
     */
    g_boot_stage = 13U;

    MX_FREERTOS_Init();

    Main_Debug_Print(
        "[BOOT 11] FreeRTOS objects OK\r\n"
    );

    /*
     * 启动调度器。
     */
    g_boot_stage = 14U;

    Main_Debug_Print(
        "[BOOT 12] Starting scheduler\r\n"
    );

    osKernelStart();

    /*
     * 正常情况下不会执行到这里。
     */
    g_boot_stage = 100U;

    Main_Debug_Print(
        "[ERROR] osKernelStart returned\r\n"
    );

    while (1)
    {
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /*
     * 160MHz需要Voltage Scale 1。
     */
    g_boot_stage = 21U;

    if (HAL_PWREx_ControlVoltageScaling(
            PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        Error_Handler();
    }

    /*
     * 时钟配置：
     *
     * HSE = 12MHz
     *
     * 12MHz / PLLM 3 = 4MHz
     * 4MHz × PLLN 40 = 160MHz
     * 160MHz / PLLR 1 = 160MHz SYSCLK
     *
     * LSI用于RTC。
     */
    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_LSI |
        RCC_OSCILLATORTYPE_HSE;

    RCC_OscInitStruct.HSEState =
        RCC_HSE_ON;

    RCC_OscInitStruct.LSIState =
        RCC_LSI_ON;

    RCC_OscInitStruct.LSIDiv =
        RCC_LSI_DIV1;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_ON;

    RCC_OscInitStruct.PLL.PLLSource =
        RCC_PLLSOURCE_HSE;

    RCC_OscInitStruct.PLL.PLLMBOOST =
        RCC_PLLMBOOST_DIV1;

    RCC_OscInitStruct.PLL.PLLM = 3;
    RCC_OscInitStruct.PLL.PLLN = 40;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 1;

    RCC_OscInitStruct.PLL.PLLRGE =
        RCC_PLLVCIRANGE_0;

    RCC_OscInitStruct.PLL.PLLFRACN = 0;

    g_boot_stage = 22U;

    if (HAL_RCC_OscConfig(
            &RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /*
     * SYSCLK、HCLK、PCLK1、PCLK2、PCLK3全部160MHz。
     */
    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2 |
        RCC_CLOCKTYPE_PCLK3;

    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_PLLCLK;

    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV1;

    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;

    RCC_ClkInitStruct.APB3CLKDivider =
        RCC_HCLK_DIV1;

    g_boot_stage = 23U;

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_4) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief Power Configuration
  * @retval None
  */
static void SystemPower_Config(void)
{
    /*
     * Disable the internal Pull-Up in Dead Battery
     * pins of UCPD peripheral.
     */
    HAL_PWREx_DisableUCPDDeadBattery();

    /* USER CODE BEGIN PWR */

    /* USER CODE END PWR */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief Period elapsed callback in non blocking mode
  */
void HAL_TIM_PeriodElapsedCallback(
    TIM_HandleTypeDef *htim
)
{
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */

    if (htim->Instance == TIM6)
    {
        HAL_IncTick();
    }

    /* USER CODE BEGIN Callback 1 */

    /* USER CODE END Callback 1 */
}

/**
  * @brief Error handler
  */
void Error_Handler(void)
{
    /*
     * USART1在SystemClock_Config之前已经初始化。
     * 因此大部分启动错误都可以直接打印。
     */
    if (g_boot_stage >= 2U)
    {
        Main_Debug_Printf(
            "\r\n[ERROR] Error_Handler stage=%lu\r\n",
            (unsigned long)g_boot_stage
        );

        Main_Debug_Printf(
            "[ERROR] RCC_CR=0x%08lX "
            "RCC_BDCR=0x%08lX\r\n",
            (unsigned long)RCC->CR,
            (unsigned long)RCC->BDCR
        );
    }

    __disable_irq();

    while (1)
    {
        /*
         * Debug时观察g_boot_stage：
         *
         * 20：准备配置系统时钟
         * 21：Voltage Scale配置失败
         * 22：HSE/LSI/PLL启动失败
         * 23：切换到PLL系统时钟失败
         * 5 ：GPDMA初始化失败
         * 6 ：I2C1初始化失败
         * 7 ：SPI1初始化失败
         * 8 ：ICACHE初始化失败
         * 9 ：TIM2初始化失败
         * 10：RTC备份域处理失败
         * 11：RTC初始化失败
         * 91：备份域复位后LSI重启失败
         */
    }
}

#ifdef USE_FULL_ASSERT

/**
  * @brief Reports source file and line of assert failure
  */
void assert_failed(
    uint8_t *file,
    uint32_t line
)
{
    Main_Debug_Printf(
        "[ASSERT] file=%s line=%lu\r\n",
        (char *)file,
        (unsigned long)line
    );
}

#endif /* USE_FULL_ASSERT */