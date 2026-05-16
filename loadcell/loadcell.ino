#include <Arduino.h>
#include "soc/rtc.h"
#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;
const int NUM_SAMPLES = 80;
const int TEST_PIN = LOADCELL_DOUT_PIN;
const float SCALE = 4.32f;

HX711 scale;

void setup() {
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(SCALE);
  scale.tare();
  delay(5000);
  Serial.println("BEGIN");
}

void loop() {
    long reading = scale.get_units(NUM_SAMPLES);
    Serial.println(reading);
}
