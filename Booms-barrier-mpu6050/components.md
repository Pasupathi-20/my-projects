# Component Details & Datasheets
## Boom Barrier Position Detection System

---

## 1. NodeMCU ESP8266 (ESP-12E)

| Spec | Value |
|------|-------|
| SoC | Espressif ESP8266EX |
| Operating Voltage | 3.3V (VIN accepts 5V) |
| Digital I/O | 11 pins |
| Analog Input | 1 (A0, 0–1V range) |
| Clock Speed | 80 / 160 MHz |
| Flash | 4 MB |
| WiFi | 802.11 b/g/n 2.4GHz |
| I2C | D1 (SCL), D2 (SDA) — software I2C |
| USB-to-Serial | CH340G onboard |

### GPIO Mapping (NodeMCU ↔ ESP8266)
| NodeMCU | GPIO | Notes |
|---------|------|-------|
| D0 | GPIO16 | No PWM, no I2C |
| D1 | GPIO5 | I2C SCL |
| D2 | GPIO4 | I2C SDA |
| D3 | GPIO0 | Boot mode, pull-up |
| D4 | GPIO2 | Onboard LED, pull-up |
| D5 | GPIO14 | SPI SCK |
| D6 | GPIO12 | SPI MISO |
| D7 | GPIO13 | SPI MOSI |
| D8 | GPIO15 | Pull-down at boot |

---

## 2. MPU6050 (GY-521 Module)

| Spec | Value |
|------|-------|
| Chip | InvenSense MPU-6050 |
| Interface | I2C (up to 400 kHz) |
| Supply Voltage | 3.3V (module: 3.3–5V) |
| Accelerometer | 3-axis, ±2/4/8/16g |
| Gyroscope | 3-axis, ±250/500/1000/2000 °/s |
| ADC | 16-bit for all axes |
| Temperature | Built-in sensor |
| I2C Address | 0x68 (AD0=0), 0x69 (AD0=1) |
| DLPF | Configurable 5–260 Hz |
| Sample Rate | Up to 8 kHz (gyro) |

### Register Map (used in this sketch)

| Register | Address | Purpose |
|----------|---------|---------|
| PWR_MGMT_1 | 0x6B | Wake up / sleep control |
| CONFIG | 0x1A | DLPF bandwidth setting |
| GYRO_CONFIG | 0x1B | Gyro full-scale range |
| ACCEL_CONFIG | 0x1C | Accel full-scale range |
| ACCEL_XOUT_H | 0x3B | Start of 14-byte sensor data |
| GYRO_XOUT_H | 0x43 | Gyro X high byte |

### Sensitivity Values

| Accel Range | Sensitivity |
|-------------|------------|
| ±2g | 16384 LSB/g |
| ±4g | 8192 LSB/g |
| ±8g | 4096 LSB/g |
| ±16g | 2048 LSB/g |

| Gyro Range | Sensitivity |
|------------|-----------|
| ±250 °/s | 131 LSB/°/s |
| ±500 °/s | 65.5 LSB/°/s |
| ±1000 °/s | 32.8 LSB/°/s |
| ±2000 °/s | 16.4 LSB/°/s |

---

## 3. Complementary Filter — Technical Detail

The complementary filter is a high-pass + low-pass filter combination:

```
pitch = α × (pitch + ωx × dt) + (1 - α) × θaccel

Where:
  α     = 0.98 (filter coefficient)
  ωx    = gyroscope X angular rate (deg/s)
  dt    = time since last sample (seconds)
  θaccel = atan2(-ax, √(ay² + az²)) × (180/π)
```

**Why complementary filter over Kalman?**
- Simpler to implement — no matrix math
- Lower computational cost (important for ESP8266 at 80MHz)
- Sufficient accuracy for barrier position detection (±1°)
- Kalman filter is better for aircraft-grade precision — overkill here

---

## 4. LCD 16×2 with I2C Backpack (PCF8574)

| Spec | Value |
|------|-------|
| Display | 16 columns × 2 rows |
| Controller | HD44780 |
| I2C Bridge | PCF8574 expander |
| Operating Voltage | 5V (module) or 3.3V (check your module) |
| I2C Address | 0x27 (most common) or 0x3F |
| Backlight | LED (software controllable) |

---

## Useful Links

- [MPU-6050 Product Specification](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [MPU-6050 Register Map](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf)
- [ESP8266 Datasheet](https://www.espressif.com/sites/default/files/documentation/0a-esp8266ex_datasheet_en.pdf)
- [NodeMCU Pinout Reference](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/)
- [ThingSpeak MATLAB Visualizations](https://uk.mathworks.com/help/thingspeak/visualize-data.html)
- [Complementary Filter Theory](https://www.pieter-jan.com/node/11)
- [LiquidCrystal_I2C Library](https://github.com/johnrickman/LiquidCrystal_I2C)
