#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "user_app.h"
#include "driver/gpio.h"
#include "user_config.h"
#include "esp_log.h"
#include "esp_err.h"

#include "lvgl.h"
#include "src/power/board_power_bsp.h"

// 全局驱动实例
epaper_driver_display *driver = NULL;

// 电源管理实例
board_power_bsp_t board_div(EPD_PWR_PIN, Audio_PWR_PIN, VBAT_PWR_PIN);

/**
 * @brief 应用初始化
 * - 开启 e-Paper 和音频电源
 * - 初始化 e-Paper 显示屏
 */
void user_app_init(void) {
    // 开启电源
    board_div.POWEER_EPD_ON();
    board_div.POWEER_Audio_ON();
    
    // 配置 e-Paper SPI 参数
    custom_lcd_spi_t driver_config = {};
    driver_config.cs = EPD_CS_PIN;
    driver_config.dc = EPD_DC_PIN;
    driver_config.rst = EPD_RST_PIN;
    driver_config.busy = EPD_BUSY_PIN;
    driver_config.mosi = EPD_MOSI_PIN;
    driver_config.scl = EPD_SCK_PIN;
    driver_config.spi_host = EPD_SPI_NUM;
    driver_config.buffer_len = 5000;
    
    // 创建驱动实例并初始化
    driver = new epaper_driver_display(EPD_WIDTH, EPD_HEIGHT, driver_config);
    driver->EPD_Init();
    driver->EPD_Clear();
    driver->EPD_DisplayPartBaseImage();
    driver->EPD_Init_Partial();  // 局部刷新初始化
    
    ESP_LOGI("APP", "e-Paper initialized");
}

/**
 * @brief UI 初始化 (Step 2: 显示不同字号的 LVGL 字体测试)
 * 使用 LVGL 内置字体显示 12px, 16px, 20px 文字
 * 注意：8px 字体在 e-Paper 上显示不清晰，已移除
 */
void user_ui_init(void) {
    ESP_LOGI("APP", "LVGL UI initialized - Font size test (12/16/20px only)");
    
    // 创建主容器
    lv_obj_t *main_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(main_container, lv_color_white(), 0);
    lv_obj_set_style_border_width(main_container, 0, 0);
    
    // 标题
    lv_obj_t *title = lv_label_create(main_container);
    lv_label_set_text(title, "Font Size Test");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    
    // 第 1 行：12px 字体
    lv_obj_t *label_12px = lv_label_create(main_container);
    lv_label_set_text(label_12px, "12px Hello World");
    lv_obj_set_style_text_font(label_12px, &lv_font_montserrat_12, 0);
    lv_obj_align(label_12px, LV_ALIGN_TOP_LEFT, 10, 35);
    
    // 第 2 行：16px 字体
    lv_obj_t *label_16px = lv_label_create(main_container);
    lv_label_set_text(label_16px, "16px Hello World");
    lv_obj_set_style_text_font(label_16px, &lv_font_montserrat_16, 0);
    lv_obj_align(label_16px, LV_ALIGN_TOP_LEFT, 10, 65);
    
    // 第 3 行：20px 字体
    lv_obj_t *label_20px = lv_label_create(main_container);
    lv_label_set_text(label_20px, "20px Hello World");
    lv_obj_set_style_text_font(label_20px, &lv_font_montserrat_20, 0);
    lv_obj_align(label_20px, LV_ALIGN_TOP_LEFT, 10, 100);
    
    // 底部说明
    lv_obj_t *footer = lv_label_create(main_container);
    lv_label_set_text(footer, "LVGL V8 + e-Paper (8px removed)");
    lv_obj_set_style_text_font(footer, &lv_font_montserrat_12, 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -5);
    
    ESP_LOGI("APP", "Font size test UI created with LVGL labels");
    ESP_LOGI("APP", "Displayed fonts: 12px, 16px, 20px (8px removed for clarity)");
}
