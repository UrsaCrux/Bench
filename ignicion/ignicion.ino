#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

const int IGNITION_PIN = 4;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_BT_Classic"); // Bluetooth device name
  Serial.println("Bluetooth device is ready to pair. Waiting 10 seconds for connection...");

  pinMode(IGNITION_PIN, OUTPUT);

  digitalWrite(IGNITION_PIN, LOW);
  Serial.println("Ignition signal: OFF");
}

void loop() {
  if (SerialBT.hasClient()){
    if (SerialBT.available()){
      char sent_info = SerialBT.read(); // Send data from Bluetooth to serial monitor
      if (sent_info == '1'){
        digitalWrite(IGNITION_PIN, HIGH);
        Serial.println("Ignition signal: ON");
      }
      if (sent_info == '0'){
        digitalWrite(IGNITION_PIN, LOW);
        Serial.println("Ignition signal: OFF");
      }
    }
  } else {
    Serial.println("Esperando conexión...");
  }
  delay(1000);
}