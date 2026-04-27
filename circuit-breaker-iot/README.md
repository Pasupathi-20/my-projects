# ⚡ IoT Based Circuit Breaker Monitoring and Control System
### Smart Protection for Power Distribution Lines & Lineman Safety

[![Arduino](https://img.shields.io/badge/Platform-Arduino-00979D?logo=arduino)](https://www.arduino.cc/)
[![IoT](https://img.shields.io/badge/IoT-ThingSpeak-blue)](https://thingspeak.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Version](https://img.shields.io/badge/Version-1.0.0-blue.svg)]()
[![Status](https://img.shields.io/badge/Status-Production%20Ready-brightgreen.svg)]()

> An IoT-based circuit breaker system designed for **lineman protection** in power distribution lines. Provides automated fault detection, remote monitoring, manual trip/reset control, and real-time cloud alerts — making power line maintenance significantly safer.

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
- [Fault Types & Thresholds](#-fault-types--thresholds)
- [Breaker States](#-breaker-states)
- [Calibration](#-calibration)
- [Lineman Safety Notes](#-lineman-safety-notes)
- [Project Structure](#-project-structure)
- [License](#-license)

---

## 🔍 Overview

This system replaces conventional manually-operated circuit breakers with an intelligent IoT-enabled protection unit. It continuously monitors **current, voltage, temperature**, and **power parameters** on a distribution line. When a fault is detected, it automatically trips the breaker and notifies operators via cloud. Linemen can also **manually trip the line** for safe maintenance using a dedicated push button.

---

## 🏗️ System Architecture

```
┌──────────────────────────────────────────────────────────┐
│                  DISTRIBUTION PANEL NODE                 │
│                                                          │
│  ACS712 ──┐                                              │
│  ZMPT101B─┤                                              │
│  DHT22  ──┼──► Arduino Mega ──► Relay ──► Line ON/OFF   │
│  Buttons ─┤         │                                    │
│  LCD/LED ─┘         └──► ESP8266 ──► WiFi               │
└──────────────────────────────────────────────────────────┘
                                  │
                                  ▼
                        ┌──────────────────┐
                        │ ThingSpeak Cloud  │
                        │ Live Charts+Logs  │
                        └────────┬─────────┘
                                 │
                    ┌────────────┴────────────┐
                    │  Mobile / Web Dashboard  │
                    │  (Engineer / Operator)   │
                    └──────────────────────────┘
```

---

## ✨ Features

- ✅ **Real-time current & voltage monitoring** (ACS712 + ZMPT101B)
- ✅ **5 automatic fault types** — overcurrent, short circuit, overvoltage, undervoltage, overtemperature
- ✅ **Automatic breaker trip** — relay cuts power instantly on fault
- ✅ **Smart reset** — checks conditions before re-closing to prevent re-fault
- ✅ **Manual TRIP button** — lineman can isolate line safely before maintenance
- ✅ **Manual RESET button** — restore power after work is complete
- ✅ **EEPROM fault logging** — retains total fault count across power cycles
- ✅ **ThingSpeak cloud upload** — 8 fields of live data with graphs
- ✅ **LCD display** — 4 rotating pages of live readings
- ✅ **LED + Buzzer alerts** — visual and audible local indication
- ✅ **Offline fallback** — operates standalone if WiFi unavailable
- ✅ **Non-blocking code** — uses `millis()` throughout

---

## 🛒 Hardware Required

| Component | Qty | Purpose |
|---|---|---|
| Arduino Mega 2560 | 1 | Main microcontroller |
| ESP8266 ESP-01 | 1 | WiFi IoT upload |
| ACS712-30A Module | 1 | Current sensing (up to 30A) |
| ZMPT101B Module | 1 | AC Voltage sensing |
| DHT22 Sensor | 1 | Panel temperature & humidity |
| 5V Relay Module | 1 | Trip/reset circuit breaker coil |
| 16×2 I2C LCD | 1 | Local display |
| Active Buzzer | 1 | Audible alarm |
| Push Buttons | 2 | Manual TRIP + RESET |
| LEDs (Green/Yellow/Red) | 3 | Status indicators |
| 220Ω Resistors | 3 | LED protection |
| 10kΩ Resistors | 2 | Button pull-down (if not using INPUT_PULLUP) |
| 1kΩ + 2kΩ Resistors | 1 set | ESP8266 voltage divider |
| 5V 2A Power Supply | 1 | Arduino + sensors |

---

## 📐 Circuit Diagram

```
ACS712-30A (Current Sensor)
  VCC  → 5V  |  GND → GND  |  OUT → A0
  [Line wire passes THROUGH the current sensor loop]

ZMPT101B (Voltage Sensor)
  VCC  → 5V  |  GND → GND  |  OUT → A1
  [Connect across AC line and neutral — use isolation transformer]

DHT22 (Temperature)
  Pin1(VCC) → 5V  |  Pin2(DATA) → D7 (+ 10kΩ to 5V)  |  Pin4(GND) → GND

Relay Module (Circuit Breaker Control)
  VCC  → 5V  |  GND → GND  |  IN → D4
  COM  → Line supply
  NO   → Load (power restored when relay energized)
  NC   → Not used

Manual Buttons
  RESET Button: one pin → D2, other pin → GND  (INPUT_PULLUP used)
  TRIP  Button: one pin → D3, other pin → GND  (INPUT_PULLUP used)

ESP8266 ESP-01
  VCC    → 3.3V  [NOT 5V]
  GND    → GND
  CH_PD  → 3.3V
  TX     → D12 (direct)
  RX     → D13 via voltage divider [1kΩ + 2kΩ to GND]

LCD 16x2 I2C
  VCC → 5V  |  GND → GND  |  SDA → A4  |  SCL → A5

LEDs (220Ω series to GND)
  Green  → D9   |  Yellow → D10  |  Red → D11

Buzzer (Active 5V)
  (+) → D8  |  (-) → GND
```

---

## 📌 Pin Configuration

| Pin | Component | Function |
|---|---|---|
| `A0` | ACS712 | Current (analog) |
| `A1` | ZMPT101B | Voltage (analog) |
| `A4` | LCD SDA | I2C data |
| `A5` | LCD SCL | I2C clock |
| `D2` | Reset Button | INT0 interrupt |
| `D3` | Trip Button | INT1 interrupt |
| `D4` | Relay | Breaker control |
| `D7` | DHT22 | Temperature data |
| `D8` | Buzzer | Alert |
| `D9` | Green LED | CLOSED / Normal |
| `D10` | Yellow LED | WARNING |
| `D11` | Red LED | TRIPPED / Danger |
| `D12` | ESP8266 TX | SoftwareSerial RX |
| `D13` | ESP8266 RX | SoftwareSerial TX |

---

## 🚀 Installation

### 1. Clone the repository
```bash
git clone https://github.com/Pasupathi-20/my-projects.git
cd my-projects/circuit-breaker-iot
```

### 2. Install Libraries

Open **Arduino IDE → Manage Libraries**, install:

| Library | Author |
|---|---|
| `DHT sensor library` | Adafruit |
| `LiquidCrystal_I2C` | Frank de Brabander |
| `Wire` | Arduino (built-in) |
| `SoftwareSerial` | Arduino (built-in) |
| `EEPROM` | Arduino (built-in) |

### 3. Configure credentials
Edit `circuit_breaker.ino`:
```cpp
const String WIFI_SSID  = "YOUR_WIFI_SSID";
const String WIFI_PASS  = "YOUR_WIFI_PASSWORD";
const String TS_API_KEY = "YOUR_THINGSPEAK_WRITE_API_KEY";
```

### 4. Upload
- Board: **Arduino Mega 2560**
- Port: **COMx / /dev/ttyUSBx**
- Click **Upload**

---

## ☁️ Cloud Setup (ThingSpeak)

1. Sign up at [thingspeak.com](https://thingspeak.com)
2. Create a channel with 8 fields:

| Field | Data |
|---|---|
| Field 1 | Current (A) |
| Field 2 | Voltage (V) |
| Field 3 | Power (W) |
| Field 4 | Temperature (°C) |
| Field 5 | Humidity (%) |
| Field 6 | Breaker State (0=Closed, 1=Tripped, 2=Resetting) |
| Field 7 | Fault Type (0=None … 6=Manual) |
| Field 8 | Total Fault Count |

3. Copy your **Write API Key** into the sketch
4. View live dashboard: `https://thingspeak.com/channels/YOUR_CHANNEL_ID`

---

## 🚨 Fault Types & Thresholds

| Fault | Condition | Priority |
|---|---|---|
| **Short Circuit** | Current ≥ 40A | 1 (Highest) |
| **Overcurrent** | Current ≥ 28A | 2 |
| **Overvoltage** | Voltage ≥ 260V | 3 |
| **Undervoltage** | Voltage ≤ 180V | 4 |
| **Overtemperature** | Panel temp ≥ 75°C | 5 |
| **Manual Trip** | Lineman button press | N/A |

---

## 🔄 Breaker States

| State | Relay | Power | LED |
|---|---|---|---|
| **CLOSED** | Energized | ON | 🟢 Green |
| **TRIPPED** | De-energized | OFF | 🔴 Red + Buzzer |
| **RESETTING** | Checking... | OFF | 🟡 Yellow |

> On reset, the system re-checks live conditions before re-closing.
> If the fault is still present, reset is **blocked** automatically.

---

## 🔧 Calibration

### ACS712 Current Sensor
```cpp
#define ACS712_SENSITIVITY    0.066   // 66mV/A for 30A version
#define ACS712_ZERO_OFFSET    2.5     // Adjust if current reads non-zero at idle
```
Use a clamp meter to verify and fine-tune `ACS712_ZERO_OFFSET`.

### ZMPT101B Voltage Sensor
```cpp
#define VOLTAGE_CALIBRATION   233.0   // Adjust until LCD matches multimeter reading
```
Measure actual AC voltage with a multimeter. Adjust the constant until they match.

---

## 🦺 Lineman Safety Notes

> ⚠️ **IMPORTANT SAFETY WARNING**

- Always press the **MANUAL TRIP button** before touching any distribution line
- Verify the **RED LED is ON** and buzzer is active before approaching the line
- The relay physically disconnects the line — verify with a voltage tester
- Never rely solely on this system — use proper lockout/tagout procedures
- This system is a **monitoring and automation aid**, not a replacement for certified safety procedures

---

## 📁 Project Structure

```
circuit-breaker-iot/
│
├── src/
│   └── circuit_breaker.ino     # Main Arduino sketch
│
├── docs/
│   ├── circuit_diagram.md      # Wiring guide
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
GitHub: [@Pasupathi-20](https://github.com/Pasupathi-20)

---

*Built for safer power distribution maintenance ⚡🦺*
