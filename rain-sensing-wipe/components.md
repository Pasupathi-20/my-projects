# Component Details & Datasheets

## 1. Arduino Uno R3

| Spec | Value |
|------|-------|
| Microcontroller | ATmega328P |
| Operating Voltage | 5V |
| Digital I/O Pins | 14 (6 PWM) |
| Analog Input Pins | 6 |
| Flash Memory | 32 KB |
| SRAM | 2 KB |
| Clock Speed | 16 MHz |

---

## 2. Rain / Water Sensor Module (FC-37)

| Spec | Value |
|------|-------|
| Operating Voltage | 3.3V – 5V |
| Output | Analog (AO) + Digital (DO) |
| Analog Range | 0 – 1023 (Arduino ADC) |
| Digital Output | HIGH = Dry, LOW = Wet |
| Sensitivity | Adjustable via onboard potentiometer |

### Analog Value Reference (approximate)

| Condition | Analog Value |
|-----------|-------------|
| Completely dry | 1023 |
| Slightly damp | 800 – 1000 |
| Light rain | 600 – 800 |
| Moderate rain | 300 – 600 |
| Heavy rain / submerged | 0 – 300 |

---

## 3. Servo Motor SG90

| Spec | Value |
|------|-------|
| Operating Voltage | 4.8V – 6V |
| Stall Torque | 1.8 kg·cm @ 4.8V |
| Operating Speed | 0.1 sec/60° @ 4.8V |
| Rotation Range | 0° – 180° |
| Signal | PWM (50Hz, 1–2ms pulse) |
| Weight | ~9g |

> For heavier physical wiper arms, use **MG995** (10kg·cm torque).

---

## 4. LM2596 I2C LCD 16×2

| Spec | Value |
|------|-------|
| Characters | 16 columns × 2 rows |
| Interface | I2C (via PCF8574 backpack) |
| Operating Voltage | 5V |
| I2C Address | 0x27 or 0x3F |
| Backlight | LED (controllable) |

---

## 5. Active Buzzer (5V)

| Spec | Value |
|------|-------|
| Operating Voltage | 3.5V – 5.5V |
| Frequency | ~2.3 kHz (internal oscillator) |
| Current | ~30 mA |
| Type | Active (no external oscillation needed) |

---

## Useful Datasheets & Links

- [ATmega328P Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf)
- [SG90 Servo Datasheet](http://www.ee.ic.ac.uk/pcheung/teaching/DE1_EE/stores/sg90_datasheet.pdf)
- [LM393 Comparator (Rain Sensor IC)](https://www.ti.com/lit/ds/symlink/lm393-n.pdf)
- [Arduino Servo Library Docs](https://www.arduino.cc/reference/en/libraries/servo/)
- [LiquidCrystal_I2C Library](https://github.com/johnrickman/LiquidCrystal_I2C)
