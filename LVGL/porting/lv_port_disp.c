/**
 * @file lv_port_disp.c
 * @brief LVGL 8.3 + ILI9341 + SPI1 GPDMA asynchronous display port
 */

#if 1

#include "lv_port_disp.h"

#include "bsp_ili9341_4line.h"
#include "spi.h"

#include <stdbool.h>
#include <stdint.h>

/* LCD resolution */
#define LCD_HOR_RES                 320U
#define LCD_VER_RES                 240U

/*
 * 每块绘图缓冲20行：
 * 320 × 20 × 2 = 12800 bytes
 *
 * 双绘图缓冲：
 * DMA发送第一个缓冲对应的数据时，
 * LVGL可以在第二个缓冲继续绘图。
 */
#define LCD_DRAW_BUF_LINES          20U
#define LCD_DRAW_BUF_PIXELS         (LCD_HOR_RES * LCD_DRAW_BUF_LINES)

/*
 * 独立DMA发送缓冲。
 *
 * 不直接修改LVGL提供的color_p，
 * 避免下一次局部刷新时出现颜色错乱、横条和花屏。
 */
#define LCD_DMA_BUF_PIXELS          LCD_DRAW_BUF_PIXELS

/* LVGL prototypes */
static void disp_init(void);

static void disp_flush(
    lv_disp_drv_t *disp_drv,
    const lv_area_t *area,
    lv_color_t *color_p
);

static void LCD_CopyRGB565ToDMABuffer(
    const lv_color_t *source,
    uint32_t pixel_count
);

/* LVGL draw buffers */
static lv_disp_draw_buf_t lcd_draw_buf;

__attribute__((aligned(4)))
static lv_color_t lcd_draw_buf_1[LCD_DRAW_BUF_PIXELS];

__attribute__((aligned(4)))
static lv_color_t lcd_draw_buf_2[LCD_DRAW_BUF_PIXELS];

/*
 * 独立DMA像素缓冲。
 *
 * uint16_t形式便于CPU快速进行RGB565字节交换，
 * 发送时仍按uint8_t BYTE DMA传输。
 */
__attribute__((aligned(4)))
static uint16_t lcd_dma_buffer[LCD_DMA_BUF_PIXELS];

/* 当前正在等待DMA完成的LVGL驱动 */
static lv_disp_drv_t * volatile lcd_active_flush_drv = NULL;

/* DMA工作状态 */
static volatile uint8_t lcd_dma_busy = 0U;

/* 是否允许屏幕刷新 */
volatile bool disp_flush_enabled = true;

/* 调试统计，可放入Keil Watch */
volatile uint32_t g_lcd_dma_flush_count = 0U;
volatile uint32_t g_lcd_dma_start_error_count = 0U;
volatile uint32_t g_lcd_dma_transfer_error_count = 0U;
volatile uint32_t g_lcd_dma_busy_error_count = 0U;
volatile uint32_t g_lcd_dma_last_pixel_count = 0U;

/**
 * @brief LVGL display port initialization
 */
void lv_port_disp_init(void)
{
    static lv_disp_drv_t disp_drv;

    disp_init();

    /*
     * 双缓冲，缓冲大小为20行。
     */
    lv_disp_draw_buf_init(
        &lcd_draw_buf,
        lcd_draw_buf_1,
        lcd_draw_buf_2,
        LCD_DRAW_BUF_PIXELS
    );

    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = (lv_coord_t)LCD_HOR_RES;
    disp_drv.ver_res = (lv_coord_t)LCD_VER_RES;

    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &lcd_draw_buf;

    /*
     * 不强制整屏刷新。
     * 静止页面更新时间、心率、温度时只刷新变化区域。
     */
    disp_drv.full_refresh = 0;
    disp_drv.direct_mode = 0;

    (void)lv_disp_drv_register(&disp_drv);
}

/**
 * @brief LCD hardware initialization
 */
static void disp_init(void)
{
    ILI9341_Init();
}

/**
 * @brief Enable display flush
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/**
 * @brief Disable display flush
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/**
 * @brief 把LVGL RGB565复制到独立DMA缓冲并交换高低字节
 *
 * LV_COLOR_16_SWAP继续保持0。
 *
 * STM32内存中的0xF800：
 * 00 F8
 *
 * ILI9341 SPI需要：
 * F8 00
 */
static void LCD_CopyRGB565ToDMABuffer(
    const lv_color_t *source,
    uint32_t pixel_count)
{
    const uint16_t *source_pixels;
    uint32_t index;
    uint16_t pixel;

    source_pixels = (const uint16_t *)source;

    for (index = 0U; index < pixel_count; index++)
    {
        pixel = source_pixels[index];

        lcd_dma_buffer[index] =
            (uint16_t)((pixel << 8) | (pixel >> 8));
    }
}

