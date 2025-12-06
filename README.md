# MicromouseF411CEU6

🐭 **Micromouse Robot using STM32F411CEU6 Black Pill**

## 📋 Overview

A complete micromouse robot firmware featuring:
- **PID Motor Control** - Precise position and velocity control
- **6x IR Sensors** - Wall detection and distance measurement
- **MPU6050 IMU** - Gyroscope for accurate turning
- **Quadrature Encoders** - Motor feedback for odometry
- **Bluetooth (JDY-33)** - Wireless debugging and control

## 🛠️ Hardware

| Component | Description | Pins |
|-----------|-------------|------|
| MCU | STM32F411CEU6 (Black Pill) | - |
| Motors | 2x N20 DC Motors with TB6612FNG | TIM2 CH1/CH2 |
| Encoders | 2x Quadrature Encoders (2800 PPR) | TIM3, TIM4 |
| IR Sensors | 6x IR LED + Phototransistor pairs | ADC1 |
| IMU | MPU6050 6-axis | I2C1 (PB8/PB9) |
| Debug UART | USB-TTL 115200 baud | USART1 (PA9/PA10) |
| Bluetooth | JDY-33 BLE Module | USART2 (PA2/PA3) |

## 📁 Project Structure

```
MicromouseF411CEU6/
├── Core/
│   ├── Inc/           # Header files
│   │   ├── pinout.h
│   │   ├── hardware.h
│   │   └── system_timer.h
│   └── Src/           # Source files
│       ├── main.c
│       ├── hardware.c
│       └── system_timer.c
├── Drivers/
│   ├── ADC/           # IR sensor ADC driver
│   ├── Encoder/       # Quadrature encoder driver
│   ├── I2C/           # I2C communication
│   ├── Motor/         # TB6612FNG motor driver
│   ├── MPU6050/       # IMU driver
│   ├── Timer/         # Timer utilities
│   ├── UART/          # Debug serial
│   └── Bluetooth/     # JDY-33 BLE module
└── MDK/               # Keil uVision project
```

## ⚡ Pin Configuration

### Motors (TB6612FNG)
- **Motor Right:** PWM=PA15 (TIM2_CH1), IN1=PB12, IN2=PB13
- **Motor Left:** PWM=PB3 (TIM2_CH2), IN1=PB14, IN2=PB15

### Encoders
- **Encoder Right:** TIM3 (PB4/PB5)
- **Encoder Left:** TIM4 (PB6/PB7)

### IR Sensors
| Sensor | RX (ADC) | TX (GPIO) |
|--------|----------|-----------|
| L0 (Front Left) | PA1 | PA8 |
| L45 | PA4 | PA11 |
| L90 (Side Left) | PA5 | PA12 |
| R0 (Front Right) | PA6 | PB1 |
| R45 | PA7 | PB2 |
| R90 (Side Right) | PB0 | PB10 |

## 🚀 Getting Started

### Prerequisites
- Keil uVision 5
- ST-Link V2 programmer
- STM32F411CEU6 Black Pill board

### Build & Flash
1. Open `MDK/MicromouseF411.uvprojx` in Keil
2. Build project (F7)
3. Flash to MCU (F8)

### Serial Monitor
Connect USB-TTL to PA9(TX)/PA10(RX) at 115200 baud to view debug output.

## 📊 PID Tuning

Default PID parameters for position control:
```c
Kp = 0.2f
Ki = 0.005f
Kd = 0.5f
```

Output format on Serial: `Setpoint,Encoder,PWM`

## ⚠️ Important Notes

1. **JTAG Pins:** PA15, PB3, PB4 are JTAG pins - must disable JTAG in code (SWD only)
2. **PB2 (BOOT1):** Requires 10k pull-down resistor
3. **PC14/PC15:** Leave floating for crystal stability

## 📝 License

MIT License - Feel free to use and modify!

## 👤 Author

**HoangAnh** - [@HoangAnh-ptit](https://github.com/HoangAnh-ptit)

---
⭐ Star this repo if you find it useful!