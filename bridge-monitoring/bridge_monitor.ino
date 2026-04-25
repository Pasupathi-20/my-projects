#include <Wire.h>
#include <MPU6050.h>
#include <HX711.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define HX711_DOUT_PIN    4
#define HX711_SCK_PIN     5
#define DHT_PIN           7
#define DHT_TYPE          DHT22
#define MQ2_ANALOG_PIN    A1
#define MQ2_DIGITAL_PIN   3
#define WATER_ANALOG_PIN  A2
#define WATER_DIGITAL_PIN 2
#define BUZZER_PIN        8
#define LED_GREEN_PIN     9   
#define LED_YELLOW_PIN    10   
#define LED_RED_PIN       11   
#define ESP_RX_PIN        12
#define ESP_TX_PIN        13
#define VIBRATION_WARNING   8.0
#define VIBRATION_DANGER    15.0
#define LOAD_WARNING        8000.0   
#define LOAD_DANGER         12000.0  
#define TILT_WARNING        3.0
#define TILT_DANGER         6.0
#define TEMP_WARNING        55.0
#define TEMP_DANGER         75.0
#define WATER_WARNING       400
#define WATER_DANGER        700
#define GAS_WARNING         300
#define GAS_DANGER          600
#define SENSOR_READ_INTERVAL    1000   
#define CLOUD_UPLOAD_INTERVAL   15000  
#define DISPLAY_UPDATE_INTERVAL 2000   
#define ALERT_BEEP_INTERVAL     500    
MPU6050         mpu;
HX711           scale;
DHT             dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial  espSerial(ESP_RX_PIN, ESP_TX_PIN);

const String WIFI_SSID    = "Pasupathi";
const String WIFI_PASS    = "871906";
const String TS_API_KEY   = "***********";
const String TS_HOST      = "api.thingspeak.com";

enum AlertLevel {
  LEVEL_NORMAL,
  LEVEL_WARNING,
  LEVEL_DANGER
};

struct BridgeData {
  float vibration;     
  float tiltAngle;     
  float loadKg;         
  float temperature;    
  float humidity;       
  int   waterLevel;     
  int   gasLevel;       
  AlertLevel alert;
};

BridgeData      currentData;
AlertLevel      previousAlert     = LEVEL_NORMAL;
unsigned long   lastSensorRead    = 0;
unsigned long   lastCloudUpload   = 0;
unsigned long   lastDisplayUpdate = 0;
unsigned long   lastBeep          = 0;
int             displayPage       = 0;   
bool            espReady          = false;

void readAllSensors(BridgeData &data);
AlertLevel evaluateAlertLevel(BridgeData &data);
void updateAlertOutputs(AlertLevel level);
void updateLCD(BridgeData &data, int page);
void uploadToCloud(BridgeData &data);
void sendATCommand(String cmd, int timeout);
bool initESP8266();
void serialPrintData(BridgeData &data);
String alertToString(AlertLevel level);

