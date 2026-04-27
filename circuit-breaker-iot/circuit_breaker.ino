/**
 * ============================================================
 * IoT Based Circuit Breaker Monitoring and Control System
 * ============================================================
 * Author      : Pasupathi
 * Version     : 1.0.0
 * Board       : Arduino Mega 2560 + ESP8266 (Wi-Fi)
 * Description : Monitors power distribution line parameters
 *               (current, voltage, temperature, fault state)
 *               for lineman protection. Detects faults, trips
 *               the breaker automatically, and allows remote
 *               reset/control via IoT cloud (ThingSpeak).
 *
 * Sensors & Modules:
 *   - ACS712      -> AC/DC Current sensing (30A version)
 *   - ZMPT101B    -> AC Voltage sensing
 *   - DHT22       -> Temperature & humidity (panel heat)
 *   - Relay Module -> Circuit breaker trip/reset control
 *   - ESP8266     -> WiFi IoT connectivity
 *   - 16x2 LCD   -> Local display
 *   - Buzzer+LEDs -> Local alerts
 *   - Push Buttons -> Manual trip and reset (lineman control)
 *
 * Fault Detection:
 *   - Overcurrent      -> Current exceeds safe threshold
 *   - Short Circuit    -> Sudden massive current spike
 *   - Overvoltage      -> Voltage exceeds safe threshold
 *   - Undervoltage     -> Voltage drops below minimum
 *   - Overtemperature  -> Panel overheating
 *   - Manual Trip      -> Lineman presses trip for safety
 *
 * Breaker States:
 *   CLOSED    -> Normal (power flowing)
 *   TRIPPED   -> Fault or manual (power cut, lineman safe)
 *   RESETTING -> Reset in progress
 *
 * ThingSpeak Fields:
 *   field1=current(A)  field2=voltage(V)  field3=power(W)
 *   field4=temp(C)     field5=humidity(%) field6=breaker state
 *   field7=fault type  field8=fault count
 * ============================================================
 */

#include <Wire.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

// ─────────────────────────────────────────────
// PIN CONFIGURATION
// ─────────────────────────────────────────────
#define CURRENT_SENSOR_PIN    A0   // ACS712 analog output
#define VOLTAGE_SENSOR_PIN    A1   // ZMPT101B analog output
#define DHT_PIN               7    // DHT22 data
#define DHT_TYPE              DHT22

#define RELAY_PIN             4    // Relay -> circuit breaker coil
#define MANUAL_RESET_BTN      2    // INT0 - reset button
#define MANUAL_TRIP_BTN       3    // INT1 - trip button (lineman safety)

#define BUZZER_PIN            8
#define LED_GREEN_PIN         9    // CLOSED / normal
#define LED_YELLOW_PIN        10   // WARNING
#define LED_RED_PIN           11   // TRIPPED / DANGER

#define ESP_RX_PIN            12
#define ESP_TX_PIN            13

// ─────────────────────────────────────────────
// SENSOR CALIBRATION
// ACS712-30A: 66mV/A sensitivity, 2.5V at 0A
// ─────────────────────────────────────────────
#define ACS712_SENSITIVITY    0.066    // V/A
#define ACS712_ZERO_OFFSET    2.5      // V at 0A
#define ADC_VREF              5.0
#define ADC_RESOLUTION        1024.0
#define VOLTAGE_CALIBRATION   233.0    // Scale factor for ZMPT101B
#define SAMPLE_COUNT          500      // ADC samples per measurement

// ─────────────────────────────────────────────
// PROTECTION THRESHOLDS
// ─────────────────────────────────────────────
#define CURRENT_WARNING       20.0    // A
#define CURRENT_TRIP          28.0    // A - overcurrent trip
#define CURRENT_SHORTCIRCUIT  40.0    // A - instant short circuit trip

