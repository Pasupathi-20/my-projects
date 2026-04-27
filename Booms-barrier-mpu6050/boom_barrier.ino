#include <Wire.h>
#include <MPU6050.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal_I2C.h>
#define SDA_PIN         D2   
#define SCL_PIN         D1   
#define LED_CLOSED_PIN  D5   
#define LED_MOVING_PIN  D6   
#define LED_OPEN_PIN    D7   
#define BUZZER_PIN      D8   
#define CLOSED_THRESHOLD   10.0   
#define OPEN_THRESHOLD     75.0   
#define MOVEMENT_HYSTERESIS 2.0  
#define ALPHA              0.96
#define SENSOR_READ_INTERVAL    50     
#define CLOUD_UPLOAD_INTERVAL   15000  
#define DISPLAY_UPDATE_INTERVAL 500    
#define STATE_CONFIRM_COUNT     5    

const char* WIFI_SSID   = "pasupathi";
const char* WIFI_PASS   = "871906";
const char* TS_API_KEY  = "**************";
const char* TS_HOST     = "api.thingspeak.com";
enum BarrierState {
  STATE_CLOSED,
  STATE_OPENING,
  STATE_OPEN,
  STATE_CLOSING,
  STATE_UNKNOWN
};
struct BarrierData {
  float         pitch;          
  float         pitchRate;     
  float         roll;          
  BarrierState  state;          
  BarrierState  prevState;      
  unsigned long lastStateChange; 
  unsigned long openCount;     
};
MPU6050            mpu;
LiquidCrystal_I2C  lcd(0x27, 16, 2);
ESP8266WebServer   webServer(80);
WiFiClient         wifiClient;
BarrierData       barrierData;
float             filteredPitch    = 0.0;
float             filteredRoll     = 0.0;
unsigned long     lastSensorRead   = 0;
unsigned long     lastCloudUpload  = 0;
unsigned long     lastDisplayUpdate= 0;
unsigned long     lastLoopTime     = 0;
int               stateConfirmCount= 0;
BarrierState      pendingState     = STATE_UNKNOWN;

// Gyro calibration offsets (measured at startup)
float gyroXoffset = 0, gyroYoffset = 0, gyroZoffset = 0;
void     calibrateGyro();
void     readMPU6050(BarrierData &data, float dt);
BarrierState detectState(float pitch, float pitchRate, BarrierState current);
void     updateStateWithConfirmation(BarrierState newState, BarrierData &data);
void     updateLEDs(BarrierState state);
void     updateLCD(BarrierData &data);
void     uploadToThingSpeak(BarrierData &data);
void     setupWebServer();
void     handleWebRoot();
void     serialPrintData(BarrierData &data);
String   stateToString(BarrierState s);
String   stateToEmoji(BarrierState s);
void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.println(F("================================================="));
  Serial.println(F("  Boom Barrier Position Detection  v1.0.0       "));
  Serial.println(F("  NodeMCU ESP8266 + MPU6050                     "));
  Serial.println(F("================================================="));

  pinMode(LED_CLOSED_PIN, OUTPUT);
  pinMode(LED_MOVING_PIN, OUTPUT);
  pinMode(LED_OPEN_PIN,   OUTPUT);
  pinMode(BUZZER_PIN,     OUTPUT);
  digitalWrite(LED_CLOSED_PIN, HIGH); delay(200);
  digitalWrite(LED_MOVING_PIN, HIGH); delay(200);
  digitalWrite(LED_OPEN_PIN,   HIGH); delay(200);
  digitalWrite(LED_CLOSED_PIN, LOW);
  digitalWrite(LED_MOVING_PIN, LOW);
  digitalWrite(LED_OPEN_PIN,   LOW);
  tone(BUZZER_PIN, 1000, 150); delay(200);
  noTone(BUZZER_PIN);

  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print(F("Boom Barrier    "));
  lcd.setCursor(0, 1); lcd.print(F("Initializing... "));
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println(F("MPU6050: Connected OK"));
  } else {
    Serial.println(F("MPU6050: CONNECTION FAILED -- check wiring"));
    lcd.setCursor(0, 1); lcd.print(F("MPU6050 ERROR!  "));
    while (true) { delay(1000); } // halt
  }
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);   
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);  
  mpu.setDLPFMode(MPU6050_DLPF_BW_20);              
  Serial.println(F("Calibrating gyro — keep sensor STILL for 3 seconds..."));
  lcd.setCursor(0, 1); lcd.print(F("Calibrating...  "));
  calibrateGyro();
  Serial.println(F("Calibration complete."));
  barrierData.state          = STATE_UNKNOWN;
  barrierData.prevState      = STATE_UNKNOWN;
  barrierData.lastStateChange = millis();
  barrierData.openCount      = 0;
  filteredPitch              = 0.0;
  filteredRoll               = 0.0;

  Serial.print(F("Connecting to WiFi: "));
  Serial.println(WIFI_SSID);
  lcd.setCursor(0, 1); lcd.print(F("WiFi Connecting "));
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print(F("WiFi Connected! IP: "));
    Serial.println(WiFi.localIP());
    lcd.setCursor(0, 1); lcd.print(F("WiFi Connected! "));
  } else {
    Serial.println(F("\nWiFi FAILED -- offline mode"));
    lcd.setCursor(0, 1); lcd.print(F("WiFi Failed     "));
  }

  // Setup web server
  setupWebServer();
  webServer.begin();
  Serial.println(F("Web server started."));

  lastLoopTime = millis();
  lcd.clear();
  Serial.println(F("System ready. Monitoring barrier..."));
}
void loop() {
  webServer.handleClient();  // Handle web requests (non-blocking)

  unsigned long now = millis();
  float dt = (now - lastLoopTime) / 1000.0;  // seconds since last loop
  lastLoopTime = now;

  if (now - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = now;

    readMPU6050(barrierData, dt);

    
    BarrierState detected = detectState(
      barrierData.pitch,
      barrierData.pitchRate,
      barrierData.state
    );
   updateStateWithConfirmation(detected, barrierData);
    updateLEDs(barrierData.state);
    serialPrintData(barrierData);
  }
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = now;
    updateLCD(barrierData);
  }

  if (WiFi.status() == WL_CONNECTED &&
      now - lastCloudUpload >= CLOUD_UPLOAD_INTERVAL) {
    lastCloudUpload = now;
    uploadToThingSpeak(barrierData);
  }
}

