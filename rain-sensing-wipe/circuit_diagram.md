# Circuit Diagram & Wiring Guide

## Rain Sensor Module (FC-37 / YL-83)

The rain sensor has two boards:
1. **Sensing pad** — exposed copper traces that detect water
2. **Control board** — with comparator (LM393) and potentiometer

### Connections

```
Rain Sensor ──────────────── Arduino
    VCC   ────────────────── 5V
    GND   ────────────────── GND
    AO    ────────────────── A0   (Analog, 0–1023)
    DO    ────────────────── D2   (Digital, HIGH/LOW)
```

> **Note:** The potentiometer on the sensor board adjusts the digital output threshold.
> Turn it clockwise to increase sensitivity.

---

## Servo Motor (SG90)

```
Servo ─────────────────────── Arduino
  Red (VCC)   ──────────────── 5V
  Brown (GND) ──────────────── GND
  Orange (Signal) ─────────── D9 (PWM)
```

> **Note:** For larger servos (MG995), use a separate 5–6V power supply.
> Share GND with Arduino.

---

## I2C LCD 16×2

```
LCD ───────────────────────── Arduino
  VCC   ────────────────────── 5V
  GND   ────────────────────── GND
  SDA   ────────────────────── A4
  SCL   ────────────────────── A5
```

> **Finding I2C Address:** Run the I2C scanner sketch.
> Most modules use `0x27` or `0x3F`.
> Update `LiquidCrystal_I2C lcd(0x27, 16, 2);` accordingly.

---

## LEDs (with 220Ω resistors)

```
Arduino D3 ── [220Ω] ── Green LED (+) ── GND    (OFF mode)
Arduino D4 ── [220Ω] ── Yellow LED (+) ── GND   (SLOW mode)
Arduino D5 ── [220Ω] ── Orange LED (+) ── GND   (MEDIUM mode)
Arduino D6 ── [220Ω] ── Red LED (+) ── GND      (FAST mode)
```

---

## Buzzer (Active, 5V)

```
Arduino D8 ──── Buzzer (+)
GND        ──── Buzzer (-)
```

---

## Power Supply Options

| Option | Notes |
|--------|-------|
| USB (5V via PC) | For testing and development |
| 9V Battery + DC jack | For standalone demo |
| 12V adapter via Vin | For permanent installation |

> ⚠️ Do not exceed 12V on the Vin pin.