#define VOLTAGE_WARNING_HIGH  245.0   // V AC
#define VOLTAGE_TRIP_HIGH     260.0   // V AC - overvoltage trip
#define VOLTAGE_WARNING_LOW   200.0   // V AC
#define VOLTAGE_TRIP_LOW      180.0   // V AC - undervoltage trip

#define TEMP_WARNING          60.0    // degrees C
#define TEMP_TRIP             75.0    // degrees C - thermal trip

// ─────────────────────────────────────────────
// TIMING (ms)
// ─────────────────────────────────────────────
#define SENSOR_READ_INTERVAL      500
#define CLOUD_UPLOAD_INTERVAL     20000
#define DISPLAY_UPDATE_INTERVAL   2000
#define FAULT_LOG_EEPROM_ADDR     0

// ─────────────────────────────────────────────
// OBJECTS
// ─────────────────────────────────────────────
DHT               dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial    espSerial(ESP_RX_PIN, ESP_TX_PIN);

// ─────────────────────────────────────────────
// THINGSPEAK CONFIG - update before deploying
// ─────────────────────────────────────────────
const String WIFI_SSID  = "YOUR_WIFI_SSID";
const String WIFI_PASS  = "YOUR_WIFI_PASSWORD";
const String TS_API_KEY = "YOUR_THINGSPEAK_WRITE_API_KEY";
const String TS_HOST    = "api.thingspeak.com";

// ─────────────────────────────────────────────
// ENUMS
// ─────────────────────────────────────────────
enum BreakerState {
  BREAKER_CLOSED,
  BREAKER_TRIPPED,
  BREAKER_RESETTING
};

enum FaultType {
  FAULT_NONE,
  FAULT_OVERCURRENT,
  FAULT_SHORT_CIRCUIT,
  FAULT_OVERVOLTAGE,
  FAULT_UNDERVOLTAGE,
  FAULT_OVERTEMPERATURE,
  FAULT_MANUAL_TRIP
};

// ─────────────────────────────────────────────
// LIVE DATA STRUCTURE
// ─────────────────────────────────────────────
struct LineData {
  float         currentRMS;
  float         voltageRMS;
  float         powerWatts;
  float         temperature;
  float         humidity;
  BreakerState  breaker;
  FaultType     fault;
  unsigned long faultCount;
};

// ─────────────────────────────────────────────
// GLOBAL STATE
// ─────────────────────────────────────────────
LineData      liveData;
unsigned long lastSensorRead    = 0;
unsigned long lastCloudUpload   = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastBeep          = 0;
int           displayPage       = 0;
bool          espReady          = false;

volatile bool resetBtnPressed   = false;
volatile bool tripBtnPressed    = false;

// ─────────────────────────────────────────────
// FUNCTION DECLARATIONS
// ─────────────────────────────────────────────
void      readSensors(LineData &data);
float     measureCurrentRMS();
float     measureVoltageRMS();
FaultType evaluateFaults(LineData &data);
void      tripBreaker(FaultType reason, LineData &data);
void      resetBreaker(LineData &data);
void      updateOutputs(LineData &data);
void      updateLCD(LineData &data, int page);
void      uploadToCloud(LineData &data);
void      sendATCommand(String cmd, int timeout);
bool      initESP8266();
void      serialPrintData(LineData &data);
String    breakerStateToString(BreakerState s);
String    faultTypeToString(FaultType f);
void      ISR_resetBtn();
void      ISR_tripBtn();

