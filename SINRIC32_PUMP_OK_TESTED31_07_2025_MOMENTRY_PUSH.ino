/*********** Sinric Pro Momentary Relay Control - 3 Channels ***********/
#include <WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>

// WiFi credentials
#define WIFI_SSID     "ElectroConnect"
#define WIFI_PASS     "vikash9296"

// Sinric Pro credentials
#define APP_KEY       "c7303938-59fa-4ad2-a3de-e62d9bae9cec"
#define APP_SECRET    "c41c8855-b93b-496c-bf27-43f0f72c2dc8-4e754444-82cd-4acb-bb2c-c9cfe22b9b13"

// Device IDs
#define DEVICE_ID_1   "6889cacbedeca866fe96e2ee"  // Relay 1
#define DEVICE_ID_2   "6889caffddd2551252ba8a70"  // Relay 2
#define DEVICE_ID_3   "6889cb32edeca866fe96e336"  // Relay 3

// Relay pins (active LOW)
#define RELAY1_PIN    19
#define RELAY2_PIN    21
#define RELAY3_PIN    22

// Push button pins
#define BUTTON1_PIN   13
#define BUTTON2_PIN   12
#define BUTTON3_PIN   14

// WiFi indicator LED
#define WIFI_LED_PIN  2

// Momentary ON duration (milliseconds)
#define PULSE_TIME    2000

unsigned long lastReconnectAttempt = 0;

void setupRelay(int pin) {
  digitalWrite(pin, HIGH);  // OFF for active-low relay
  pinMode(pin, OUTPUT);
}

void pulseRelay(int relayPin) {
  digitalWrite(relayPin, LOW);    // ON
  delay(PULSE_TIME);
  digitalWrite(relayPin, HIGH);   // OFF
}

void setupButton(int buttonPin, void (*callback)()) {
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), callback, FALLING);
}

IRAM_ATTR void handleButton1() { pulseRelay(RELAY1_PIN); }
IRAM_ATTR void handleButton2() { pulseRelay(RELAY2_PIN); }
IRAM_ATTR void handleButton3() { pulseRelay(RELAY3_PIN); }

bool onPowerState1(const String &deviceId, bool &state) {
  if (deviceId == DEVICE_ID_1) pulseRelay(RELAY1_PIN);
  else if (deviceId == DEVICE_ID_2) pulseRelay(RELAY2_PIN);
  else if (deviceId == DEVICE_ID_3) pulseRelay(RELAY3_PIN);
  return true;
}

void setupSinricPro() {
  SinricProSwitch &mySwitch1 = SinricPro[DEVICE_ID_1];
  SinricProSwitch &mySwitch2 = SinricPro[DEVICE_ID_2];
  SinricProSwitch &mySwitch3 = SinricPro[DEVICE_ID_3];

  mySwitch1.onPowerState(onPowerState1);
  mySwitch2.onPowerState(onPowerState1);
  mySwitch3.onPowerState(onPowerState1);

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}

void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(100);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(WIFI_LED_PIN, OUTPUT);

  setupRelay(RELAY1_PIN);
  setupRelay(RELAY2_PIN);
  setupRelay(RELAY3_PIN);

  setupButton(BUTTON1_PIN, handleButton1);
  setupButton(BUTTON2_PIN, handleButton2);
  setupButton(BUTTON3_PIN, handleButton3);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  setupSinricPro();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(WIFI_LED_PIN, HIGH);
    SinricPro.handle();
  } else {
    digitalWrite(WIFI_LED_PIN, LOW);
    if (millis() - lastReconnectAttempt > 10000) {
      lastReconnectAttempt = millis();
      reconnectWiFi();
      if (WiFi.status() == WL_CONNECTED) {
        SinricPro.begin(APP_KEY, APP_SECRET);
      }
    }
  }
}