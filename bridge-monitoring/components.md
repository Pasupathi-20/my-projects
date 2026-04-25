# Component Details & Datasheets
## IoT Bridge Health Monitoring System

---

## 1. Arduino Mega 2560 (Recommended)

| Spec | Value |
|------|-------|
| Microcontroller | ATmega2560 |
| Operating Voltage | 5V |
| Digital I/O Pins | 54 (15 PWM) |
| Analog Input Pins | 16 |
| Flash Memory | 256 KB |
| SRAM | 8 KB |
| Clock Speed | 16 MHz |

> Mega preferred over Uno for this project — more pins, more SRAM for sensor data buffering.

---

## 2. MPU6050 (GY-521 Module)

| Spec | Value |
|------|-------|
| Interface | I2C |
| Operating Voltage | 3.3V – 5V (module has regulator) |
| Accelerometer Range | ±2g / ±4g / ±8g / ±16g |
| Gyroscope Range | ±250 / ±500 / ±1000 / ±2000 °/s |
| ADC Resolution | 16-bit |
| I2C Address | 0x68 (AD0=LOW), 0x69 (AD0=HIGH) |

### What it measures for this project:
- **Vibration** — magnitude of acceleration vector (m/s²)
- **Tilt** — angle from vertical using atan2 calculation

---

## 3. HX711 24-bit ADC + Load Cell

| Spec | Value |
|------|-------|
| ADC Resolution | 24-bit |
| Input Channels | 2 (A and B) |
| Data Rate | 10 SPS or 80 SPS |
| Operating Voltage | 2.6V – 5.5V |
| Gain | 128 (Ch A), 64 (Ch A), 32 (Ch B) |

### Load Cell Types Compatible:
- Half-bridge (2-wire)
- Full-bridge/Wheatstone (4-wire) — **recommended for bridge monitoring**

### Calibration Steps:
1. Power on with no load, run `scale.tare()`
2. Place known weight (e.g. 1 kg)
3. Read raw value: `scale.get_units(10)`
4. `calibration_factor = raw_value / 1.0` (for 1 kg reference)
5. Set: `scale.set_scale(calibration_factor)`

---

## 4. DHT22 (AM2302)

| Spec | Value |
|------|-------|
| Operating Voltage | 3.3V – 6V |
| Temperature Range | -40°C to +80°C |
| Temperature Accuracy | ±0.5°C |
| Humidity Range | 0% – 100% RH |
| Humidity Accuracy | ±2% RH |
| Sampling Rate | 0.5 Hz (one reading per 2 seconds) |

---

## 5. MQ-2 Gas Sensor

| Spec | Value |
|------|-------|
| Operating Voltage | 5V |
| Detects | LPG, Propane, Hydrogen, Smoke, Methane |
| Warm-up Time | 2 minutes |
| Output | Analog (0–5V) + Digital (adjustable) |
| Heater Resistance | 33Ω ± 5% |

> Higher analog value = more gas detected.

---

## 6. Water Level Sensor

| Spec | Value |
|------|-------|
| Operating Voltage | 3.3V – 5V |
| Output | Analog (0–1023 via ADC) + Digital |
| Detection Area | Exposed traces (varies by module) |
| Weatherproofing | None — requires external sealing |

---

## 7. ESP8266 ESP-01

| Spec | Value |
|------|-------|
| SoC | Espressif ESP8266EX |
| Operating Voltage | 3.3V ⚠️ |
| WiFi Standard | 802.11 b/g/n |
| Frequency | 2.4 GHz |
| TCP Connections | Up to 5 simultaneous |
| Flash | 1 MB |
| AT Firmware | Required (pre-flashed) |

---

## Useful Links

- [MPU6050 Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [HX711 Datasheet](https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf)
- [DHT22 Datasheet](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf)
- [MQ-2 Datasheet](https://www.pololu.com/file/0J309/MQ2.pdf)
- [ESP8266 AT Commands](https://www.espressif.com/sites/default/files/documentation/4a-esp8266_at_instruction_set_en.pdf)
- [ThingSpeak API Docs](https://uk.mathworks.com/help/thingspeak/)
- [Arduino MPU6050 Library](https://github.com/ElectronicCats/mpu6050)
- [HX711 Arduino Library](https://github.com/bogde/HX711)
