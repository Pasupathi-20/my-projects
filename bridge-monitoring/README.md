# 🌉 IoT Based Bridge Health Monitoring and Alerting System
### Real-time Structural Safety Monitoring via Arduino + ESP8266 + ThingSpeak

[![Arduino](https://img.shields.io/badge/Platform-Arduino-00979D?logo=arduino)](https://www.arduino.cc/)
[![IoT](https://img.shields.io/badge/IoT-ThingSpeak-blue)](https://thingspeak.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Version](https://img.shields.io/badge/Version-1.0.0-blue.svg)]()
[![Status](https://img.shields.io/badge/Status-Production%20Ready-brightgreen.svg)]()

> A multi-sensor IoT system that continuously monitors bridge structural health — detecting vibration, overload, tilt, temperature, flooding, and gas leaks — and sends real-time alerts to cloud platforms and mobile devices.

---

## 📋 Table of Contents

- [Overview](#-overview)
- [System Architecture](#-system-architecture)
- [Features](#-features)
- [Hardware Required](#-hardware-required)
- [Circuit Diagram](#-circuit-diagram)
- [Pin Configuration](#-pin-configuration)
- [Installation](#-installation)
- [Cloud Setup (ThingSpeak)](#-cloud-setup-thingspeak)
- [Alert Levels](#-alert-levels)
- [Calibration](#-calibration)
- [Project Structure](#-project-structure)
- [License](#-license)

---

## 🔍 Overview

Bridges are critical infrastructure subject to constant stress from traffic loads, environmental conditions, and material degradation. This IoT-based system provides **continuous automated structural health monitoring** using a network of sensors connected to an Arduino microcontroller, with data uploaded to the **ThingSpeak cloud** for visualization and remote alerting.

---

## 🏗️ System Architecture

```
┌────────────────────────────────────────────────────────┐
│                    BRIDGE NODE                         │
│                                                        │
│  MPU6050 ──┐                                           │
│  HX711  ──┤                                           │
│  DHT22  ──┼──► Arduino Uno/Mega ──► ESP8266 ──► WiFi  │
│  MQ-2   ──┤                    │                       │
│  Water  ──┘                    └──► LCD + LED + Buzzer │
└────────────────────────────────────────────────────────┘
                                         │
                                         ▼
                               ┌─────────────────┐
                               │  ThingSpeak Cloud│
                               │  (Data + Graphs) │
                               └────────┬────────┘
                                        │
                              ┌─────────┴─────────┐
                              │   Mobile / Web     │
                              │   Dashboard        │
                              └───────────────────┘
```

---

## ✨ Features

- ✅ **Vibration monitoring** — detects structural resonance & shock (MPU6050)
- ✅ **Tilt / inclination detection** — monitors bridge sag or lean (MPU6050)
- ✅ **Load / weight monitoring** — overload detection via strain gauge (HX711)
- ✅ **Temperature & humidity** — thermal stress monitoring (DHT22)
- ✅ **Flood detection** — water level sensor under bridge deck
- ✅ **Gas / smoke detection** — fire and hazardous gas alert (MQ-2)
- ✅ **3-level alerting** — NORMAL / WARNING / DANGER
- ✅ **IoT cloud upload** — ThingSpeak with real-time graphs
- ✅ **Local alerts** — LCD, RGB LEDs, buzzer
- ✅ **Non-blocking code** — uses `millis()` throughout
- ✅ **Offline fallback** — operates standalone if WiFi fails

---

## 🛒 Hardware Required

| Component | Quantity | Purpose |
|---|---|---|
| Arduino Uno / Mega | 1 | Main microcontroller |
| ESP8266 (ESP-01) | 1 | WiFi IoT connectivity |
| MPU6050 (GY-521) | 1 | Vibration & tilt sensing |
| HX711 + Load Cell | 1 set | Structural load / weight |
| DHT22 | 1 | Temperature & humidity |
| MQ-2 Gas Sensor | 1 | Gas / smoke / fire detection |
| Water Level Sensor | 1 | Flood detection |
| 16×2 I2C LCD | 1 | Local display |
| Active Buzzer | 1 | Audible alert |
| LEDs (Green/Yellow/Red) | 3 | Visual alert levels |
| 220Ω Resistors | 3 | LED current limiting |
| 1kΩ + 2kΩ Resistors | 2 | ESP8266 voltage divider |
| Breadboard + Jumper Wires | — | Connections |
| 5V 2A Power Supply | 1 | Stable power for all sensors |

---

## 📐 Circuit Diagram

```
MPU6050 (GY-521)
┌─────────────────┐
│  VCC ───────────┼── 3.3V
│  GND ───────────┼── GND
│  SDA ───────────┼── A4 (Arduino)
│  SCL ───────────┼── A5 (Arduino)
└─────────────────┘

HX711 Load Cell Amplifier
┌─────────────────┐
│  VCC ───────────┼── 5V
│  GND ───────────┼── GND
│  DT  ───────────┼── D4 (Arduino)
│  SCK ───────────┼── D5 (Arduino)
└─────────────────┘

DHT22 Sensor
┌─────────────────┐
│  VCC ───────────┼── 5V
│  GND ───────────┼── GND
│  DATA ──────────┼── D7 (Arduino)
└─────────────────┘

MQ-2 Gas Sensor
┌─────────────────┐
│  VCC ───────────┼── 5V
│  GND ───────────┼── GND
│  AO  ───────────┼── A1 (Arduino)
│  DO  ───────────┼── D3 (Arduino)
└─────────────────┘

Water Level Sensor
┌─────────────────┐
│  VCC ───────────┼── 5V
│  GND ───────────┼── GND
│  AO  ───────────┼── A2 (Arduino)
│  DO  ───────────┼── D2 (Arduino)
└─────────────────┘

ESP8266 (ESP-01) — Use 3.3V only!
┌─────────────────┐
│  VCC ───────────┼── 3.3V
│  GND ───────────┼── GND
│  TX  ───────────┼── D12 (Arduino RX) via voltage divider
│  RX  ───────────┼── D13 (Arduino TX)
│  CH_PD ─────────┼── 3.3V (enable)
└─────────────────┘

⚠️ Voltage Divider for ESP8266 RX (5V → 3.3V):
Arduino TX (D13) ── [1kΩ] ── ESP RX
                               │
                            [2kΩ]
                               │
                              GND

LCD 16×2 I2C
  SDA → A4  |  SCL → A5  |  VCC → 5V  |  GND → GND

LEDs (+ 220Ω resistors to GND)
  Green  → D9   |  Yellow → D10  |  Red → D11

Buzzer
  (+) → D8  |  (-) → GND
```

---

## 📌 Pin Configuration

| Pin | Component | Signal |
|---|---|---|
| `A0` | MPU6050 | I2C (shared with A4/A5) |
| `A1` | MQ-2 | Analog gas level |
| `A2` | Water Sensor | Analog water level |
| `A4` | LCD + MPU6050 | I2C SDA |
| `A5` | LCD + MPU6050 | I2C SCL |
| `D2` | Water Sensor | Digital output |
| `D3` | MQ-2 | Digital output |
| `D4` | HX711 | DOUT |
| `D5` | HX711 | SCK |
| `D7` | DHT22 | Data |
| `D8` | Buzzer | PWM alert |
| `D9` | Green LED | Normal indicator |
| `D10` | Yellow LED | Warning indicator |
| `D11` | Red LED | Danger indicator |
| `D12` | ESP8266 TX | SoftwareSerial RX |
| `D13` | ESP8266 RX | SoftwareSerial TX |

---

## 🚀 Installation

### 1. Clone the Repository
```bash
git clone https://github.com/Pasupathi-20/my-projects.git
cd my-projects/bridge-health-monitor
```

### 2. Install Required Libraries

Open **Arduino IDE → Sketch → Include Library → Manage Libraries**, install:

| Library | Author |
|---|---|
| `MPU6050` | Electronic Cats |
| `HX711` | Bogdan Necula |
| `DHT sensor library` | Adafruit |
| `LiquidCrystal_I2C` | Frank de Brabander |
| `Wire` | Arduino (built-in) |
| `SoftwareSerial` | Arduino (built-in) |

### 3. Configure Your Credentials

Edit these lines in `bridge_monitor.ino`:
```cpp
const String WIFI_SSID  = "YOUR_WIFI_SSID";
const String WIFI_PASS  = "YOUR_WIFI_PASSWORD";
const String TS_API_KEY = "YOUR_THINGSPEAK_WRITE_API_KEY";
```

### 4. Upload
1. Select board: **Tools → Board → Arduino Mega 2560** (recommended)
2. Select port: **Tools → Port → COMx**
3. Click **Upload ⬆️**

---

## ☁️ Cloud Setup (ThingSpeak)

1. Create a free account at [thingspeak.com](https://thingspeak.com)
2. Create a new **Channel** with 8 fields:

| Field | Data |
|---|---|
| Field 1 | Vibration (m/s²) |
| Field 2 | Tilt Angle (°) |
| Field 3 | Load (kg) |
| Field 4 | Temperature (°C) |
| Field 5 | Humidity (%) |
| Field 6 | Water Level (0–1023) |
| Field 7 | Gas Level (0–1023) |
| Field 8 | Alert Level (0/1/2) |

3. Copy your **Write API Key** into the sketch
4. View live graphs at `https://thingspeak.com/channels/YOUR_CHANNEL_ID`

---

## 🚨 Alert Levels

| Level | Condition | LED | Buzzer | Action |
|---|---|---|---|---|
| **NORMAL** | All sensors in safe range | 🟢 Green | Silent | Monitor |
| **WARNING** | Approaching threshold | 🟡 Yellow | Slow beep | Log + Alert |
| **DANGER** | Critical threshold breached | 🔴 Red | Rapid beep | Immediate alert |

### Threshold Reference

| Parameter | Warning | Danger |
|---|---|---|
| Vibration | 8.0 m/s² | 15.0 m/s² |
| Tilt Angle | 3.0° | 6.0° |
| Load | 8,000 kg | 12,000 kg |
| Temperature | 55°C | 75°C |
| Water Level | 400 (0–1023) | 700 (0–1023) |
| Gas Level | 300 (0–1023) | 600 (0–1023) |

---

## 🔧 Calibration

### Load Cell (HX711)
```cpp
scale.set_scale(2280.0);  // Replace with your calibration factor
```
To calibrate: place a known weight, read the raw value, then:
`calibration_factor = raw_reading / known_weight_kg`

### MPU6050 Vibration Thresholds
Use Serial Monitor to observe real-world values under normal traffic, then set thresholds ~20% above the baseline.

---

## 📁 Project Structure

```
bridge-health-monitor/
│
├── src/
│   └── bridge_monitor.ino      # Main Arduino sketch
│
├── docs/
│   ├── circuit_diagram.md      # Detailed wiring guide
│   └── components.md           # Component specs & datasheets
│
├── schematics/
│   └── wiring_notes.txt        # Quick wiring reference
│
├── LICENSE
└── README.md
```

---

## 📄 License

MIT License — see [LICENSE](LICENSE) for details.

---

## 👤 Author

**Pasupathi**
- GitHub: [@Pasupathi-20](https://github.com/Pasupathi-20)

---

*Built for safer infrastructure monitoring 🌉*
