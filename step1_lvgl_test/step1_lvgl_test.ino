/*
 * 项目：ESP32-S3 ePaper 1.54 LVGL Step 1 - 点亮水墨屏
 * 硬件：ESP32-S3-ePaper-1.54
 * 功能：验证 e-Paper 屏幕能正常显示
 * 版本：1.0.0
 * 
 * 参考官方示例:
 * https://github.com/waveshareteam/ESP32-S3-ePaper-1.54/tree/main/Example/Arduino/09_LVGL_V8_Test
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "user_config.h"
#include "lvgl.h"
#include "user_app.h"

// LVGL 相关定义
static const char *TAG = "main";
static SemaphoreHandle_t lvgl_mux = NULL;

/**
 * @brief LVGL 显示刷新回调
 * 将 LVGL 的图形缓冲区渲染到 e-Paper
 */
static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    uint16_t *buffer = (uint16_t *)color_map;
    
    // 清屏
    driver->EPD_Clear();
    
    // 逐像素渲染
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            uint8_t color = (*buffer < 0x7fff) ? DRIVER_COLOR_BLACK : DRIVER_COLOR_WHITE;
            driver->EPD_DrawColorPixel(x, y, color);
            buffer++;
        }
    }
    
    // 局部刷新显示
    driver->EPD_DisplayPart();
    
    // 通知 LVGL 刷新完成
    lv_disp_flush_ready(drv);
}

/**
 * @brief LVGL 心跳定时器回调
 */
static void example_increase_lvgl_tick(void *arg) {
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

/**
 * @brief LVGL 端口任务
 * 运行 LVGL 定时器处理器
 */
static void example_lvgl_port_task(void *arg) {
    uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
    
    for (;;) {
        if (xSemaphoreTake(lvgl_mux, portMAX_DELAY) == pdTRUE) {
            task_delay_ms = lv_timer_handler();
            xSemaphoreGive(lvgl_mux);
        }
        
        // 限制延迟范围
        if (task_delay_ms > EXAMPLE_LVGL_TASK_MAX_DELAY_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < EXAMPLE_LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS;
        }
        
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

/**
 * @brief 初始化 LVGL 图形库
 */
static void lvgl_port(void) {
    static lv_disp_draw_buf_t disp_buf;      // 图形缓冲区
    static lv_disp_drv_t disp_drv;           // 显示驱动
    
    // 初始化 LVGL
    lv_init();
    
    // 分配 SPIRAM 中的图形缓冲区 (双缓冲)
    lv_color_t *buffer_1 = (lv_color_t *)heap_caps_malloc(LVGL_SPIRAM_BUFF_LEN, MALLOC_CAP_SPIRAM);
    lv_color_t *buffer_2 = (lv_color_t *)heap_caps_malloc(LVGL_SPIRAM_BUFF_LEN, MALLOC_CAP_SPIRAM);
    
    assert(buffer_1);
    assert(buffer_2);
    
    // 初始化显示缓冲区
    lv_disp_draw_buf_init(&disp_buf, buffer_1, buffer_2, EPD_WIDTH * EPD_HEIGHT);
    
    // 注册显示驱动
    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EPD_WIDTH;
    disp_drv.ver_res = EPD_HEIGHT;
    disp_drv.flush_cb = example_lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.full_refresh = 1;  // 必须设置为 1
    lv_disp_drv_register(&disp_drv);
    
    // 创建 LVGL 心跳定时器
    ESP_LOGI(TAG, "Install LVGL tick timer");
    esp_timer_create_args_t lvgl_tick_timer_args = {};
    lvgl_tick_timer_args.callback = &example_increase_lvgl_tick;
    lvgl_tick_timer_args.name = "lvgl_tick";
    
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));
    
    // 创建互斥锁
    lvgl_mux = xSemaphoreCreateMutex();
    assert(lvgl_mux);
    
    // 创建 LVGL 任务 (运行在 Core 1)
    xTaskCreatePinnedToCore(
        example_lvgl_port_task,  // 任务函数
        "LVGL",                  // 任务名称
        8 * 1024,                // 栈大小
        NULL,                    // 参数
        4,                       // 优先级
        NULL,                    // 任务句柄
        1                        // CPU 核心
    );
    
    // 初始化用户 UI
    if (xSemaphoreTake(lvgl_mux, portMAX_DELAY) == pdTRUE) {
        user_ui_init();
        xSemaphoreGive(lvgl_mux);
    }
}

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);  // 等待串口稳定
    
    Serial.println(F("================================"));
    Serial.println(F("ESP32-S3 ePaper LVGL Step 1"));
    Serial.println(F("Test: Power on e-Paper display"));
    Serial.println(F("================================"));
    
    // 初始化应用 (e-Paper 硬件)
    user_app_init();
    
    // 初始化 LVGL
    lvgl_port();
    
    Serial.println(F("[OK] Setup complete!"));
}

void loop() {
    // 主循环不需要做任何事
    // LVGL 任务会在后台运行
    delay(100);
}
