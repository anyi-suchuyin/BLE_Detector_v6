# Step 1: 使用 LVGL 库点亮水墨屏 - 完整开发指南

## 项目概述

本教程基于 Waveshare ESP32-S3-ePaper-1.54 开发板，演示如何使用 LVGL V8 图形库驱动 1.54 寸 e-Paper 水墨屏。

**目标**: 验证硬件正常，屏幕能显示文字和图形。

**参考官方示例**: https://github.com/waveshareteam/ESP32-S3-ePaper-1.54/tree/main/Example/Arduino/09_LVGL_V8_Test

---

## 一、硬件信息

### 1.1 设备规格
- **屏幕**: 1.54 寸 e-Paper 电子纸
- **分辨率**: 200×200 像素
- **主控**: ESP32-S3 (带 PSRAM)
- **接口**: SPI

### 1.2 引脚定义

| 功能 | GPIO | 说明 |
|------|------|------|
| EPD_CS | IO11 | e-Paper 片选 |
| EPD_DC | IO10 | 数据/命令选择 |
| EPD_RST | IO9 | 复位 |
| EPD_BUSY | IO8 | 忙信号 |
| EPD_MOSI | IO13 | SPI 数据 |
| EPD_SCK | IO12 | SPI 时钟 |
| EPD_PWR | IO6 | 电源使能 (低电平开启) |

---

## 二、项目文件结构

```
step1_lvgl_test/
├── step1_lvgl_test.ino      # 主程序入口 (setup/loop)
├── user_app.cpp             # 应用层初始化
├── user_app.h               # 应用层头文件
├── user_config.h            # 硬件配置宏定义
├── platformio.ini           # PlatformIO 配置 (可选)
├── README.md                # 项目说明文档
└── src/
    ├── display/
    │   ├── epaper_driver_bsp.h    # e-Paper 驱动类声明
    │   └── epaper_driver_bsp.cpp  # e-Paper 驱动实现
    └── power/
        ├── board_power_bsp.h      # 电源管理类声明
        └── board_power_bsp.cpp    # 电源管理实现
```

---

## 三、核心代码解析

### 3.1 硬件配置 (user_config.h)

```cpp
// 屏幕尺寸
#define EPD_WIDTH  200
#define EPD_HEIGHT 200

// LVGL 缓冲区 (SPIRAM 中分配)
#define LVGL_SPIRAM_BUFF_LEN (EPD_WIDTH * EPD_HEIGHT * 2)

// e-Paper 引脚
#define EPD_DC_PIN    GPIO_NUM_10
#define EPD_CS_PIN    GPIO_NUM_11
#define EPD_SCK_PIN   GPIO_NUM_12
#define EPD_MOSI_PIN  GPIO_NUM_13
#define EPD_RST_PIN   GPIO_NUM_9
#define EPD_BUSY_PIN  GPIO_NUM_8
#define EPD_PWR_PIN   GPIO_NUM_6
```

### 3.2 e-Paper 驱动类 (epaper_driver_bsp.h/cpp)

**关键方法**:
- `EPD_Init()`: 初始化屏幕 (发送初始化命令序列)
- `EPD_Clear()`: 清屏 (填充白色)
- `EPD_DrawColorPixel(x, y, color)`: 绘制单个像素
- `EPD_DisplayPart()`: 局部刷新显示
- `EPD_DisplayPartBaseImage()`: 设置基础图像 (用于局部刷新)

**波形表**:
- `WF_Full_1IN54[]`: 全刷波形 (速度慢，残影少)
- `WF_PARTIAL_1IN54_0[]`: 局部刷新波形 (速度快，可能有残影)

### 3.3 LVGL 集成 (step1_lvgl_test.ino)

**LVGL 初始化流程**:
1. `lv_init()`: 初始化 LVGL 库
2. 分配双缓冲 (SPIRAM)
3. `lv_disp_drv_init()`: 初始化显示驱动
4. 设置 `flush_cb` 回调函数
5. `lv_disp_drv_register()`: 注册驱动

**刷新回调函数**:
```cpp
static void example_lvgl_flush_cb(lv_disp_drv_t *drv, 
                                   const lv_area_t *area, 
                                   lv_color_t *color_map) {
    // 1. 清屏
    driver->EPD_Clear();
    
    // 2. 逐像素渲染 LVGL 缓冲区到 e-Paper
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            uint8_t color = (*buffer < 0x7fff) ? BLACK : WHITE;
            driver->EPD_DrawColorPixel(x, y, color);
            buffer++;
        }
    }
    
    // 3. 刷新显示
    driver->EPD_DisplayPart();
    
    // 4. 通知 LVGL 完成
    lv_disp_flush_ready(drv);
}
```

