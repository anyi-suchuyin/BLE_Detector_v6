# Arduino AI 编码规范 v1.0

> 目标：确保AI生成的Arduino代码一次性编译通过，零警告，可直接烧录运行。

---

## 1. 基础铁律（必须严格遵守）

### 1.1 语法正确性
- **绝不省略分号**：每个语句必须以 `;` 结尾
- **大括号必须成对**：开括号 `{` 必须在同一行，闭括号 `}` 独占一行且与开括号缩进对齐
- **字符串必须用双引号**：Arduino/C++ 不支持单引号字符串
- **字符用单引号**，字符串用双引号：`char c = 'A'; String s = "Hello";`

### 1.2 头文件管理
```cpp
// ✅ 正确：标准库在前，第三方库次之，项目头文件最后
#include <Arduino.h>      // 始终显式包含（即使IDE会自动加）
#include <Wire.h>       // 硬件I2C
#include <SPI.h>        // 硬件SPI
#include <EEPROM.h>     // 内置存储

// 第三方库
#include <WiFi.h>       // ESP32/ESP8266
#include <PubSubClient.h>
#include <ArduinoJson.h>

// 项目自定义
#include "config.h"
#include "utils.h"
```

**禁止行为：**
- 假设任何库已默认包含
- 使用未声明的类或函数
- 包含不存在的头文件路径

### 1.3 类型系统严格规范
```cpp
// ✅ 明确指定整数大小
uint8_t  pinNumber = 13;      // 0-255，用于引脚、小计数
uint16_t sensorValue = 1023;  // ADC读取
uint32_t timestamp = 0;       // millis()/micros()
int16_t  temperature = -10;   // 可能为负的传感器值

// ✅ 浮点数明确精度
float  voltage = 3.3f;        // 普通精度，后缀f
float  reading = analogRead(A0) * (5.0f / 1023.0f);

// ❌ 禁止使用
int pin;                      // 平台相关，32位和8位MCU行为不同
word data;                    // 非标准类型，避免使用
```

---

## 2. 硬件抽象层（HAL）规范

### 2.1 引脚定义必须常量化
```cpp
// ✅ 硬件配置区（文件顶部，const + 类型明确）
const uint8_t PIN_LED_STATUS   = LED_BUILTIN;  // 使用内置定义
const uint8_t PIN_SENSOR_DHT   = 4;
const uint8_t PIN_MOTOR_PWM    = 9;            // PWM引脚验证过
const uint8_t PIN_I2C_SDA      = SDA;          // 使用板级定义
const uint8_t PIN_I2C_SCL      = SCL;

// 引脚能力检查（可选但推荐）
static_assert(PIN_MOTOR_PWM == 9 || PIN_MOTOR_PWM == 10, "PWM仅支持9/10");
```

### 2.2 平台检测与适配
```cpp
// ✅ 多平台兼容代码结构
#if defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  const uint32_t SERIAL_BAUD = 115200;
  #define BOARD_NAME "ESP32"
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  const uint32_t SERIAL_BAUD = 115200;
  #define BOARD_NAME "ESP8266"
#elif defined(AVR) || defined(__AVR_ATmega328P__)
  #include <avr/power.h>
  const uint32_t SERIAL_BAUD = 9600;
  #define BOARD_NAME "AVR-328P"
#else
  #error "不支持的硬件平台"
#endif
```

---

## 3. 内存管理规范（关键！）

### 3.1 栈内存限制
```cpp
// ✅ 大数组必须静态或全局
float sensorBuffer[1000];  // 放在全局区，4KB在SRAM

void loop() {
  // ❌ 禁止：大数组在栈上
  // float tempBuffer[1000];  // 可能导致栈溢出！

  // ✅ 使用动态分配（谨慎）或分段处理
  static float processedData[100];  // 静态分配
  processChunk(processedData, 100);
}
```

### 3.2 字符串与动态内存
```cpp
// ✅ 使用F()宏存储常量字符串到Flash（PROGMEM）
Serial.println(F("System initialized"));

// ✅ ArduinoJson 使用静态分配
StaticJsonDocument<256> doc;  // 栈上256字节，确定大小
// ❌ 避免：DynamicJsonDocument（堆碎片风险）

// ✅ String类使用限制
String message;
message.reserve(100);  // 预分配避免碎片化
message = F("Status: ");
message += sensorValue;
```

