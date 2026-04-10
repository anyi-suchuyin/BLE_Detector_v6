# Step 2: LVGL 不同字号字体测试

## 更新内容
在 Step 1 成功点亮水墨屏的基础上，本步骤添加了 **LVGL 多字号字体显示**功能。

### 新增功能
- 使用 LVGL 内置的 Montserrat 字体族
- 显示 5 种不同字号：8px, 12px, 16px, 20px, 24px
- 每个字号显示对应的测试文字
- 添加标题和底部说明

### UI 布局
```
┌─────────────────────────────────┐
│     Font Size Test (16px)       │
│                                 │
│ 8px Hello World                 │
│ 12px Hello World                │
│ 16px Hello World                │
│ 20px Hello                      │
│ 24px Hi                         │
│                                 │
│      LVGL V8 + e-Paper          │
└─────────────────────────────────┘
```

## 使用的 LVGL 字体
| 字体名称 | 大小 | 用途 |
|---------|------|------|
| `lv_font_montserrat_8` | 8px | 小字提示信息 |
| `lv_font_montserrat_10` | 10px | 底部说明 |
| `lv_font_montserrat_12` | 12px | 常规文本 |
| `lv_font_montserrat_16` | 16px | 标题/正文 |
| `lv_font_montserrat_20` | 20px | 大标题 |
| `lv_font_montserrat_24` | 24px | 超大标题 |

## 编译与上传

### Arduino IDE
1. 打开 `step1_lvgl_test.ino`
2. 确保已安装 lvgl 库 (版本 8.3.x)
3. 选择 ESP32S3 Dev Module
4. 启用 PSRAM (Tools → PSRAM → "OPI PSRAM" 或 "Enabled")
5. 编译并上传

### PlatformIO
```bash
cd step1_lvgl_test
pio run --target upload
pio device monitor
```

## 预期效果
- 串口输出：
  ```
  I APP: e-Paper initialized
  I APP: LVGL UI initialized - Font size test
  I APP: Font size test UI created with LVGL labels
  I APP: Displayed fonts: 8px, 12px, 16px, 20px, 24px
  [OK] Setup complete!
  ```

- 屏幕显示：
  - 白色背景
  - 顶部居中显示 "Font Size Test" (16px)
  - 左侧对齐显示 5 行不同字号的测试文字
  - 底部居中显示 "LVGL V8 + e-Paper" (10px)

## 注意事项
1. 水墨屏刷新较慢，首次显示可能需要 2-3 秒
2. 局部刷新可能会有轻微残影，属正常现象
3. 如需完全清除残影，可执行一次全局刷新

## 下一步
- Step 3: 添加按钮、滑块等 LVGL 控件
- Step 4: 实现触摸交互（如硬件支持）
- Step 5: 多页面切换与动画效果