void setup() {
  Serial.begin(9600);
  espSerial.begin(115200);

  Serial.println(F("============================================"));
  Serial.println(F("  IoT Bridge Health Monitoring System v1.0  "));
  Serial.println(F("============================================"));

  pinMode(MQ2_DIGITAL_PIN,   INPUT);
  pinMode(WATER_DIGITAL_PIN, INPUT);
  pinMode(BUZZER_PIN,        OUTPUT);
  pinMode(LED_GREEN_PIN,     OUTPUT);
  pinMode(LED_YELLOW_PIN,    OUTPUT);
  pinMode(LED_RED_PIN,       OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(F("Bridge Monitor  "));
  lcd.setCursor(0, 1);
  lcd.print(F("Initializing... "));
  delay(1500);

  Wire.begin();
  mpu.initialize();     
  if (mpu.testConnection()) {
    Serial.println(F("MPU6050: Connected"));
  } else {
    Serial.println(F("MPU6050: FAILED — Check wiring"));
  }
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(2280.0);  
  scale.tare();
  Serial.println(F("HX711: Tared and ready"));
  dht.begin();
  Serial.println(F("DHT22: Ready"));
  espReady = initESP8266();
  digitalWrite(LED_GREEN_PIN,  HIGH);
  delay(300);
  digitalWrite(LED_YELLOW_PIN, HIGH);
  delay(300);
  digitalWrite(LED_RED_PIN,    HIGH);
  delay(300);
  digitalWrite(LED_GREEN_PIN,  LOW);
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_RED_PIN,    LOW);
  tone(BUZZER_PIN, 1200, 150); delay(200);
  tone(BUZZER_PIN, 1600, 150); delay(200);
  noTone(BUZZER_PIN);

  lcd.clear();
  Serial.println(F("System Ready. Monitoring started."));
}
void loop() {
  unsigned long now = millis();

  if (now - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = now;

    readAllSensors(currentData);
    currentData.alert = evaluateAlertLevel(currentData);

    if (currentData.alert != previousAlert) {
      Serial.print(F("ALERT LEVEL CHANGED → "));
      Serial.println(alertToString(currentData.alert));
      updateAlertOutputs(currentData.alert);
      previousAlert = currentData.alert;
    }

    serialPrintData(currentData);
  }
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = now;
    updateLCD(currentData, displayPage);
    displayPage = (displayPage + 1) % 4; 
  }

  if (currentData.alert == LEVEL_DANGER) {
    if (now - lastBeep >= ALERT_BEEP_INTERVAL) {
      lastBeep = now;
      tone(BUZZER_PIN, 900, 200);
    }
  } else if (currentData.alert == LEVEL_WARNING) {
    if (now - lastBeep >= 1500) {
      lastBeep = now;
      tone(BUZZER_PIN, 600, 100);
    }
  }

  if (espReady && (now - lastCloudUpload >= CLOUD_UPLOAD_INTERVAL)) {
    lastCloudUpload = now;
    uploadToCloud(currentData);
  }
}
void readAllSensors(BridgeData &data) {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float axG = ax / 16384.0;
  float ayG = ay / 16384.0;
  float azG = az / 16384.0;

  data.vibration = sqrt(axG * axG + ayG * ayG + azG * azG) * 9.81;

  data.tiltAngle = atan2(sqrt(axG * axG + ayG * ayG), azG) * 180.0 / PI;

  if (scale.is_ready()) {
    data.loadKg = scale.get_units(5);  
    if (data.loadKg < 0) data.loadKg = 0;
  }

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) data.temperature = t;
  if (!isnan(h)) data.humidity    = h;

  data.waterLevel = analogRead(WATER_ANALOG_PIN);

  data.gasLevel = analogRead(MQ2_ANALOG_PIN);
}
AlertLevel evaluateAlertLevel(BridgeData &data) {

  if (data.vibration  >= VIBRATION_DANGER  ||
      data.loadKg     >= LOAD_DANGER       ||
      data.tiltAngle  >= TILT_DANGER       ||
      data.temperature >= TEMP_DANGER      ||
      data.waterLevel >= WATER_DANGER      ||
      data.gasLevel   >= GAS_DANGER) {
    return LEVEL_DANGER;
  }

  if (data.vibration  >= VIBRATION_WARNING  ||
      data.loadKg     >= LOAD_WARNING       ||
      data.tiltAngle  >= TILT_WARNING       ||
      data.temperature >= TEMP_WARNING      ||
      data.waterLevel >= WATER_WARNING      ||
      data.gasLevel   >= GAS_WARNING) {
    return LEVEL_WARNING;
  }

  return LEVEL_NORMAL;
}
void updateAlertOutputs(AlertLevel level) {
  digitalWrite(LED_GREEN_PIN,  level == LEVEL_NORMAL  ? HIGH : LOW);
  digitalWrite(LED_YELLOW_PIN, level == LEVEL_WARNING ? HIGH : LOW);
  digitalWrite(LED_RED_PIN,    level == LEVEL_DANGER  ? HIGH : LOW);

  if (level == LEVEL_NORMAL) {
    noTone(BUZZER_PIN);
  }
}
void updateLCD(BridgeData &data, int page) {
  lcd.clear();
  switch (page) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print(F("Vibration:      "));
      lcd.setCursor(10, 0);
      lcd.print(data.vibration, 1);
      lcd.setCursor(0, 1);
      lcd.print(F("Tilt:           "));
      lcd.setCursor(6, 1);
      lcd.print(data.tiltAngle, 1);
      lcd.print(F(" deg"));
      break;

    case 1:
      lcd.setCursor(0, 0);
      lcd.print(F("Load:           "));
      lcd.setCursor(6, 0);
      lcd.print(data.loadKg, 0);
      lcd.print(F(" kg"));
      lcd.setCursor(0, 1);
      lcd.print(F("Temp:           "));
      lcd.setCursor(6, 1);
      lcd.print(data.temperature, 1);
      lcd.print(F(" C"));
      break;

    case 2:
      lcd.setCursor(0, 0);
      lcd.print(F("Humidity:       "));
      lcd.setCursor(10, 0);
      lcd.print(data.humidity, 1);
      lcd.print(F("%"));
      lcd.setCursor(0, 1);
      lcd.print(F("Water Lvl:      "));
      lcd.setCursor(10, 1);
      lcd.print(data.waterLevel);
      break;

    case 3:
      lcd.setCursor(0, 0);
      lcd.print(F("Gas:            "));
      lcd.setCursor(5, 0);
      lcd.print(data.gasLevel);
      lcd.setCursor(0, 1);
      lcd.print(F("Status:         "));
      lcd.setCursor(8, 1);
      lcd.print(alertToString(data.alert));
      break;
  }
}
void uploadToCloud(BridgeData &data) {
  Serial.println(F("Uploading to ThingSpeak..."));

  String payload = "GET /update?api_key=" + TS_API_KEY
    + "&field1=" + String(data.vibration, 2)
    + "&field2=" + String(data.tiltAngle, 2)
    + "&field3=" + String(data.loadKg, 1)
    + "&field4=" + String(data.temperature, 1)
    + "&field5=" + String(data.humidity, 1)
    + "&field6=" + String(data.waterLevel)
    + "&field7=" + String(data.gasLevel)
    + "&field8=" + String((int)data.alert)
    + " HTTP/1.1\r\nHost: " + TS_HOST + "\r\nConnection: close\r\n\r\n";

  sendATCommand("AT+CIPSTART=\"TCP\",\"" + TS_HOST + "\",80", 3000);
  delay(100);
  sendATCommand("AT+CIPSEND=" + String(payload.length()), 2000);
  espSerial.print(payload);
  delay(2000);
  sendATCommand("AT+CIPCLOSE", 1000);

  Serial.println(F("Upload complete."));
}
bool initESP8266() {
  Serial.println(F("Initializing ESP8266..."));
  sendATCommand("AT", 1000);
  sendATCommand("AT+CWMODE=1", 1000);
  sendATCommand("AT+CWJAP=\"" + WIFI_SSID + "\",\"" + WIFI_PASS + "\"", 8000);

  if (espSerial.find("OK")) {
    Serial.println(F("ESP8266: WiFi Connected"));
    return true;
  }
  Serial.println(F("ESP8266: WiFi FAILED — running in offline mode"));
  return false;
}
void sendATCommand(String cmd, int timeout) {
  espSerial.println(cmd);
  long t = millis();
  while ((millis() - t) < timeout) {
    if (espSerial.available()) {
      Serial.write(espSerial.read());
    }
  }
}
void serialPrintData(BridgeData &data) {
  Serial.println(F("─────────────────────────────"));
  Serial.print(F("Vibration : ")); Serial.print(data.vibration, 2); Serial.println(F(" m/s²"));
  Serial.print(F("Tilt      : ")); Serial.print(data.tiltAngle, 2); Serial.println(F(" °"));
  Serial.print(F("Load      : ")); Serial.print(data.loadKg, 1);    Serial.println(F(" kg"));
  Serial.print(F("Temp      : ")); Serial.print(data.temperature, 1); Serial.println(F(" °C"));
  Serial.print(F("Humidity  : ")); Serial.print(data.humidity, 1);  Serial.println(F(" %"));
  Serial.print(F("Water Lvl : ")); Serial.println(data.waterLevel);
  Serial.print(F("Gas Level : ")); Serial.println(data.gasLevel);
  Serial.print(F("ALERT     : ")); Serial.println(alertToString(data.alert));
  Serial.println(F("─────────────────────────────"));
}
String alertToString(AlertLevel level) {
  switch (level) {
    case LEVEL_NORMAL:  return "NORMAL";
    case LEVEL_WARNING: return "WARNING";
    case LEVEL_DANGER:  return "DANGER!";
    default:            return "UNKNOWN";
  }
}
