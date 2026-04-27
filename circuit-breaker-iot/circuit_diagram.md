# Circuit Diagram & Wiring Guide
## IoT Circuit Breaker Monitoring and Control System

---

## ⚠️ Safety Warning
This project interfaces with mains AC voltage (230V/110V).
- Use proper insulation on all AC connections
- Use a certified isolation transformer for ZMPT101B
- Never work on AC wiring without proper training
- All AC connections must be made inside a sealed enclosure

---

## 1. ACS712-30A Current Sensor

The ACS712 uses Hall-effect to measure current without direct contact.
The current-carrying wire passes THROUGH the sensor's hole.

```
ACS712 Module    →    Arduino
────────────────────────────
VCC              →    5V
GND              →    GND
OUT              →    A0 (Analog)

AC Line Wire     →    Through ACS712 aperture (no electrical contact)
```

> For a 30A ACS712: sensitivity = 66 mV/A
> For a 20A ACS712: sensitivity = 100 mV/A
> For a  5A ACS712: sensitivity = 185 mV/A
> Update ACS712_SENSITIVITY in the sketch accordingly.

---

## 2. ZMPT101B AC Voltage Sensor

The ZMPT101B uses a small transformer to safely step down AC voltage.

```
ZMPT101B Module  →    Arduino
────────────────────────────
VCC              →    5V
GND              →    GND
OUT              →    A1 (Analog)

AC Input         →    Across Line and Neutral (230V AC)
```

> Adjust the onboard potentiometer until the LCD voltage reading
> matches a multimeter reading on the same line.
> Update VOLTAGE_CALIBRATION constant accordingly.

---

## 3. Relay Module (Circuit Breaker Control)

```
Relay Module     →    Arduino / Line
──────────────────────────────────
VCC              →    5V
GND              →    GND
IN               →    D4 (control signal)

COM (Common)     →    AC Line supply (Live wire)
NO  (Normally Open) → Load / Distribution output
NC  (Normally Closed) → Not connected

Relay HIGH (D4=HIGH) → Contacts closed → Power flows (BREAKER CLOSED)
Relay LOW  (D4=LOW)  → Contacts open  → Power cut   (BREAKER TRIPPED)
```

> Use a relay rated for your line voltage and current.
> For 230V/30A: use a 30A SSR or heavy-duty mechanical relay.
> The onboard relay (SRD-05VDC) is rated for 10A/250VAC — for demo only.
> For real deployment, drive a contactor coil via the relay.

---

## 4. DHT22 Temperature Sensor

```
DHT22 Pin        →    Arduino
──────────────────────────────
Pin 1 (VCC)      →    5V
Pin 2 (DATA)     →    D7 + 10kΩ pull-up to 5V
Pin 3            →    Not connected
Pin 4 (GND)      →    GND
```

> Mount inside the distribution panel enclosure to monitor ambient temperature.

---

## 5. Manual Buttons (TRIP & RESET)

Using Arduino's internal INPUT_PULLUP — no external resistor needed.

```
RESET Button:
  One terminal  →  D2 (INT0)
  Other terminal →  GND
  [Press = D2 goes LOW = interrupt fires]

TRIP Button:
  One terminal  →  D3 (INT1)
  Other terminal →  GND
  [Press = D3 goes LOW = interrupt fires]
```

> Use momentary push buttons (normally open).
> Label them clearly: RED = TRIP (danger), GREEN = RESET

---

## 6. ESP8266 ESP-01

```
ESP8266 Pin      →    Connection
──────────────────────────────────────────────
VCC              →    3.3V (NOT 5V — will damage chip)
GND              →    GND
CH_PD (EN)       →    3.3V (must tie HIGH)
RST              →    3.3V or leave floating
TX               →    D12 (Arduino SoftSerial RX) — direct
RX               →    D13 via voltage divider below
```

Voltage divider (5V Arduino TX → 3.3V ESP RX):
```
D13 ──[1kΩ]──┬── ESP RX
              │
           [2kΩ]
              │
             GND
```

---

## 7. LCD 16×2 I2C

```
LCD I2C Module   →    Arduino
────────────────────────────
VCC              →    5V
GND              →    GND
SDA              →    A4
SCL              →    A5
```

> Default I2C address: 0x27 or 0x3F
> Run I2C scanner to confirm. Update `LiquidCrystal_I2C lcd(0x27, 16, 2)` if needed.

---

## 8. LEDs and Buzzer

```
Green LED  (+) → D9  → 220Ω → GND    (NORMAL / CLOSED)
Yellow LED (+) → D10 → 220Ω → GND    (WARNING)
Red LED    (+) → D11 → 220Ω → GND    (TRIPPED / DANGER)

Buzzer (+) → D8
Buzzer (-) → GND
```

---

## Power Supply Recommendations

| Component | Current (approx) |
|-----------|-----------------|
| Arduino Mega | 100 mA |
| ESP8266 (peak TX) | 250 mA |
| ACS712 | 10 mA |
| ZMPT101B | 20 mA |
| DHT22 | 2.5 mA |
| Relay coil | 70 mA |
| LCD + LEDs | 30 mA |
| **Total (est.)** | **~485 mA** |

Use a **5V 1A** minimum, **5V 2A** recommended regulated power supply.
Add a **100µF electrolytic** capacitor across 5V/GND near the ESP8266.

For field/outdoor deployment:
- 12V sealed lead-acid battery + 5V 2A buck converter
- Add a low-voltage cutoff circuit to protect the battery