### 3.3 指针安全
```cpp
// ✅ 所有指针初始化，检查空值
uint8_t* buffer = nullptr;

void initBuffer() {
  buffer = (uint8_t*)malloc(BUFFER_SIZE);
  if (buffer == nullptr) {
    Serial.println(F("ERR: 内存分配失败"));
    return;
  }
  memset(buffer, 0, BUFFER_SIZE);
}

// ✅ 释放后归零
free(buffer);
buffer = nullptr;  // 防止野指针
```

---

## 4. 实时性与中断规范

### 4.1 非阻塞代码原则
```cpp
// ✅ 使用状态机替代delay()
enum State { IDLE, WARMING, MEASURING, SENDING };
State currentState = IDLE;
uint32_t stateTimer = 0;

void loop() {
  uint32_t now = millis();

  switch(currentState) {
    case WARMING:
      if (now - stateTimer >= 2000) {  // 2秒预热
        currentState = MEASURING;
      }
      break;
    // ... 其他状态
  }
}

// ❌ 绝对禁止
// delay(1000);  // 阻塞CPU，影响响应
```

### 4.2 中断服务程序（ISR）铁律
```cpp
// ✅ ISR必须简单、快速、无阻塞
volatile uint32_t pulseCount = 0;  // volatile必须！
volatile bool dataReady = false;

void IRAM_ATTR onPulse() {  // ESP32需IRAM_ATTR
  pulseCount++;
  dataReady = true;
}

void setup() {
  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR), onPulse, RISING);
}

void loop() {
  if (dataReady) {
    noInterrupts();           // 关中断保护复制
    uint32_t count = pulseCount;
    dataReady = false;
    interrupts();             // 开中断

    processData(count);       // 长操作在主循环
  }
}
```

**ISR禁止操作：**
- `delay()`、`millis()`（除ESP32外）
- `Serial.print()`、`printf()`
- `malloc()`、`free()`、`new`、`delete`
- 浮点运算（AVR上极慢）

---

## 5. 库使用规范

### 5.1 实例化规范
```cpp
// ✅ 对象在全局区实例化，构造函数无参或常参
DHT dht(PIN_DHT, DHT22);           // 传感器对象
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void setup() {
  dht.begin();                     // 初始化在setup
  mqtt.setServer(MQTT_BROKER, 1883);
}
```

### 5.2 传感器读取规范
```cpp
// ✅ 检查读取有效性
float temperature = dht.readTemperature();
if (isnan(temperature)) {
  Serial.println(F("ERR: DHT读取失败"));
  temperature = lastValidTemp;  // 使用上一次有效值
} else {
  lastValidTemp = temperature;
}
```

---

## 6. 通信协议规范

### 6.1 I2C (Wire)
```cpp
#include <Wire.h>
const uint8_t I2C_ADDR = 0x23;  // 设备地址确认

bool readI2CRegister(uint8_t reg, uint8_t* data, size_t len) {
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {  // 不发送停止位
    return false;
  }

  Wire.requestFrom(I2C_ADDR, len);
  if (Wire.available() != len) {
    return false;
  }

  for (size_t i = 0; i < len; i++) {
    data[i] = Wire.read();
  }
  return true;
}
```

### 6.2 SPI
```cpp
#include <SPI.h>
const uint8_t PIN_CS = 10;  // 片选必须管理

void setup() {
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);  // 初始不选中
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV4);  // 设置速率
}

uint8_t spiTransfer(uint8_t data) {
  digitalWrite(PIN_CS, LOW);
  uint8_t result = SPI.transfer(data);
  digitalWrite(PIN_CS, HIGH);
  return result;
}
```

### 6.3 Serial
```cpp
// ✅ 波特率常量化，检查可用性
void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial && millis() < 3000);  // 等待连接，3秒超时

  Serial.println(F("
--- Boot ---"));
  Serial.print(F("Version: "));
  Serial.println(FIRMWARE_VERSION);
}
```

---

## 7. 调试与日志规范

