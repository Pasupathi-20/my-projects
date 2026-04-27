# Circuit Diagram & Wiring Guide
## Boom Barrier Position Detection — NodeMCU + MPU6050

---

## 1. MPU6050 GY-521 Module

The GY-521 breakout has an onboard 3.3V regulator, so you can safely
feed it 5V or connect directly to the NodeMCU 3.3V pin.

```
GY-521 Pin    →    NodeMCU Pin
──────────────────────────────
VCC           →    3.3V (or 5V — module has onboard regulator)
GND           →    GND
SDA           →    D2  (GPIO4)
SCL           →    D1  (GPIO5)
AD0           →    GND  (sets I2C address = 0x68)
INT           →    Not connected (not used in this sketch)
```

### I2C Address:
- AD0 = GND → address **0x68** (default)
- AD0 = VCC → address **0x69** (if two MPU6050s on same bus)

### Physical Mounting:
```
Barrier Arm (horizontal when closed)
─────────────────────────────────────────
         ┌──────────┐
         │ MPU6050  │  ← Mount flush on arm surface
         │  X──►    │     X-axis pointing along arm length
         │  │       │     Z-axis pointing up when arm is closed
         └──────────┘
─────────────────────────────────────────
     Hinge end                  Free end
```

> Fix the sensor with hot glue, double-sided tape, or a 3D-printed bracket.
> Avoid positions close to the motor — EMI can affect I2C communication.

---

## 2. LCD 16×2 I2C Display

The LCD shares the I2C bus with the MPU6050 — different addresses, same wires.

```
LCD I2C Pin   →    NodeMCU Pin
──────────────────────────────
VCC           →    3.3V or 5V
GND           →    GND
SDA           →    D2  (shared with MPU6050)
SCL           →    D1  (shared with MPU6050)
```

> Default I2C address: **0x27** or **0x3F**
> Run I2C scanner sketch to confirm your module's address.
> Update: `LiquidCrystal_I2C lcd(0x27, 16, 2);`

---

## 3. LED Indicators

Each LED needs a 220Ω series resistor to limit current.

```
NodeMCU D5 ──[220Ω]──  Blue  LED (+) ──── GND   (CLOSED)
NodeMCU D6 ──[220Ω]── Yellow LED (+) ──── GND   (OPENING / CLOSING)
NodeMCU D7 ──[220Ω]──  Green LED (+) ──── GND   (OPEN)
```

> NodeMCU GPIO output is 3.3V — a 220Ω resistor gives ~10mA, adequate brightness.

---

## 4. Active Buzzer

```
NodeMCU D8  ──────────  Buzzer (+)
GND         ──────────  Buzzer (-)
```

> Use an **active** buzzer (generates tone internally).
> D8 (GPIO15) is pulled LOW at boot — this is safe for an active buzzer.
> Do not use a **passive** buzzer here; it requires PWM frequency.

---

## 5. Power Options

### Option A — USB Power (Development)
Power NodeMCU via USB from PC or phone charger.
All 3.3V peripherals are powered from NodeMCU's onboard 3.3V regulator.

### Option B — External 5V Adapter (Deployment)
```
5V Adapter → NodeMCU VIN pin
GND        → NodeMCU GND
```

### Option C — Battery (Portable)
```
18650 Li-ion (3.7V) + TP4056 charger + MT3608 boost to 5V
→ NodeMCU VIN
```

### Current Budget

| Component | Current |
|-----------|---------|
| NodeMCU (active WiFi) | ~80 mA |
| MPU6050 | ~3.9 mA |
| LCD (backlight on) | ~20 mA |
| LEDs (1 at a time) | ~10 mA |
| Buzzer (peak) | ~30 mA |
| **Total** | **~145 mA** |

Use **5V 500mA** minimum power supply (USB charger works fine).

---

## 6. I2C Pull-up Resistors

NodeMCU has internal I2C pull-ups enabled by the Wire library.
For cable lengths > 30cm, add external **4.7kΩ** pull-up resistors:

```
3.3V ──[4.7kΩ]── SDA (D2)
3.3V ──[4.7kΩ]── SCL (D1)
```

---

## 7. Wiring Checklist

- [ ] MPU6050 VCC connected to 3.3V (not 5V directly to chip)
- [ ] MPU6050 AD0 tied to GND (I2C address 0x68)
- [ ] LCD and MPU6050 share SDA/SCL lines — this is correct, I2C is a bus
- [ ] Each LED has a 220Ω resistor in series
- [ ] Buzzer is ACTIVE type (not passive)
- [ ] NodeMCU powered via USB or VIN (not 3.3V pin directly)
- [ ] MPU6050 physically secured to barrier arm, X-axis along arm length
