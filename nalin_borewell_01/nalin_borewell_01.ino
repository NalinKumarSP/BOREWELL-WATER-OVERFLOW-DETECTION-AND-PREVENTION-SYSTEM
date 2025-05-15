#include <SoftwareSerial.h>
#include <HX710B.h>

#define WATER_SENSOR_1 A0  // First Water Level Sensor
#define WATER_SENSOR_2 A1  // Second Water Level Sensor
#define WATER_SENSOR_3 A2  // Third Water Level Sensor
#define SWITCH_PIN A3      // Switch Pin

#define GREEN_LED 7        // Green LED for first water level
#define RED_LED 8          // Red LED for overflow condition
#define RELAY_PIN 9        // Relay Pin

#define HX710B_DOUT 2      // Data Output Pin for HX710B
#define HX710B_SCK  3      // Clock Pin for HX710B

#define GSM_TX 10          // GSM Module TX to Arduino Pin 10
#define GSM_RX 11          // GSM Module RX to Arduino Pin 11

HX710B pressureSensor(HX710B_DOUT, HX710B_SCK);
SoftwareSerial gsm(GSM_TX, GSM_RX);

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

    // Pressure Sensor Reading Test
    uint32_t pressureData;
    uint8_t status = pressureSensor.read(&pressureData);
    if (status == 0) {
        Serial.print("Pressure Sensor Reading: ");
        Serial.println(pressureData);
    } else {
        Serial.println("Error reading pressure data");
    }

    Serial.println("System Initialized...");
}

void sendSMS(const char* message) {
    Serial.print("Sending SMS: ");
    Serial.println(message);
    gsm.println("AT+CMGF=1");  // Set SMS mode to text
    delay(1000);
    gsm.println("AT+CMGS=\"+918883711939\"");  // Replace with your number
    delay(1000);
    gsm.println(message); 
    delay(1000);
    gsm.write(26);  // End SMS with Ctrl+Z
    delay(5000);
    Serial.println("SMS Sent!");
}

void loop() {
    int waterLevel1 = analogRead(WATER_SENSOR_1);
    int waterLevel2 = analogRead(WATER_SENSOR_2);
    int waterLevel3 = analogRead(WATER_SENSOR_3);
    int switchState = digitalRead(SWITCH_PIN);

    uint32_t pressureData;
    uint8_t status = pressureSensor.read(&pressureData);
    if (status == 0) {
        Serial.print("Pressure Sensor Reading: ");
        Serial.println(pressureData);
    } else {
        Serial.println("Error reading pressure data");
    }

    // First Sensor - Green LED
    digitalWrite(GREEN_LED, waterLevel1 > 500 ? HIGH : LOW);

    // Second Sensor - Send High Water Level SMS
    if (waterLevel2 > 500 && !highWaterSMS) {
        sendSMS("Borewell has high water level.");
        highWaterSMS = true;
    } else if (waterLevel2 <= 500) {
        highWaterSMS = false;
    }

    // Third Sensor - Send Overflow Condition SMS
    if (waterLevel3 > 500 && !overflowSMS) {
        sendSMS("Borewell is under overflow condition.");
        digitalWrite(RED_LED, HIGH);
        overflowSMS = true;
    } else if (waterLevel3 <= 500) {
        digitalWrite(RED_LED, LOW);
        overflowSMS = false;
    }

    // Switch Control for Relay
    if (switchState == HIGH) {
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("Relay Activated! Drain valve is open.");
        if (!drainValveSMS) {
            sendSMS("Drain valve is open in the borewell.");
            drainValveSMS = true;
        }
    } else {
        digitalWrite(RELAY_PIN, LOW);
        drainValveSMS = false;
    }

    delay(500);  // Small delay for stability
}
