# Circuit Diagram & Wiring Guide
## IoT Bridge Health Monitoring System

---

## 1. MPU6050 — Vibration & Tilt Sensor

```
MPU6050 Pin   →   Arduino Pin
─────────────────────────────
VCC           →   3.3V  ⚠️ (NOT 5V)
GND           →   GND
SDA           →   A4
SCL           →   A5
INT           →   (optional) D2
AD0           →   GND (I2C address = 0x68)
```

> Place the MPU6050 flat and level on the bridge deck surface.
> Use hot glue or mounting screws for secure attachment.

---

## 2. HX711 + Load Cell — Weight / Strain

```
HX711 Pin     →   Arduino Pin
─────────────────────────────
VCC           →   5V
GND           →   GND
DT (DOUT)     →   D4
SCK           →   D5

Load Cell Wires → HX711 terminals:
  Red   → E+
  Black → E-
  White → A-
  Green → A+
```

> Mount the load cell under the bridge deck or on support pillars.
> Run calibration with known weights before deployment.

---

## 3. DHT22 — Temperature & Humidity

```
DHT22 Pin     →   Arduino Pin
─────────────────────────────
Pin 1 (VCC)   →   5V
Pin 2 (DATA)  →   D7  (+ 10kΩ pull-up to 5V)
Pin 3          →   Not connected
Pin 4 (GND)   →   GND
```

> Install in a ventilated enclosure, shaded from direct sunlight.

---

## 4. MQ-2 — Gas / Smoke Sensor

```
MQ-2 Pin      →   Arduino Pin
─────────────────────────────
VCC           →   5V
GND           →   GND
AO            →   A1
DO            →   D3
```

> Allow 2-minute warm-up after power on for accurate readings.
> Adjust the onboard potentiometer for digital threshold.

---

## 5. Water Level Sensor

```
Water Sensor  →   Arduino Pin
─────────────────────────────
VCC           →   5V
GND           →   GND
AO            →   A2
DO            →   D2
```

> Mount vertically under bridge deck, near water surface.
> Use waterproof housing/potting for long-term outdoor use.

---

## 6. ESP8266 (ESP-01) — WiFi Module

```
ESP8266 Pin   →   Connection
──────────────────────────────────────────────
VCC           →   3.3V  ⚠️ (NOT 5V — will damage module)
GND           →   GND
CH_PD (EN)    →   3.3V  (must be HIGH to enable)
RST           →   3.3V  (or leave floating)
TX            →   D12 (Arduino SoftwareSerial RX) — direct
RX            →   D13 (Arduino TX) via voltage divider ⬇️
```

### Voltage Divider (5V → 3.3V) for ESP RX:
```
Arduino D13 ──[1kΩ]──┬── ESP8266 RX
                      │
                    [2kΩ]
                      │
                     GND
```

> ⚠️ The ESP8266 is a 3.3V device. Never connect 5V directly to its RX or VCC pins.

---

## 7. LCD 16×2 I2C Display

```
LCD Pin       →   Arduino Pin
─────────────────────────────
VCC           →   5V
GND           →   GND
SDA           →   A4
SCL           →   A5
```

> I2C Address is usually 0x27 or 0x3F.
> Run I2C scanner sketch to confirm your module's address.

---

## 8. LEDs + Buzzer

```
Component     →   Arduino Pin   →   Notes
──────────────────────────────────────────
Green LED     →   D9            →   220Ω to GND (NORMAL)
Yellow LED    →   D10           →   220Ω to GND (WARNING)
Red LED       →   D11           →   220Ω to GND (DANGER)
Active Buzzer →   D8 (+), GND   →   5V active buzzer
```

---

## Power Supply Recommendation

| Component | Current Draw |
|-----------|-------------|
| Arduino Mega | ~90 mA |
| ESP8266 (peak) | ~250 mA |
| MPU6050 | ~3.9 mA |
| HX711 | ~1.5 mA |
| DHT22 | ~2.5 mA |
| MQ-2 (heating) | ~150 mA |
| LCD Backlight | ~20 mA |
| **Total (estimate)** | **~520 mA** |

> Use a **5V 2A** regulated power supply.
> Add a **100µF capacitor** across 5V/GND near ESP8266 for stability.
> For outdoor/field deployment, use a **12V battery + buck converter (5V 2A)**.
