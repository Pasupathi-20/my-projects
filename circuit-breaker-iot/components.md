# Component Details & Datasheets
## IoT Circuit Breaker Monitoring System

---

## 1. Arduino Mega 2560

| Spec | Value |
|------|-------|
| MCU | ATmega2560 |
| Voltage | 5V |
| Digital I/O | 54 pins (15 PWM) |
| Analog Inputs | 16 |
| Flash | 256 KB |
| SRAM | 8 KB |
| EEPROM | 4 KB |
| Clock | 16 MHz |
| Hardware Interrupts | INT0–INT5 (D2, D3, D18, D19, D20, D21) |

> Mega preferred over Uno — more SRAM, more interrupt pins, hardware Serial ports.

---

## 2. ACS712 Hall-Effect Current Sensor

| Spec | 5A Version | 20A Version | 30A Version |
|------|-----------|------------|------------|
| Sensitivity | 185 mV/A | 100 mV/A | 66 mV/A |
| Supply Voltage | 5V | 5V | 5V |
| Output at 0A | 2.5V | 2.5V | 2.5V |
| Bandwidth | 80 kHz | 80 kHz | 80 kHz |
| Isolation | 2.1 kVrms | 2.1 kVrms | 2.1 kVrms |

> This project uses the 30A version.
> The output voltage swings around 2.5V (midpoint of 5V supply).
> Positive current → voltage above 2.5V
> Negative current → voltage below 2.5V

### RMS Calculation:
Collect ~500 samples over 50ms (covers 2.5 AC cycles at 50Hz),
compute sqrt(mean of squares) for true RMS.

---

## 3. ZMPT101B AC Voltage Transformer

| Spec | Value |
|------|-------|
| Input Voltage | Up to 250V AC |
| Output | Scaled analog (centered at Vcc/2) |
| Bandwidth | 50Hz / 60Hz |
| Operating Voltage | 5V module |
| Isolation | Transformer isolation |

> Onboard potentiometer adjusts the output gain.
> Calibrate by matching LCD reading to a trusted multimeter.
> Never connect directly to mains without the module's transformer.

---

## 4. DHT22 (AM2302)

| Spec | Value |
|------|-------|
| Supply Voltage | 3.3V – 6V |
| Temperature Range | -40 to +80°C |
| Temp Accuracy | ±0.5°C |
| Humidity Range | 0 – 100% RH |
| Humidity Accuracy | ±2% RH |
| Sample Rate | 0.5 Hz (one reading per 2s) |

---

## 5. SRD-05VDC Relay (Onboard module)

| Spec | Value |
|------|-------|
| Coil Voltage | 5V DC |
| Coil Current | ~70 mA |
| Contact Rating | 10A @ 250V AC / 10A @ 30V DC |
| Type | SPDT (Single Pole Double Throw) |

> For higher current loads (>10A), drive a DIN-rail contactor
> (e.g. Schneider LC1D09 or Siemens 3RT) using the relay output as control signal.

---

## 6. ESP8266 ESP-01

| Spec | Value |
|------|-------|
| SoC | Espressif ESP8266EX |
| Operating Voltage | 3.3V (MAX 3.6V) |
| WiFi | 802.11 b/g/n, 2.4GHz |
| Protocol | TCP/IP stack built-in |
| AT Firmware | Required |
| Flash | 1 MB |
| Max TX Current | 170 mA (peak) |

---

## Useful Links & Datasheets

- [ACS712 Datasheet (Allegro)](https://www.allegromicro.com/en/products/sense/current-sensor-ics/zero-to-fifty-amp-integrated-conductor-sensor-ics/acs712)
- [ZMPT101B Module Guide](https://how2electronics.com/zmpt101b-ac-voltage-sensor-module/)
- [DHT22 Datasheet](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf)
- [ESP8266 AT Commands Reference](https://www.espressif.com/sites/default/files/documentation/4a-esp8266_at_instruction_set_en.pdf)
- [ThingSpeak REST API](https://uk.mathworks.com/help/thingspeak/rest-api.html)
- [Arduino EEPROM Library](https://www.arduino.cc/en/Reference/EEPROM)
- [Arduino Interrupt Reference](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/)