void calibrateGyro() {
  int16_t gx, gy, gz;
  long sumX = 0, sumY = 0, sumZ = 0;
  const int samples = 500;

  for (int i = 0; i < samples; i++) {
    mpu.getRotation(&gx, &gy, &gz);
    sumX += gx;
    sumY += gy;
    sumZ += gz;
    delay(3);
  }

  gyroXoffset = sumX / (float)samples;
  gyroYoffset = sumY / (float)samples;
  gyroZoffset = sumZ / (float)samples;

  Serial.print(F("Gyro offsets -> X:")); Serial.print(gyroXoffset);
  Serial.print(F(" Y:"));                Serial.print(gyroYoffset);
  Serial.print(F(" Z:"));                Serial.println(gyroZoffset);
}

void readMPU6050(BarrierData &data, float dt) {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float axG = ax / 16384.0;
  float ayG = ay / 16384.0;
  float azG = az / 16384.0;

  float gyroX = (gx - gyroXoffset) / 131.0;  // deg/s
  float gyroY = (gy - gyroYoffset) / 131.0;

  float accelPitch = atan2(ayG, sqrt(axG * axG + azG * azG)) * 180.0 / PI;
  float accelRoll  = atan2(axG, sqrt(ayG * ayG + azG * azG)) * 180.0 / PI;

  filteredPitch = ALPHA * (filteredPitch + gyroY * dt) + (1.0 - ALPHA) * accelPitch;
  filteredRoll  = ALPHA * (filteredRoll  + gyroX * dt) + (1.0 - ALPHA) * accelRoll;

  data.pitch     = filteredPitch;
  data.roll      = filteredRoll;
  data.pitchRate = gyroY;  // degrees per second
}
BarrierState detectState(float pitch, float pitchRate, BarrierState current) {
  if (pitch <= CLOSED_THRESHOLD) {
    return STATE_CLOSED;
  }
  else if (pitch >= OPEN_THRESHOLD) {
    return STATE_OPEN;
  }
  else {
        if (pitchRate > MOVEMENT_HYSTERESIS) {
      return STATE_OPENING;
    } else if (pitchRate < -MOVEMENT_HYSTERESIS) {
      return STATE_CLOSING;
    } else {
           return current;
    }
  }
}