// ─────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  espSerial.begin(115200);

  Serial.println(F("====================================================="));
  Serial.println(F("  IoT Circuit Breaker Monitor & Control  v1.0.0    "));
  Serial.println(F("====================================================="));

  // Configure pins
  pinMode(RELAY_PIN,        OUTPUT);
  pinMode(BUZZER_PIN,       OUTPUT);
  pinMode(LED_GREEN_PIN,    OUTPUT);
  pinMode(LED_YELLOW_PIN,   OUTPUT);
  pinMode(LED_RED_PIN,      OUTPUT);
  pinMode(MANUAL_RESET_BTN, INPUT_PULLUP);
  pinMode(MANUAL_TRIP_BTN,  INPUT_PULLUP);

  // Hardware interrupts for buttons
  attachInterrupt(digitalPinToInterrupt(MANUAL_RESET_BTN), ISR_resetBtn, FALLING);
  attachInterrupt(digitalPinToInterrupt(MANUAL_TRIP_BTN),  ISR_tripBtn,  FALLING);

  // Start with breaker CLOSED (relay energized = power on)
  digitalWrite(RELAY_PIN, HIGH);
  liveData.breaker    = BREAKER_CLOSED;
  liveData.fault      = FAULT_NONE;
  liveData.faultCount = 0;

  // Load persistent fault count from EEPROM
  EEPROM.get(FAULT_LOG_EEPROM_ADDR, liveData.faultCount);
  if (liveData.faultCount == 0xFFFFFFFF) liveData.faultCount = 0;

  dht.begin();

  // LCD splash
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print(F("Circuit Breaker "));
  lcd.setCursor(0, 1); lcd.print(F("Initializing... "));
  delay(1500);

  // LED self-test
  digitalWrite(LED_GREEN_PIN,  HIGH); delay(300);
  digitalWrite(LED_YELLOW_PIN, HIGH); delay(300);
  digitalWrite(LED_RED_PIN,    HIGH); delay(300);
  digitalWrite(LED_GREEN_PIN,  LOW);
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_RED_PIN,    LOW);

  // Startup beeps
  tone(BUZZER_PIN, 1000, 100); delay(150);
  tone(BUZZER_PIN, 1500, 100); delay(150);
  noTone(BUZZER_PIN);

  espReady = initESP8266();
  updateOutputs(liveData);
  lcd.clear();

  Serial.println(F("System armed. Breaker CLOSED. Monitoring started."));
  Serial.print(F("Total faults since install: "));
  Serial.println(liveData.faultCount);
}

// ─────────────────────────────────────────────
// MAIN LOOP
// ─────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // Handle manual button presses (set by ISR)
  if (resetBtnPressed) {
    resetBtnPressed = false;
    if (liveData.breaker == BREAKER_TRIPPED) {
      Serial.println(F("Manual RESET pressed"));
      resetBreaker(liveData);
    }
  }
  if (tripBtnPressed) {
    tripBtnPressed = false;
    if (liveData.breaker == BREAKER_CLOSED) {
      Serial.println(F("Manual TRIP pressed -- lineman safety isolation"));
      tripBreaker(FAULT_MANUAL_TRIP, liveData);
    }
  }

  // Read sensors at interval
  if (now - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = now;
    readSensors(liveData);

    // Only auto-evaluate faults when breaker is closed
    if (liveData.breaker == BREAKER_CLOSED) {
      FaultType detected = evaluateFaults(liveData);
      if (detected != FAULT_NONE) {
        tripBreaker(detected, liveData);
      }
    }

    updateOutputs(liveData);
    serialPrintData(liveData);
  }

  // Rotate LCD pages
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = now;
    updateLCD(liveData, displayPage);
    displayPage = (displayPage + 1) % 4;
  }

  // Repeating buzzer alert when tripped
  if (liveData.breaker == BREAKER_TRIPPED) {
    if (now - lastBeep >= 400) {
      lastBeep = now;
      tone(BUZZER_PIN, 1000, 150);
    }
  }

  // Cloud upload
  if (espReady && (now - lastCloudUpload >= CLOUD_UPLOAD_INTERVAL)) {
    lastCloudUpload = now;
    uploadToCloud(liveData);
  }
}

