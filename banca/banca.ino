#include <Arduino.h>
#include "BluetoothSerial.h"
#include "soc/rtc.h"
#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;
const int SAMPLES_PER_SECOND = 80;
const int SAMPLES_AT_TIME = 1;

//Ignition settings
const int IGNITION_PIN = 2;
// Empirically tested scale factor, each unit in the tared program is aproximatedly 4.32g
const float SCALE = 4.32f;


bool has_ignited = false;
bool reading_loadcell = false;

BluetoothSerial SerialBT;
HX711 scale;

void setup() {
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  SerialBT.begin("UrsaCrux_Bench"); // Bluetooth device name
  Serial.println("Bluetooth device is ready to pair. Waiting 10 seconds for connection...");
  pinMode(IGNITION_PIN, OUTPUT);
  digitalWrite(IGNITION_PIN, LOW);
  Serial.println("Ignition signal: OFF");
}

void loop() {
  if (reading_loadcell == true){
    long reading = scale.get_units(SAMPLES_AT_TIME);
    Serial.println(reading);
  } else {
    if (SerialBT.hasClient()){
      if (SerialBT.available()){
        char sent_info = SerialBT.read(); // Send data from Bluetooth to serial monitor
        if (sent_info == '1'){
          digitalWrite(IGNITION_PIN, HIGH);
          Serial.println("Ignition signal: ON");
          has_ignited = true;
        }
        if (sent_info == '0'){
          digitalWrite(IGNITION_PIN, LOW);
          Serial.println("Ignition signal: OFF");
          if (has_ignited == true) {
            reading_loadcell = true;

            scale.tare();
            Serial.println("BEGIN");
          }
        }
      }
    } else {
      Serial.println("Esperando conexión...");
      delay(1000);
    }
  }
}