/**
 * @brief LVGL display flush callback
 *
 * 这里只启动DMA，不等待DMA完成。
 * DMA完成后由HAL_SPI_TxCpltCallback通知LVGL。
 */
static void disp_flush(
    lv_disp_drv_t *disp_drv,
    const lv_area_t *area,
    lv_color_t *color_p)
{
    uint32_t width;
    uint32_t height;
    uint32_t pixel_count;
    uint32_t byte_count;
    HAL_StatusTypeDef status;

    if (disp_flush_enabled == false)
    {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    if ((disp_drv == NULL) ||
        (area == NULL) ||
        (color_p == NULL) ||
        (area->x2 < area->x1) ||
        (area->y2 < area->y1))
    {
        g_lcd_dma_start_error_count++;
        lv_disp_flush_ready(disp_drv);
        return;
    }

    /*
     * 正常情况下LVGL不会在上一次flush完成前再次进入。
     */
    if (lcd_dma_busy != 0U)
    {
        g_lcd_dma_busy_error_count++;

        /*
         * 防止LVGL永久等待。
         * 若该计数增加，说明DMA完成中断没有正常执行。
         */
        lv_disp_flush_ready(disp_drv);
        return;
    }

    width =
        (uint32_t)(area->x2 - area->x1 + 1);

    height =
        (uint32_t)(area->y2 - area->y1 + 1);

    pixel_count = width * height;
    byte_count = pixel_count * 2U;

    /*
     * 本方案LVGL每次最多提供20行。
     */
    if ((pixel_count == 0U) ||
        (pixel_count > LCD_DMA_BUF_PIXELS) ||
        (byte_count > 65535U))
    {
        g_lcd_dma_start_error_count++;
        lv_disp_flush_ready(disp_drv);
        return;
    }

    /*
     * 设置ILI9341本次写入区域。
     * 这些命令使用阻塞SPI发送，但数据量非常小。
     */
    ILI9341_SetArea(
        (uint16_t)area->x1,
        (uint16_t)area->y1,
        (uint16_t)area->x2,
        (uint16_t)area->y2
    );

    ILI9341_WriteRAM_Prepare();

    /*
     * 不修改LVGL原始color_p。
     */
    LCD_CopyRGB565ToDMABuffer(
        color_p,
        pixel_count
    );

    g_lcd_dma_last_pixel_count = pixel_count;

    lcd_active_flush_drv = disp_drv;
    lcd_dma_busy = 1U;

    /*
     * SPI固定8位，DMA固定BYTE。
     * Size参数是字节数量。
     */
    status = HAL_SPI_Transmit_DMA(
        &hspi1,
        (uint8_t *)lcd_dma_buffer,
        (uint16_t)byte_count
    );

    if (status != HAL_OK)
    {
        lcd_active_flush_drv = NULL;
        lcd_dma_busy = 0U;

        g_lcd_dma_start_error_count++;

        lv_disp_flush_ready(disp_drv);
    }
}

/**
 * @brief SPI1 DMA transmission complete callback
 *
 * 注意：全工程只能有一个HAL_SPI_TxCpltCallback定义。
 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    lv_disp_drv_t *completed_drv;

    if ((hspi == NULL) ||
        (hspi->Instance != SPI1))
    {
        return;
    }

    completed_drv =
        (lv_disp_drv_t *)lcd_active_flush_drv;

    lcd_active_flush_drv = NULL;
    lcd_dma_busy = 0U;

    g_lcd_dma_flush_count++;

    if (completed_drv != NULL)
    {
        /*
         * 告诉LVGL该缓冲已经发送完成。
         */
        lv_disp_flush_ready(completed_drv);
    }
}

/**
 * @brief SPI1 transfer error callback
 */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    lv_disp_drv_t *error_drv;

    if ((hspi == NULL) ||
        (hspi->Instance != SPI1))
    {
        return;
    }

    error_drv =
        (lv_disp_drv_t *)lcd_active_flush_drv;

    lcd_active_flush_drv = NULL;
    lcd_dma_busy = 0U;

    g_lcd_dma_transfer_error_count++;

    /*
     * 即使发生错误也必须释放LVGL，
     * 否则后续所有刷新都会停止。
     */
    if (error_drv != NULL)
    {
        lv_disp_flush_ready(error_drv);
    }
}

#else

typedef int keep_pedantic_happy;

#endif