// ─────────────────────────────────────────────
// READ ALL SENSORS
// ─────────────────────────────────────────────
void readSensors(LineData &data) {
  data.currentRMS = measureCurrentRMS();
  data.voltageRMS = measureVoltageRMS();
  // Power with assumed power factor 0.9 (inductive load)
  data.powerWatts = data.voltageRMS * data.currentRMS * 0.9;
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) data.temperature = t;
  if (!isnan(h)) data.humidity    = h;
}

// ─────────────────────────────────────────────
// MEASURE CURRENT RMS (ACS712)
// Uses 500 samples over ~50ms (covers 2.5 cycles at 50Hz)
// ─────────────────────────────────────────────
float measureCurrentRMS() {
  float sumSquares = 0.0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    int   raw     = analogRead(CURRENT_SENSOR_PIN);
    float voltage = (raw / ADC_RESOLUTION) * ADC_VREF;
    float current = (voltage - ACS712_ZERO_OFFSET) / ACS712_SENSITIVITY;
    sumSquares += current * current;
    delayMicroseconds(100);
  }
  return sqrt(sumSquares / SAMPLE_COUNT);
}

// ─────────────────────────────────────────────
// MEASURE VOLTAGE RMS (ZMPT101B)
// ─────────────────────────────────────────────
float measureVoltageRMS() {
  float sumSquares = 0.0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    int   raw        = analogRead(VOLTAGE_SENSOR_PIN);
    float adcVoltage = (raw / ADC_RESOLUTION) * ADC_VREF;
    float centered   = adcVoltage - (ADC_VREF / 2.0);
    sumSquares += centered * centered;
    delayMicroseconds(100);
  }
  return sqrt(sumSquares / SAMPLE_COUNT) * VOLTAGE_CALIBRATION;
}

// ─────────────────────────────────────────────
// EVALUATE FAULTS (priority order)
// ─────────────────────────────────────────────
FaultType evaluateFaults(LineData &data) {
  if (data.currentRMS >= CURRENT_SHORTCIRCUIT)                      return FAULT_SHORT_CIRCUIT;
  if (data.currentRMS >= CURRENT_TRIP)                              return FAULT_OVERCURRENT;
  if (data.voltageRMS >= VOLTAGE_TRIP_HIGH)                         return FAULT_OVERVOLTAGE;
  if (data.voltageRMS > 0 && data.voltageRMS <= VOLTAGE_TRIP_LOW)  return FAULT_UNDERVOLTAGE;
  if (data.temperature >= TEMP_TRIP)                                return FAULT_OVERTEMPERATURE;
  return FAULT_NONE;
}

// ─────────────────────────────────────────────
// TRIP THE BREAKER
// ─────────────────────────────────────────────
void tripBreaker(FaultType reason, LineData &data) {
  digitalWrite(RELAY_PIN, LOW);   // De-energize relay = open circuit = power OFF
  data.breaker = BREAKER_TRIPPED;
  data.fault   = reason;
  data.faultCount++;

  // Persist fault count to EEPROM
  EEPROM.put(FAULT_LOG_EEPROM_ADDR, data.faultCount);

  Serial.print(F(">>> BREAKER TRIPPED | Fault: "));
  Serial.println(faultTypeToString(reason));
  Serial.println(F("    POWER CUT. Lineman work area is now safe."));

  tone(BUZZER_PIN, 1200, 1500);
  updateOutputs(data);
}

// ─────────────────────────────────────────────
// RESET THE BREAKER
// Checks conditions before re-closing
// ─────────────────────────────────────────────
void resetBreaker(LineData &data) {
  Serial.println(F("Initiating breaker reset..."));
  data.breaker = BREAKER_RESETTING;

  delay(2000);  // Safety delay

  // Re-check live conditions before closing
  readSensors(data);
  FaultType check = evaluateFaults(data);

  if (check != FAULT_NONE) {
    Serial.print(F("Reset BLOCKED -- fault still active: "));
    Serial.println(faultTypeToString(check));
    data.breaker = BREAKER_TRIPPED;
    data.fault   = check;
    tone(BUZZER_PIN, 400, 500);
    return;
  }

  digitalWrite(RELAY_PIN, HIGH);  // Energize relay = close circuit = power ON
  data.breaker = BREAKER_CLOSED;
  data.fault   = FAULT_NONE;

  Serial.println(F("Breaker CLOSED -- power restored"));
  tone(BUZZER_PIN, 1500, 200);
  updateOutputs(data);
}