### 3.4 应用初始化 (user_app.cpp)

```cpp
void user_app_init(void) {
    // 1. 开启 e-Paper 电源
    board_div.POWEER_EPD_ON();
    
    // 2. 配置 SPI 参数
    custom_lcd_spi_t driver_config = {};
    driver_config.cs = EPD_CS_PIN;
    driver_config.dc = EPD_DC_PIN;
    // ... 其他引脚配置
    
    // 3. 创建驱动实例
    driver = new epaper_driver_display(EPD_WIDTH, EPD_HEIGHT, driver_config);
    
    // 4. 初始化屏幕
    driver->EPD_Init();
    driver->EPD_Clear();
    driver->EPD_DisplayPartBaseImage();
    driver->EPD_Init_Partial();  // 启用局部刷新
}
```

---

## 四、编译与上传

### 4.1 Arduino IDE 方式

1. **安装 ESP32 开发板支持**
   - 文件 → 首选项 → 附加开发板管理器 URL
   - 添加：`https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   - 工具 → 开发板 → 开发板管理器 → 搜索 "ESP32" → 安装

2. **安装 LVGL 库**
   - 项目 → 加载库 → 管理库
   - 搜索 "lvgl" → 安装版本 8.3.x

3. **配置开发板**
   - 工具 → 开发板 → ESP32S3 Dev Module
   - PSRAM: Enabled
   - USB CDC On Boot: Enabled

4. **编译上传**
   - 打开 `step1_lvgl_test.ino`
   - 点击上传按钮

### 4.2 PlatformIO 方式

1. **安装 PlatformIO**
   ```bash
   pip install platformio
   ```

2. **编译**
   ```bash
   cd step1_lvgl_test
   pio run
   ```

3. **上传**
   ```bash
   pio run --target upload
   ```

4. **串口监控**
   ```bash
   pio device monitor
   ```

---

## 五、预期效果

### 5.1 串口输出
```
================================
ESP32-S3 ePaper LVGL Step 1
Test: Power on e-Paper display
================================
[OK] Setup complete!
```

### 5.2 屏幕显示
- 初始全黑闪烁 (初始化)
- 然后显示白色背景 + 黑色矩形边框
- 边框位置：距离边缘 10 像素
- 边框大小：180×180 像素

---

## 六、常见问题排查

### Q1: 屏幕无任何显示
**检查**:
1. GPIO6 是否正确输出低电平 (开启 e-Paper 电源)
2. SPI 接线是否正确
3. 串口打印是否有 "e-Paper initialized" 信息

### Q2: 编译错误 "lvgl.h not found"
**解决**:
- Arduino IDE: 通过库管理器安装 lvgl
- PlatformIO: 确保 `platformio.ini` 中有 `lib_deps = lvgl/lvgl@^8.3.0`

### Q3: 花屏/乱码
**可能原因**:
1. SPI 时钟频率过高 → 降低 `clock_speed_hz` 到 20MHz
2. 缓冲区未正确分配 → 检查 PSRAM 是否启用
3. 波形表不匹配 → 确认使用的是 1.54 寸屏幕的波形

### Q4: 刷新速度极慢
**优化**:
- 使用局部刷新 (`EPD_DisplayPart()` 而非 `EPD_Display()`)
- 减少 LVGL 动画复杂度
- 调整 `EXAMPLE_LVGL_TASK_MAX_DELAY_MS`

---

## 七、下一步扩展

完成 Step 1 后，可以继续开发:

### Step 2: BLE 扫描验证
- 添加 BLE 扫描功能
- 串口打印周围设备 MAC、RSSI

### Step 3: LVGL 图形界面
- 添加按钮、标签等控件
- 显示蓝牙设备列表
- 实现触摸交互 (如有触摸屏)

### Step 4: 人物库 + Web 配置
- 集成 WiFi AP 模式
- 创建 Web 配置界面
- 存储设备指纹到 SPIFFS/NVS

---

## 八、参考资料

- [Waveshare 官方 GitHub](https://github.com/waveshareteam/ESP32-S3-ePaper-1.54)
- [LVGL V8 文档](https://docs.lvgl.io/8.3/)
- [ESP32 Arduino 核心文档](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [本项目代码仓库](./step1_lvgl_test/)

---

*文档版本：1.0 | 更新日期：2024*