void updateStateWithConfirmation(BarrierState newState, BarrierData &data) {
  if (newState == pendingState) {
    stateConfirmCount++;
  } else {
    pendingState     = newState;
    stateConfirmCount = 1;
  }

  if (stateConfirmCount >= STATE_CONFIRM_COUNT && newState != data.state) {
    data.prevState      = data.state;
    data.state          = newState;
    data.lastStateChange = millis();
    stateConfirmCount   = 0;

  
    if (newState == STATE_OPEN) {
      data.openCount++;
    }
    tone(BUZZER_PIN, 1200, 80);

    Serial.print(F(">>> STATE CHANGE: "));
    Serial.print(stateToString(data.prevState));
    Serial.print(F(" -> "));
    Serial.println(stateToString(data.state));
  }
}

void updateLEDs(BarrierState state) {
  digitalWrite(LED_CLOSED_PIN, state == STATE_CLOSED  ? HIGH : LOW);
  digitalWrite(LED_MOVING_PIN, (state == STATE_OPENING || state == STATE_CLOSING) ? HIGH : LOW);
  digitalWrite(LED_OPEN_PIN,   state == STATE_OPEN    ? HIGH : LOW);
}

void updateLCD(BarrierData &data) {
  lcd.setCursor(0, 0);
  lcd.print(F("Pitch:          "));
  lcd.setCursor(7, 0);
  lcd.print(data.pitch, 1);
  lcd.print(F(" deg"));

  lcd.setCursor(0, 1);
  lcd.print(F("State:          "));
  lcd.setCursor(7, 1);
  lcd.print(stateToString(data.state));
  lcd.print(F("      "));
}

void uploadToThingSpeak(BarrierData &data) {
  Serial.println(F("Uploading to ThingSpeak..."));

  if (wifiClient.connect(TS_HOST, 80)) {
    String payload = "field1=" + String(data.pitch, 2)
      + "&field2=" + String(data.pitchRate, 2)
      + "&field3=" + String(data.roll, 2)
      + "&field4=" + String((int)data.state)
      + "&field5=" + String(data.openCount);

    wifiClient.print("POST /update HTTP/1.1\r\n");
    wifiClient.print("Host: "); wifiClient.print(TS_HOST); wifiClient.print("\r\n");
    wifiClient.print("Connection: close\r\n");
    wifiClient.print("X-THINGSPEAKAPIKEY: "); wifiClient.print(TS_API_KEY); wifiClient.print("\r\n");
    wifiClient.print("Content-Type: application/x-www-form-urlencoded\r\n");
    wifiClient.print("Content-Length: "); wifiClient.print(payload.length()); wifiClient.print("\r\n\r\n");
    wifiClient.print(payload);

    delay(500);
    wifiClient.stop();
    Serial.println(F("ThingSpeak upload done."));
  } else {
    Serial.println(F("ThingSpeak connect failed."));
  }
}

void setupWebServer() {
  webServer.on("/", handleWebRoot);
  webServer.on("/data", []() {
    
    String json = "{";
    json += "\"pitch\":"    + String(barrierData.pitch, 2) + ",";
    json += "\"pitchRate\":" + String(barrierData.pitchRate, 2) + ",";
    json += "\"roll\":"     + String(barrierData.roll, 2) + ",";
    json += "\"state\":\""  + stateToString(barrierData.state) + "\",";
    json += "\"openCount\":" + String(barrierData.openCount);
    json += "}";
    webServer.send(200, "application/json", json);
  });
}