// ─────────────────────────────────────────────
// UPDATE LED INDICATORS
// ─────────────────────────────────────────────
void updateOutputs(LineData &data) {
  bool warn = (data.currentRMS  >= CURRENT_WARNING     ||
               data.voltageRMS  >= VOLTAGE_WARNING_HIGH ||
               data.voltageRMS  <= VOLTAGE_WARNING_LOW  ||
               data.temperature >= TEMP_WARNING);

  switch (data.breaker) {
    case BREAKER_CLOSED:
      digitalWrite(LED_GREEN_PIN,  warn ? LOW  : HIGH);
      digitalWrite(LED_YELLOW_PIN, warn ? HIGH : LOW);
      digitalWrite(LED_RED_PIN,    LOW);
      break;
    case BREAKER_TRIPPED:
      digitalWrite(LED_GREEN_PIN,  LOW);
      digitalWrite(LED_YELLOW_PIN, LOW);
      digitalWrite(LED_RED_PIN,    HIGH);
      break;
    case BREAKER_RESETTING:
      digitalWrite(LED_GREEN_PIN,  LOW);
      digitalWrite(LED_YELLOW_PIN, HIGH);
      digitalWrite(LED_RED_PIN,    LOW);
      break;
  }
}

// ─────────────────────────────────────────────
// LCD DISPLAY -- 4 ROTATING PAGES
// ─────────────────────────────────────────────
void updateLCD(LineData &data, int page) {
  lcd.clear();
  switch (page) {
    case 0:  // Current & Voltage
      lcd.setCursor(0,0); lcd.print(F("Current:        "));
      lcd.setCursor(9,0); lcd.print(data.currentRMS, 1); lcd.print(F("A"));
      lcd.setCursor(0,1); lcd.print(F("Voltage:        "));
      lcd.setCursor(9,1); lcd.print(data.voltageRMS, 0); lcd.print(F("V"));
      break;
    case 1:  // Power & Temperature
      lcd.setCursor(0,0); lcd.print(F("Power:          "));
      lcd.setCursor(7,0); lcd.print(data.powerWatts, 0); lcd.print(F("W"));
      lcd.setCursor(0,1); lcd.print(F("Temp:           "));
      lcd.setCursor(6,1); lcd.print(data.temperature, 1); lcd.print(F("C"));
      break;
    case 2:  // Breaker state & fault count
      lcd.setCursor(0,0); lcd.print(F("Breaker:        "));
      lcd.setCursor(9,0); lcd.print(breakerStateToString(data.breaker));
      lcd.setCursor(0,1); lcd.print(F("Faults:         "));
      lcd.setCursor(8,1); lcd.print(data.faultCount);
      break;
    case 3:  // Active fault type
      lcd.setCursor(0,0); lcd.print(F("Fault Type:     "));
      lcd.setCursor(0,1); lcd.print(faultTypeToString(data.fault));
      break;
  }
}

// ─────────────────────────────────────────────
// UPLOAD TO THINGSPEAK VIA ESP8266
// ─────────────────────────────────────────────
void uploadToCloud(LineData &data) {
  Serial.println(F("Uploading to ThingSpeak..."));
  String payload = "GET /update?api_key=" + TS_API_KEY
    + "&field1=" + String(data.currentRMS, 2)
    + "&field2=" + String(data.voltageRMS, 1)
    + "&field3=" + String(data.powerWatts, 1)
    + "&field4=" + String(data.temperature, 1)
    + "&field5=" + String(data.humidity, 1)
    + "&field6=" + String((int)data.breaker)
    + "&field7=" + String((int)data.fault)
    + "&field8=" + String(data.faultCount)
    + " HTTP/1.1\r\nHost: " + TS_HOST + "\r\nConnection: close\r\n\r\n";

  sendATCommand("AT+CIPSTART=\"TCP\",\"" + TS_HOST + "\",80", 3000);
  delay(100);
  sendATCommand("AT+CIPSEND=" + String(payload.length()), 2000);
  espSerial.print(payload);
  delay(2000);
  sendATCommand("AT+CIPCLOSE", 1000);
  Serial.println(F("Upload complete."));
}

