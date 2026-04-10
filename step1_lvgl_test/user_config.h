#ifndef USER_CONFIG_H
#define USER_CONFIG_H

// SPI 配置
#define EPD_SPI_NUM        SPI2_HOST

// 屏幕尺寸
#define EPD_WIDTH  200
#define EPD_HEIGHT 200

// LVGL 缓冲区大小 (SPIRAM)
#define LVGL_SPIRAM_BUFF_LEN (EPD_WIDTH * EPD_HEIGHT * 2)

// e-Paper 引脚定义 (根据接口信息.md)
#define EPD_DC_PIN    GPIO_NUM_10
#define EPD_CS_PIN    GPIO_NUM_11
#define EPD_SCK_PIN   GPIO_NUM_12
#define EPD_MOSI_PIN  GPIO_NUM_13
#define EPD_RST_PIN   GPIO_NUM_9
#define EPD_BUSY_PIN  GPIO_NUM_8

// 电源控制引脚
#define EPD_PWR_PIN     GPIO_NUM_6
#define Audio_PWR_PIN   GPIO_NUM_42
#define VBAT_PWR_PIN    GPIO_NUM_17

// 按钮引脚
#define BOOT_BUTTON_PIN GPIO_NUM_0

// LVGL 任务配置
#define EXAMPLE_LVGL_TICK_PERIOD_MS    5
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 500
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 100

#endif // !USER_CONFIG_H