### 7.1 调试宏（条件编译）
```cpp
// config.h
#define DEBUG_LEVEL 2  // 0=关, 1=错误, 2=信息, 3=调试

#if DEBUG_LEVEL >= 1
  #define LOG_ERROR(x)   Serial.println(F("[ERR] " x))
#else
  #define LOG_ERROR(x)
#endif

#if DEBUG_LEVEL >= 2
  #define LOG_INFO(x)    Serial.println(F("[INF] " x))
  #define LOG_INFO_VAR(x) Serial.println(x)
#else
  #define LOG_INFO(x)
  #define LOG_INFO_VAR(x)
#endif
```

### 7.2 看门狗与故障恢复
```cpp
#include <esp_task_wdt.h>  // ESP32看门狗

void setup() {
  #if defined(ESP32)
    esp_task_wdt_init(10, true);  // 10秒超时，panic重启
    esp_task_wdt_add(NULL);       // 添加当前任务
  #elif defined(AVR)
    wdt_enable(WDTO_8S);          // AVR 8秒看门狗
  #endif
}

void loop() {
  #if defined(ESP32)
    esp_task_wdt_reset();
  #elif defined(AVR)
    wdt_reset();
  #endif

  // 主逻辑
}
```

---

## 8. 项目结构模板

```
project/
├── project.ino              // 主文件，仅含setup/loop
├── config.h                 // 所有常量、宏定义
├── hardware.h/.cpp          // 引脚定义、硬件初始化
├── sensors.h/.cpp           // 传感器驱动封装
├── network.h/.cpp           // WiFi/蓝牙/通信
├── protocols.h/.cpp         // MQTT/HTTP/自定义协议
├── utils.h/.cpp             // 工具函数
└── arduino-guidelines.md    // 本规范
```

**主文件（.ino）结构：**
```cpp
/*
 * 项目: [名称]
 * 硬件: [具体型号，如 ESP32-S3-DevKitC-1]
 * 功能: [一句话描述]
 * 版本: 1.0.0
 */

#include "config.h"
#include "hardware.h"
#include "sensors.h"

void setup() {
  initHardware();
  initSensors();
  LOG_INFO("Boot complete");
}

void loop() {
  updateSensors();
  processLogic();
  handleCommunication();
  feedWatchdog();
}
```

---

## 9. 常见编译错误预防清单

| 错误类型 | 预防措施 |
|---------|---------|
| `not declared` | 每个函数前必须有原型声明或定义在使用前 |
| `does not name a type` | 检查头文件名拼写，确认库已安装 |
| `invalid operands` | 混合运算时强制类型转换 `(float)analog / 1023.0f` |
| `section exceeds` | SRAM不足，大数组改用`const`或`PROGMEM` |
| `undefined reference` | 函数定义与声明不匹配，检查参数类型 |
| `redefinition` | 头文件必须有`#pragma once`或`#ifndef`保护 |
| `conversion from pointer` | String转char*用`.c_str()`，检查API要求 |

---

## 10. AI生成自检提示（生成代码后自检）

生成代码后，AI必须验证：
1. [ ] 所有`{`都有匹配的`}`
2. [ ] 所有字符串用双引号，字符用单引号
3. [ ] 所有函数有返回类型，无默认`int`返回
4. [ ] `setup()`和`loop()`存在且格式正确
5. [ ] 没有`delay()`超过100ms（除非明确需求）
6. [ ] 所有库通过`#include`显式引入
7. [ ] 变量在首次使用前初始化
8. [ ] 数组访问不越界（循环条件严格`<`长度）
9. [ ] 浮点运算使用`f`后缀或明确double
10. [ ] 注释说明每个硬件引脚用途

---

## 快速参考：最小可编译模板

```cpp
#include <Arduino.h>

const uint8_t PIN_LED = LED_BUILTIN;
uint32_t lastBlink = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  Serial.println(F("Ready"));
}

void loop() {
  uint32_t now = millis();
  if (now - lastBlink >= 1000) {
    lastBlink = now;
    ledState = !ledState;
    digitalWrite(PIN_LED, ledState);
    Serial.print(F("LED: "));
    Serial.println(ledState ? F("ON") : F("OFF"));
  }
}
```

---

*规范版本: 1.0.0 | 适用: Arduino IDE 2.x / PlatformIO / Arduino-CLI*


==================================================
文档长度: 8489 字符