void handleWebRoot() {
  String stateColor;
  switch (barrierData.state) {
    case STATE_CLOSED:  stateColor = "#e74c3c"; break;
    case STATE_OPEN:    stateColor = "#2ecc71"; break;
    case STATE_OPENING:
    case STATE_CLOSING: stateColor = "#f39c12"; break;
    default:            stateColor = "#95a5a6"; break;
  }

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<title>Boom Barrier Monitor</title>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;background:#1a1a2e;color:#eee;margin:0;padding:20px;text-align:center;}";
  html += "h1{color:#e94560;font-size:1.8em;margin-bottom:5px;}";
  html += ".subtitle{color:#aaa;font-size:0.9em;margin-bottom:30px;}";
  html += ".card{background:#16213e;border-radius:12px;padding:20px;margin:10px auto;max-width:420px;box-shadow:0 4px 15px rgba(0,0,0,0.4);}";
  html += ".state-badge{display:inline-block;padding:12px 30px;border-radius:30px;font-size:1.4em;font-weight:bold;color:#fff;background:" + stateColor + ";margin:10px 0;}";
  html += ".metric{display:flex;justify-content:space-between;padding:8px 0;border-bottom:1px solid #0f3460;}";
  html += ".metric:last-child{border-bottom:none;}";
  html += ".metric-label{color:#aaa;}";
  html += ".metric-value{color:#e94560;font-weight:bold;font-size:1.1em;}";
  html += ".gauge-bar{background:#0f3460;border-radius:10px;height:18px;margin:5px 0;}";
  html += ".gauge-fill{height:18px;border-radius:10px;background:linear-gradient(to right,#e94560,#f39c12,#2ecc71);transition:width 0.3s;}";
  html += "footer{margin-top:20px;color:#555;font-size:0.8em;}";
  html += "</style></head><body>";

  html += "<h1>Boom Barrier Monitor</h1>";
  html += "<p class='subtitle'>NodeMCU ESP8266 + MPU6050 | Pasupathi</p>";

  html += "<div class='card'>";
  html += "<p style='color:#aaa;margin:0 0 5px;'>Current Position</p>";
  html += "<div class='state-badge'>" + stateToString(barrierData.state) + "</div>";
  html += "</div>";

  float pct = constrain(barrierData.pitch / 90.0 * 100.0, 0, 100);
  html += "<div class='card'>";
  html += "<p style='color:#aaa;margin:0 0 5px;'>Pitch Angle</p>";
  html += "<div style='font-size:2em;font-weight:bold;color:#e94560;'>" + String(barrierData.pitch, 1) + " &deg;</div>";
  html += "<div class='gauge-bar'><div class='gauge-fill' style='width:" + String(pct, 0) + "%'></div></div>";
  html += "<p style='color:#555;font-size:0.8em;margin:2px 0;'>0° (Closed) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 90° (Open)</p>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='metric'><span class='metric-label'>Pitch Rate</span><span class='metric-value'>" + String(barrierData.pitchRate, 1) + " &deg;/s</span></div>";
  html += "<div class='metric'><span class='metric-label'>Roll Angle</span><span class='metric-value'>" + String(barrierData.roll, 1) + " &deg;</span></div>";
  html += "<div class='metric'><span class='metric-label'>Total Open Cycles</span><span class='metric-value'>" + String(barrierData.openCount) + "</span></div>";
  html += "<div class='metric'><span class='metric-label'>IP Address</span><span class='metric-value'>" + WiFi.localIP().toString() + "</span></div>";
  html += "</div>";

  html += "<footer>Auto-refreshes every 2s &nbsp;|&nbsp; <a href='/data' style='color:#e94560;'>JSON API</a></footer>";
  html += "</body></html>";

  webServer.send(200, "text/html", html);
}
void serialPrintData(BarrierData &data) {
  Serial.print(F("Pitch: ")); Serial.print(data.pitch, 1);
  Serial.print(F("°  Rate: ")); Serial.print(data.pitchRate, 1);
  Serial.print(F("°/s  Roll: ")); Serial.print(data.roll, 1);
  Serial.print(F("°  State: ")); Serial.print(stateToString(data.state));
  Serial.print(F("  Opens: ")); Serial.println(data.openCount);
}

String stateToString(BarrierState s) {
  switch (s) {
    case STATE_CLOSED:  return "CLOSED";
    case STATE_OPENING: return "OPENING";
    case STATE_OPEN:    return "OPEN";
    case STATE_CLOSING: return "CLOSING";
    default:            return "UNKNOWN";
  }
}

String stateToEmoji(BarrierState s) {
  switch (s) {
    case STATE_CLOSED:  return "BARRIER DOWN";
    case STATE_OPENING: return "RISING...";
    case STATE_OPEN:    return "BARRIER UP";
    case STATE_CLOSING: return "LOWERING...";
    default:            return "DETECTING...";
  }
}
