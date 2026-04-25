
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define RAIN_ANALOG_PIN   A0   
#define RAIN_DIGITAL_PIN  2    
#define SERVO_PIN         9    
#define BUZZER_PIN        8   
#define LED_OFF_PIN       3   
#define LED_SLOW_PIN      4   
#define LED_MEDIUM_PIN    5   
#define LED_FAST_PIN      6   
#define THRESHOLD_NO_RAIN    900   
#define THRESHOLD_LIGHT_RAIN 700   
#define THRESHOLD_MOD_RAIN   400   
#define WIPER_LEFT_POS    30
#define WIPER_RIGHT_POS   150
#define WIPER_CENTER_POS  90
#define SPEED_SLOW        20
#define SPEED_MEDIUM      10
#define SPEED_FAST        3

Servo wiperServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);  

enum WiperMode {
  MODE_OFF,
  MODE_SLOW,
  MODE_MEDIUM,
  MODE_FAST
};

WiperMode currentMode    = MODE_OFF;
WiperMode previousMode   = MODE_OFF;
int       servoPos       = WIPER_CENTER_POS;
bool      sweepDirection = true;   
unsigned long lastSweepTime  = 0;
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;

const unsigned long SENSOR_READ_INTERVAL   = 200;   
const unsigned long DISPLAY_UPDATE_INTERVAL = 500;  
WiperMode getRainMode(int sensorValue);
void      updateWiper(WiperMode mode);
void      updateLEDs(WiperMode mode);
void      updateLCD(WiperMode mode, int sensorValue);
void      parkWiper();
String    modeToString(WiperMode mode);
int       getModeDelay(WiperMode mode);

void setup() {
  Serial.begin(9600);
  Serial.println(F("================================="));
  Serial.println(F("  Auto Rain Wiper System v1.0.0  "));
  Serial.println(F("================================="));

  pinMode(RAIN_DIGITAL_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_OFF_PIN, OUTPUT);
  pinMode(LED_SLOW_PIN, OUTPUT);
  pinMode(LED_MEDIUM_PIN, OUTPUT);
  pinMode(LED_FAST_PIN, OUTPUT);

  wiperServo.attach(SERVO_PIN);
  wiperServo.write(WIPER_CENTER_POS);
  servoPos = WIPER_CENTER_POS;

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(F("  Rain Wiper    "));
  lcd.setCursor(0, 1);
  lcd.print(F("  Initializing.."));
  delay(2000);
  lcd.clear();

  tone(BUZZER_PIN, 1000, 200);
  delay(300);
  tone(BUZZER_PIN, 1500, 200);
  delay(300);
  noTone(BUZZER_PIN);

  digitalWrite(LED_OFF_PIN, HIGH);
  digitalWrite(LED_SLOW_PIN, HIGH);
  digitalWrite(LED_MEDIUM_PIN, HIGH);
  digitalWrite(LED_FAST_PIN, HIGH);
  delay(500);
  digitalWrite(LED_OFF_PIN, LOW);
  digitalWrite(LED_SLOW_PIN, LOW);
  digitalWrite(LED_MEDIUM_PIN, LOW);
  digitalWrite(LED_FAST_PIN, LOW);

  Serial.println(F("System Ready."));
}

void loop() {
  unsigned long now = millis();

  if (now - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = now;

    int sensorValue = analogRead(RAIN_ANALOG_PIN);
    currentMode = getRainMode(sensorValue);

    if (currentMode != previousMode) {
      Serial.print(F("Mode changed → "));
      Serial.println(modeToString(currentMode));

      if (currentMode != MODE_OFF) {
        tone(BUZZER_PIN, 800, 100);
      }

      updateLEDs(currentMode);
      previousMode = currentMode;
    }

    if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
      lastDisplayUpdate = now;
      updateLCD(currentMode, sensorValue);
    }
  }

  updateWiper(currentMode);
}

WiperMode getRainMode(int value) {
  if (value >= THRESHOLD_NO_RAIN) {
    return MODE_OFF;
  } else if (value >= THRESHOLD_LIGHT_RAIN) {
    return MODE_SLOW;
  } else if (value >= THRESHOLD_MOD_RAIN) {
    return MODE_MEDIUM;
  } else {
    return MODE_FAST;
  }
}

void updateWiper(WiperMode mode) {
  if (mode == MODE_OFF) {
    parkWiper();
    return;
  }

  int sweepDelay = getModeDelay(mode);
  unsigned long now = millis();

  if (now - lastSweepTime >= (unsigned long)sweepDelay) {
    lastSweepTime = now;

    if (sweepDirection) {
      servoPos++;
      if (servoPos >= WIPER_RIGHT_POS) {
        sweepDirection = false;
      }
    } else {
      servoPos--;
      if (servoPos <= WIPER_LEFT_POS) {
        sweepDirection = true;
      }
    }

    wiperServo.write(servoPos);
  }
}

void parkWiper() {
  if (servoPos != WIPER_CENTER_POS) {
    // Smoothly move to center
    if (servoPos < WIPER_CENTER_POS) {
      servoPos++;
    } else {
      servoPos--;
    }
    wiperServo.write(servoPos);
    delay(10);
  }
}

void updateLEDs(WiperMode mode) {
  digitalWrite(LED_OFF_PIN,    mode == MODE_OFF    ? HIGH : LOW);
  digitalWrite(LED_SLOW_PIN,   mode == MODE_SLOW   ? HIGH : LOW);
  digitalWrite(LED_MEDIUM_PIN, mode == MODE_MEDIUM ? HIGH : LOW);
  digitalWrite(LED_FAST_PIN,   mode == MODE_FAST   ? HIGH : LOW);
}

void updateLCD(WiperMode mode, int sensorValue) {
  lcd.setCursor(0, 0);
  lcd.print(F("Rain:           "));
  lcd.setCursor(6, 0);
  lcd.print(sensorValue);
  lcd.print(F("   "));

  lcd.setCursor(0, 1);
  lcd.print(F("Mode:           "));
  lcd.setCursor(6, 1);
  lcd.print(modeToString(mode));
  lcd.print(F("      "));
}

String modeToString(WiperMode mode) {
  switch (mode) {
    case MODE_OFF:    return "OFF";
    case MODE_SLOW:   return "SLOW";
    case MODE_MEDIUM: return "MEDIUM";
    case MODE_FAST:   return "FAST";
    default:          return "UNKNOWN";
  }
}

int getModeDelay(WiperMode mode) {
  switch (mode) {
    case MODE_SLOW:   return SPEED_SLOW;
    case MODE_MEDIUM: return SPEED_MEDIUM;
    case MODE_FAST:   return SPEED_FAST;
    default:          return SPEED_SLOW;
  }
}
