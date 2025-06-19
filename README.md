# Kevin Yuying 313 LCD MVP

这是一个最小可行产品(MVP)，用于演示 Kevin Yuying 313 LCD 开发板的基本显示功能。

## 硬件特性

- **开发板**: Kevin Yuying 313 LCD
- **主控芯片**: ESP32-S3
- **显示屏**: 376x960 RGB LCD，使用 GC9503 驱动芯片
- **背光控制**: PWM 背光控制，GPIO4

## 功能特性

- ✅ 点亮 LCD 屏幕
- ✅ LVGL 图形库集成
- ✅ 基本图形绘制
- ✅ 背光亮度控制
- ✅ 状态显示和通知

## 项目结构

```
mvp/
├── CMakeLists.txt          # 项目构建配置
├── sdkconfig.defaults      # ESP-IDF默认配置
├── idf_component.yml       # 组件依赖配置
├── main/
│   ├── main.cc            # 主程序入口
│   ├── config.h           # 硬件配置
│   ├── pin_config.h       # 引脚配置
│   ├── esp_lcd_gc9503.c   # GC9503显示驱动
│   ├── esp_lcd_gc9503.h   # GC9503驱动头文件
│   ├── board/             # 板级抽象层
│   ├── display/           # 显示驱动
│   └── backlight/         # 背光控制
└── README.md              # 说明文档
```

## 编译和烧录

1. 确保已安装 ESP-IDF 开发环境
2. 设置目标芯片为 ESP32-S3:
   ```bash
   idf.py set-target esp32s3
   ```
3. 编译项目:
   ```bash
   idf.py build
   ```
4. 烧录到开发板:
   ```bash
   idf.py flash monitor
   ```

## 功能说明

程序启动后会：

1. 初始化 ESP32-S3 系统
2. 配置 GC9503 RGB LCD 驱动
3. 初始化 LVGL 图形库
4. 点亮屏幕并显示测试内容
5. 设置背光亮度为 80%

屏幕上会显示：

- 红色文本："LVGL Working!"
- 屏幕分辨率信息
- 一个绿色矩形框

## 硬件连接

主要引脚连接：

- 背光控制: GPIO4
- RGB 数据线: GPIO3,8-21,46,47 等
- 同步信号: HSYNC(GPIO6), VSYNC(GPIO5), DE(GPIO15), PCLK(GPIO7)
- SPI 控制线: CS(GPIO48), SCL(GPIO17), SDA(GPIO16)

详细引脚定义请参考 `pin_config.h` 文件。

## 注意事项

- 项目需要使用 PSRAM 来存储 framebuffer
- 确保硬件连接正确
- 如有问题请检查串口输出日志
