# 🚧 Boom Barrier Position Detection Using MPU6050 and NodeMCU
### Real-time Arm Position Monitoring for Automated Gates & Parking Barriers

[![NodeMCU](https://img.shields.io/badge/Platform-NodeMCU%20ESP8266-blue)](https://www.nodemcu.com/)
[![IoT](https://img.shields.io/badge/IoT-ThingSpeak-blue)](https://thingspeak.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Version](https://img.shields.io/badge/Version-1.0.0-blue.svg)]()
[![Status](https://img.shields.io/badge/Status-Production%20Ready-brightgreen.svg)]()

> Detects boom barrier arm positions (CLOSED, OPENING, OPEN, CLOSING) using an **MPU6050** accelerometer/gyroscope mounted on the barrier arm. A **complementary filter** algorithm fuses accelerometer and gyroscope data for smooth, drift-free pitch angle measurement. Real-time data is published to **ThingSpeak** and served via a built-in **web dashboard**.

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
- [Web Dashboard](#-web-dashboard)
- [Position Detection Algorithm](#-position-detection-algorithm)
- [Calibration](#-calibration)
- [Project Structure](#-project-structure)
- [License](#-license)

---

## 🔍 Overview

This system mounts an **MPU6050 IMU sensor** on the boom barrier arm. As the arm moves, the sensor continuously reads **pitch angle** (0° = horizontal/closed, 90° = vertical/open). A threshold-based algorithm with hysteresis determines the current state in real time:

| Pitch Angle | State |
|-------------|-------|
| ≤ 10° | CLOSED |
| 10° → 80°, rising | OPENING |
| ≥ 80° | OPEN |
| 80° → 10°, falling | CLOSING |

---

## 🏗️ System Architecture

```
┌────────────────────────────────────────────────────────┐
│                  BOOM BARRIER NODE                     │
│                                                        │
│  MPU6050 ──[I2C]──► NodeMCU ESP8266                   │
│                         │                              │
│                    ┌────┴─────┐                        │
│                    │          │                        │
│               LCD+LEDs    Web Server                   │
│               (local)     port 80                      │
└────────────────────────────────────────────────────────┘
                         │
                    WiFi (2.4GHz)
                         │
              ┌──────────┴──────────┐
              │                     │
        ThingSpeak              Browser
        (cloud log)         (live dashboard
                             at device IP)
```

---

## ✨ Features

- ✅ **Real-time pitch angle** using MPU6050 (accelerometer + gyroscope)
- ✅ **Complementary filter** — smooth, drift-free angle at 50Hz
- ✅ **4 position states** — CLOSED / OPENING / OPEN / CLOSING
- ✅ **Hysteresis** — prevents state flicker at threshold boundaries
- ✅ **Gyro calibration** on boot — removes drift offset
- ✅ **ThingSpeak upload** — pitch + state + open/close counts
- ✅ **Built-in web dashboard** — auto-refreshing live view at device IP
- ✅ **JSON API endpoint** — `/data` for integration with other systems
- ✅ **LCD display** — local pitch and status readout
- ✅ **LED indicators** — Blue=Closed, Yellow=Moving, Green=Open
- ✅ **Buzzer** — beep on every state change
- ✅ **Open/Close cycle counter** — tracks total barrier usage

---

## 🛒 Hardware Required

| Component | Qty | Notes |
|---|---|---|
| NodeMCU ESP8266 (ESP-12E) | 1 | Main controller + WiFi |
| MPU6050 GY-521 Module | 1 | Accelerometer + Gyroscope |
| 16×2 I2C LCD Display | 1 | Local readout |
| LEDs (Blue/Yellow/Green) | 3 | Position indicators |
| Active Buzzer (5V) | 1 | State change alert |
| 220Ω Resistors | 3 | LED protection |
| Breadboard + Wires | — | Connections |
| 3.3V / 5V Power Supply | 1 | NodeMCU via USB or adapter |
| Mounting bracket/tape | 1 | To fix MPU6050 on barrier arm |

---

## 📐 Circuit Diagram

```
MPU6050 (GY-521)          NodeMCU ESP8266
┌──────────────┐          ┌──────────────────┐
│ VCC ─────────┼──────────┤ 3.3V             │
│ GND ─────────┼──────────┤ GND              │
│ SDA ─────────┼──────────┤ D2 (GPIO4)       │
│ SCL ─────────┼──────────┤ D1 (GPIO5)       │
│ AD0 ─────────┼── GND    │  (addr = 0x68)   │
│ INT ─────────┼── N/C    │                  │
└──────────────┘          │                  │
                          │                  │
LCD 16x2 I2C              │                  │
┌──────────────┐          │                  │
│ VCC ─────────┼──────────┤ 3.3V or 5V       │
│ GND ─────────┼──────────┤ GND              │
│ SDA ─────────┼──────────┤ D2 (shared)      │
│ SCL ─────────┼──────────┤ D1 (shared)      │
└──────────────┘          │                  │
                          │                  │
LED + 220Ω resistors:     │                  │
  Blue  (CLOSED)  ────────┤ D5 (GPIO14)      │
  Yellow(MOVING)  ────────┤ D6 (GPIO12)      │
  Green (OPEN)    ────────┤ D7 (GPIO13)      │
                          │                  │
Active Buzzer (+) ────────┤ D8 (GPIO15)      │
Active Buzzer (-) ────────┤ GND              │
                          └──────────────────┘
```

> ⚠️ The MPU6050 VCC must be **3.3V** on NodeMCU — the GY-521 module has an onboard 3.3V regulator if you feed 5V, but check your module.

---

## 📌 Pin Configuration

| NodeMCU Pin | GPIO | Component | Function |
|---|---|---|---|
| `D1` | GPIO5 | MPU6050 + LCD | I2C SCL |
| `D2` | GPIO4 | MPU6050 + LCD | I2C SDA |
| `D5` | GPIO14 | Blue LED | CLOSED indicator |
| `D6` | GPIO12 | Yellow LED | MOVING indicator |
| `D7` | GPIO13 | Green LED | OPEN indicator |
| `D8` | GPIO15 | Buzzer | State change alert |

---

## 🚀 Installation

### 1. Clone the repository
```bash
git clone https://github.com/Pasupathi-20/my-projects.git
cd my-projects/boom-barrier-mpu6050
```

### 2. Install Arduino Board Support

In Arduino IDE → **File → Preferences → Additional Board Manager URLs**, add:
```
http://arduino.esp8266.com/stable/package_esp8266com_index.json
```
Then: **Tools → Board → Boards Manager** → Install **esp8266 by ESP8266 Community**

### 3. Install Libraries

**Sketch → Include Library → Manage Libraries**, install:

| Library | Author |
|---|---|
| `LiquidCrystal_I2C` | Frank de Brabander |
| `Wire` | Arduino (built-in) |
| `ESP8266WiFi` | ESP8266 Community (built-in) |
| `ESP8266WebServer` | ESP8266 Community (built-in) |

> No external MPU6050 library needed — this sketch communicates directly via I2C registers.

### 4. Configure credentials
Edit `boom_barrier.ino`:
```cpp
const char* WIFI_SSID  = "YOUR_WIFI_SSID";
const char* WIFI_PASS  = "YOUR_WIFI_PASSWORD";
const char* TS_API_KEY = "YOUR_THINGSPEAK_WRITE_API_KEY";
```

### 5. Upload
- Board: **NodeMCU 1.0 (ESP-12E Module)**
- Upload Speed: **115200**
- Port: **COMx / /dev/ttyUSBx**
- Click **Upload**

---

## ☁️ Cloud Setup (ThingSpeak)

1. Sign up at [thingspeak.com](https://thingspeak.com)
2. Create a channel with 4 fields:

| Field | Data |
|---|---|
| Field 1 | Pitch Angle (degrees) |
| Field 2 | Barrier State (0=Closed, 1=Opening, 2=Open, 3=Closing) |
| Field 3 | Total Open Count |
| Field 4 | Total Close Count |

3. Copy the **Write API Key** into the sketch
4. View live graphs at `https://thingspeak.com/channels/YOUR_CHANNEL_ID`

---

## 🌐 Web Dashboard

After connecting to WiFi, open Serial Monitor to see the device IP address.  
Navigate to `http://<device-ip>/` in any browser on the same WiFi network.

**Features:**
- Live pitch angle display
- Color-coded barrier status card
- Open/close cycle counters
- Auto-refreshes every 2 seconds
- JSON API at `http://<device-ip>/data`

**JSON Response example:**
```json
{
  "pitch": 45.32,
  "position": "OPENING",
  "position_code": 1,
  "open_count": 12,
  "close_count": 11,
  "total_cycles": 3200,
  "ip": "192.168.1.105"
}
```

---

## 📐 Position Detection Algorithm

### Complementary Filter

Raw accelerometer data is noisy. Raw gyroscope data drifts over time.
The **complementary filter** combines both for best accuracy:

```
pitch = 0.98 × (pitch + gyroX × dt) + 0.02 × accelAngle
```

- **98%** gyroscope (fast, smooth, short-term accurate)
- **2%** accelerometer (slow, noisy, long-term accurate)

This gives accurate angles at 50Hz with minimal drift.

### Position State Machine

```
       pitch <= 10°          pitch >= 80°
         ┌────┐              ┌────┐
         │    │              │    │
    ► CLOSED  ──rising──► OPENING ──► OPEN
         │                          │
         │    ◄──falling──CLOSING ◄──┤
         └────────────────────────┘
```

Hysteresis of ±2° is applied at boundaries to avoid state flicker.

---

## 🔧 Calibration

### Gyro Offset Calibration (automatic on boot)
Keep the barrier **completely still** during the first 1–2 seconds after power-on.
The system takes 200 samples to compute gyro bias offsets automatically.

### Position Thresholds (manual)
```cpp
#define PITCH_CLOSED_MAX    10.0   // deg — arm fully closed
#define PITCH_OPEN_MIN      80.0   // deg — arm fully open
```

Adjust these to match your physical barrier installation angle.
Use Serial Monitor to observe raw pitch values during manual movement.

### Sensor Mounting
- Mount MPU6050 **flat on the barrier arm**, parallel to the arm length
- Secure firmly — vibrations will add noise to readings
- Ensure the X-axis of MPU6050 is aligned with the rotation axis

---

## 📁 Project Structure

```
boom-barrier-mpu6050/
│
├── src/
│   └── boom_barrier.ino        # Main NodeMCU sketch
│
├── docs/
│   ├── circuit_diagram.md      # Wiring guide
│   └── components.md           # Component specs
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

*Built for smarter parking and gate automation 🚧*
