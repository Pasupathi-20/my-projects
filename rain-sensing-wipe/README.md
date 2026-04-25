# 🌧️ Automatic Rain Sensing Car Wiper System
### Arduino-based Smart Wiper Controller

[![Arduino](https://img.shields.io/badge/Platform-Arduino-00979D?logo=arduino)](https://www.arduino.cc/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Version](https://img.shields.io/badge/Version-1.0.0-blue.svg)]()
[![Status](https://img.shields.io/badge/Status-Production%20Ready-brightgreen.svg)]()

> An intelligent windshield wiper control system that automatically detects rainfall intensity and adjusts wiper speed accordingly — enhancing driving safety without manual intervention.

---

## 📋 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [Hardware Required](#-hardware-required)
- [Circuit Diagram](#-circuit-diagram)
- [Pin Configuration](#-pin-configuration)
- [Installation](#-installation)
- [Usage](#-usage)
- [Wiper Modes](#-wiper-modes)
- [Calibration](#-calibration)
- [Project Structure](#-project-structure)
- [Contributing](#-contributing)
- [License](#-license)

---

## 🔍 Overview

This project uses an **Arduino microcontroller** paired with a **rain sensor module** to automatically detect rainfall and activate/control windshield wipers. The system reads analog values from the rain sensor and maps them to four distinct wiper modes — OFF, SLOW, MEDIUM, and FAST — providing real-time response to changing weather conditions.

---

## ✨ Features

- ✅ **Automatic rain detection** using analog + digital rain sensor
- ✅ **4 wiper speed modes** — OFF / SLOW / MEDIUM / FAST
- ✅ **Smooth servo-based wiper sweep** with configurable angles
- ✅ **16×2 LCD display** showing real-time sensor value and wiper mode
- ✅ **LED indicators** for each wiper mode
- ✅ **Buzzer alert** on mode change
- ✅ **Non-blocking code** using `millis()` — no `delay()` in main loop
- ✅ **Fully calibratable** thresholds via `#define` constants
- ✅ **Serial Monitor support** for debugging

---

## 🛒 Hardware Required

| Component | Quantity | Notes |
|---|---|---|
| Arduino Uno / Nano | 1 | Any ATmega328P board works |
| Rain Sensor Module | 1 | FC-37 or YL-83 recommended |
| Servo Motor | 1 | SG90 or MG995 for wiper arm |
| 16×2 I2C LCD Display | 1 | Address: 0x27 or 0x3F |
| Buzzer (Active) | 1 | 5V buzzer |
| LEDs (4 colors) | 4 | Green, Yellow, Orange, Red |
| 220Ω Resistors | 4 | For LEDs |
| Breadboard | 1 | |
| Jumper Wires | As needed | |
| USB Cable / 9V Battery | 1 | Power supply |

---

## 📐 Circuit Diagram

```
Rain Sensor Module
┌─────────────────┐
│  VCC  ──────────┼──── 5V (Arduino)
│  GND  ──────────┼──── GND (Arduino)
│  AO   ──────────┼──── A0 (Analog In)
│  DO   ──────────┼──── D2 (Digital In)
└─────────────────┘

Servo Motor (Wiper)
┌─────────────────┐
│  Red   ─────────┼──── 5V (Arduino)
│  Brown ─────────┼──── GND (Arduino)
│  Orange ────────┼──── D9 (PWM)
└─────────────────┘

I2C LCD Display
┌─────────────────┐
│  VCC  ──────────┼──── 5V
│  GND  ──────────┼──── GND
│  SDA  ──────────┼──── A4
│  SCL  ──────────┼──── A5
└─────────────────┘

LEDs (each with 220Ω resistor to GND)
  Green  (OFF)    ──── D3
  Yellow (SLOW)   ──── D4
  Orange (MEDIUM) ──── D5
  Red    (FAST)   ──── D6

Buzzer
  (+) ──── D8
  (-) ──── GND
```

---

## 📌 Pin Configuration

| Arduino Pin | Component | Description |
|---|---|---|
| `A0` | Rain Sensor AO | Analog rain intensity |
| `D2` | Rain Sensor DO | Digital rain presence |
| `D3` | Green LED | Mode: OFF |
| `D4` | Yellow LED | Mode: SLOW |
| `D5` | Orange LED | Mode: MEDIUM |
| `D6` | Red LED | Mode: FAST |
| `D8` | Buzzer | Alert on mode change |
| `D9` | Servo Motor | Wiper arm control |
| `A4` | LCD SDA | I2C data |
| `A5` | LCD SCL | I2C clock |

---

## 🚀 Installation

### 1. Clone the Repository
```bash
git clone https://github.com//rain-wiper-arduino.git
cd rain-wiper-arduino
```

### 2. Install Arduino Libraries

Open **Arduino IDE** → **Sketch** → **Include Library** → **Manage Libraries**, then install:

| Library | Author |
|---|---|
| `Servo` | Arduino (built-in) |
| `Wire` | Arduino (built-in) |
| `LiquidCrystal_I2C` | Frank de Brabander |

Or install via Arduino CLI:
```bash
arduino-cli lib install "LiquidCrystal I2C"
```

### 3. Upload the Sketch

1. Open `src/rain_wiper.ino` in Arduino IDE
2. Select your board: **Tools → Board → Arduino Uno**
3. Select the correct port: **Tools → Port → COMx (Windows) / /dev/ttyUSBx (Linux)**
4. Click **Upload** ⬆️

---

## 🎮 Usage

1. Power on the Arduino
2. The system performs a **self-test** (all LEDs blink, two beeps)
3. LCD shows `Rain Wiper — Initializing...`
4. Place the rain sensor in the rain or simulate with water drops
5. The wiper servo activates automatically based on rain intensity
6. Open **Serial Monitor** at `9600 baud` for debug output

---

## 🌧️ Wiper Modes

| Mode | Sensor Value (0–1023) | Wiper Speed | LED |
|---|---|---|---|
| **OFF** | > 900 | Parked | 🟢 Green |
| **SLOW** | 700 – 900 | 20ms/step | 🟡 Yellow |
| **MEDIUM** | 400 – 700 | 10ms/step | 🟠 Orange |
| **FAST** | < 400 | 3ms/step | 🔴 Red |

> 💡 Lower analog value = more water detected = faster wipers

---

## 🔧 Calibration

Edit these defines in `rain_wiper.ino` to match your sensor:

```cpp

#define THRESHOLD_NO_RAIN    900   // Dry condition
#define THRESHOLD_LIGHT_RAIN 700   // Light drizzle
#define THRESHOLD_MOD_RAIN   400   // Moderate rain

#define WIPER_LEFT_POS    30     
#define WIPER_RIGHT_POS   150    

#define SPEED_SLOW        20
#define SPEED_MEDIUM      10
#define SPEED_FAST        3
```

Use the **Serial Monitor** to read raw sensor values and set your thresholds accurately.

---

## 📁 Project Structure

```
rain-wiper-arduino/
│
├── src/
│   └── rain_wiper.ino          # Main Arduino sketch
│
├── docs/
│   ├── circuit_diagram.md      # Wiring instructions
│   └── components.md           # Component details & datasheets
│
├── schematics/
│   └── wiring_notes.txt        # Schematic reference
│
├── .gitignore                  # Arduino build artifacts
├── LICENSE                     # MIT License
└── README.md                   # This file
```

---

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create your feature branch: `git checkout -b feature/AmazingFeature`
3. Commit your changes: `git commit -m 'Add AmazingFeature'`
4. Push to the branch: `git push origin feature/AmazingFeature`
5. Open a Pull Request

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## 👤 Author

**Your Name**
- GitHub: [@YOUR_USERNAME](https://github.com/YOUR_USERNAME)

---

## 🙏 Acknowledgments

- Arduino Community for open-source libraries
- Rain sensor module manufacturers for datasheets
- All contributors who improve this project

---

*Built with ❤️ for safer driving*