// ─────────────────────────────────────────────
// INIT ESP8266
// ─────────────────────────────────────────────
bool initESP8266() {
  Serial.println(F("Connecting ESP8266 to WiFi..."));
  sendATCommand("AT",           1000);
  sendATCommand("AT+CWMODE=1",  1000);
  sendATCommand("AT+CWJAP=\"" + WIFI_SSID + "\",\"" + WIFI_PASS + "\"", 8000);
  if (espSerial.find("OK")) {
    Serial.println(F("ESP8266: Connected"));
    return true;
  }
  Serial.println(F("ESP8266: Failed -- running offline"));
  return false;
}

void sendATCommand(String cmd, int timeout) {
  espSerial.println(cmd);
  long t = millis();
  while ((millis() - t) < timeout) {
    if (espSerial.available()) Serial.write(espSerial.read());
  }
}

// ─────────────────────────────────────────────
// SERIAL DEBUG OUTPUT
// ─────────────────────────────────────────────
void serialPrintData(LineData &data) {
  Serial.println(F("------------------------------------"));
  Serial.print(F("Current     : ")); Serial.print(data.currentRMS, 2);  Serial.println(F(" A"));
  Serial.print(F("Voltage     : ")); Serial.print(data.voltageRMS, 1);  Serial.println(F(" V"));
  Serial.print(F("Power       : ")); Serial.print(data.powerWatts, 1);  Serial.println(F(" W"));
  Serial.print(F("Temperature : ")); Serial.print(data.temperature, 1); Serial.println(F(" C"));
  Serial.print(F("Humidity    : ")); Serial.print(data.humidity, 1);    Serial.println(F(" %"));
  Serial.print(F("Breaker     : ")); Serial.println(breakerStateToString(data.breaker));
  Serial.print(F("Fault       : ")); Serial.println(faultTypeToString(data.fault));
  Serial.print(F("Fault Count : ")); Serial.println(data.faultCount);
  Serial.println(F("------------------------------------"));
}

// ─────────────────────────────────────────────
// INTERRUPT SERVICE ROUTINES
// ─────────────────────────────────────────────
void ISR_resetBtn() { resetBtnPressed = true; }
void ISR_tripBtn()  { tripBtnPressed  = true; }

// ─────────────────────────────────────────────
// HELPER FUNCTIONS
// ─────────────────────────────────────────────
String breakerStateToString(BreakerState s) {
  switch (s) {
    case BREAKER_CLOSED:    return "CLOSED";
    case BREAKER_TRIPPED:   return "TRIPPED";
    case BREAKER_RESETTING: return "RESET...";
    default:                return "UNKNOWN";
  }
}

String faultTypeToString(FaultType f) {
  switch (f) {
    case FAULT_NONE:            return "NONE";
    case FAULT_OVERCURRENT:     return "OVERCURRENT";
    case FAULT_SHORT_CIRCUIT:   return "SHORT CIRCUIT";
    case FAULT_OVERVOLTAGE:     return "OVERVOLTAGE";
    case FAULT_UNDERVOLTAGE:    return "UNDERVOLTAGE";
    case FAULT_OVERTEMPERATURE: return "OVER TEMP";
    case FAULT_MANUAL_TRIP:     return "MANUAL TRIP";
    default:                    return "UNKNOWN";
  }
}
