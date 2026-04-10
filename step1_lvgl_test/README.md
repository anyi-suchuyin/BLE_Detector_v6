# Step 1: 点亮水墨屏 (LVGL V8)

## 项目说明

这是 ESP32-S3-ePaper-1.54 的 LVGL Step 1 示例代码，用于验证 e-Paper 屏幕能正常显示。

### 功能
- ✅ 初始化 e-Paper 显示屏 (200x200, 1.54 寸)
- ✅ 集成 LVGL V8 图形库
- ✅ 显示测试图案 (矩形边框)
- ✅ 支持局部刷新

### 硬件要求
- ESP32-S3-ePaper-1.54 开发板
- 需要安装 Arduino IDE 或 PlatformIO

### 依赖库

在 Arduino IDE 中安装以下库：

1. **lvgl** (版本 8.x)
   - 通过库管理器搜索 "lvgl"
   - 安装版本 8.3.x 或更高

2. **ESP32 by Espressif**
   - 确保使用最新版本的 ESP32 核心

### 文件结构

```
step1_lvgl_test/
├── step1_lvgl_test.ino      # 主程序入口
├── user_app.cpp             # 应用初始化
├── user_app.h               # 应用头文件
├── user_config.h            # 硬件配置
└── src/
    ├── display/
    │   ├── epaper_driver_bsp.h    # e-Paper 驱动头文件
    │   └── epaper_driver_bsp.cpp  # e-Paper 驱动实现
    └── power/
        ├── board_power_bsp.h      # 电源管理头文件
        └── board_power_bsp.cpp    # 电源管理实现
```

### 引脚配置

根据 `user_config.h`:

| 功能 | 引脚 | 说明 |
|------|------|------|
| EPD_CS | GPIO11 | e-Paper 片选 |
| EPD_DC | GPIO10 | e-Paper 数据/命令 |
| EPD_RST | GPIO9 | e-Paper 复位 |
| EPD_BUSY | GPIO8 | e-Paper 忙信号 |
| EPD_MOSI | GPIO13 | SPI 数据输出 |
| EPD_SCK | GPIO12 | SPI 时钟 |
| EPD_PWR | GPIO6 | e-Paper 电源使能 |

### 编译步骤

#### Arduino IDE
1. 打开 `step1_lvgl_test.ino`
2. 选择开发板：**Xiao ESP32S3** 或 **ESP32S3 Dev Module**
3. 安装 lvgl 库 (库管理器搜索 "lvgl")
4. 点击编译/上传按钮

#### PlatformIO
```ini
[env:esp32s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
lib_deps = 
    lvgl/lvgl@^8.3.0
```

### 预期输出

#### 串口输出
```
================================
ESP32-S3 ePaper LVGL Step 1
Test: Power on e-Paper display
================================
[OK] Setup complete!
```

#### 屏幕显示
- 清屏后显示一个黑色矩形边框
- 边框大小：180x180 像素 (从坐标 10,10 到 190,190)

### 故障排除

1. **屏幕无显示**
   - 检查 GPIO6 是否正确开启 e-Paper 电源
   - 确认 SPI 接线正确

2. **编译错误**
   - 确保安装了 lvgl 库 (版本 8.x)
   - 检查 ESP32 核心是否为最新版本

3. **花屏/乱码**
   - 检查 SPI 时钟频率是否过高
   - 尝试降低 `epaper_driver_bsp.cpp` 中的 `clock_speed_hz`

### 下一步

完成 Step 1 后，可以继续开发：
- Step 2: BLE 扫描验证
- Step 3: 人物库 + Web 配置
- Step 4: 水墨屏状态机 + 按钮捕获

### 参考资源

- [官方 GitHub](https://github.com/waveshareteam/ESP32-S3-ePaper-1.54)
- [官方 LVGL 示例](https://github.com/waveshareteam/ESP32-S3-ePaper-1.54/tree/main/Example/Arduino/09_LVGL_V8_Test)
- [LVGL 文档](https://docs.lvgl.io/)
