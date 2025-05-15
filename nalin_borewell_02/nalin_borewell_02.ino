#include <SoftwareSerial.h>
#include <HX710B.h>

// Pin Definitions
#define WATER_SENSOR_1 A0
#define WATER_SENSOR_2 A1
#define WATER_SENSOR_3 A2
#define SWITCH_PIN A3

#define GREEN_LED 7
#define RED_LED 8
#define RELAY_PIN 9

#define HX710B_DOUT 2
#define HX710B_SCK  3

#define GSM_TX 10
#define GSM_RX 11

// Objects
HX710B pressureSensor(HX710B_DOUT, HX710B_SCK);
SoftwareSerial gsm(GSM_TX, GSM_RX);

// Flags to prevent repeated SMS
bool highWaterSMS = false;
bool overflowSMS = false;
bool drainValveSMS = false;

void setup() {
  pinMode(WATER_SENSOR_1, INPUT);
  pinMode(WATER_SENSOR_2, INPUT);
  pinMode(WATER_SENSOR_3, INPUT);
  pinMode(SWITCH_PIN, INPUT);
  
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  Serial.begin(9600);
  gsm.begin(9600);

  Serial.println("System Initialized...");
}

void sendSMS(const char* message) {
  Serial.print("Sending SMS: ");
  Serial.println(message);
  gsm.println("AT+CMGF=1");
  delay(1000);
  gsm.println("AT+CMGS=\"+918883711939\"");
  delay(1000);
  gsm.println(message);
  delay(1000);
  gsm.write(26);
  delay(5000);
  Serial.println("SMS Sent!");
}

void readPressureSensor() {
  uint32_t pressureData;
  uint8_t status = pressureSensor.read(&pressureData);
  if (status == 0) {
    Serial.print("Pressure Sensor Reading: ");
    Serial.println(pressureData);
  } else {
    Serial.println("Error reading pressure data");
  }
}

void loop() {
  int waterLevel1 = analogRead(WATER_SENSOR_1);
  int waterLevel2 = analogRead(WATER_SENSOR_2);
  int waterLevel3 = analogRead(WATER_SENSOR_3);
  int switchState = digitalRead(SWITCH_PIN);

  readPressureSensor();

  // Control Green LED based on Sensor 1
  digitalWrite(GREEN_LED, waterLevel1 > 500 ? HIGH : LOW);

  // High Water Level Notification
  if (waterLevel2 > 500 && !highWaterSMS) {
    sendSMS("Borewell has high water level.");
    highWaterSMS = true;
  } else if (waterLevel2 <= 500) {
    highWaterSMS = false;
  }

  // Overflow Condition Notification
  if (waterLevel3 > 500 && !overflowSMS) {
    sendSMS("Borewell is under overflow condition.");
    digitalWrite(RED_LED, HIGH);
    overflowSMS = true;
  } else if (waterLevel3 <= 500) {
    digitalWrite(RED_LED, LOW);
    overflowSMS = false;
  }

  // Relay Control and Drain Valve Notification
  if (switchState == HIGH && !drainValveSMS) {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Relay Activated! Drain valve is open.");
    sendSMS("Drain valve is open in the borewell.");
    drainValveSMS = true;
  } else if (switchState == LOW) {
    digitalWrite(RELAY_PIN, LOW);
    drainValveSMS = false;
  }

  delay(500); // Stability delay
}
