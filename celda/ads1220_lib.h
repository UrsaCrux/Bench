// ads1220_lib.h — Funciones base ADS1220 vía SPI nativa (ESP32)
// Cableado: SCLK=18, MISO=19, MOSI=23, CS=21, DRDY=22
// CLK del ADS1220 -> GND (oscilador interno)
// REFP0/REFN0 sin conectar (se usa AVDD como referencia, ratiométrico)

#ifndef ADS1220_LIB_H
#define ADS1220_LIB_H

#include <SPI.h>

#define PIN_CS    21
#define PIN_DRDY  22

// Comandos del ADS1220
#define CMD_RESET 0x06
#define CMD_START 0x08
#define CMD_PWRDN 0x02
#define CMD_RDATA 0x10
#define CMD_RREG  0x20   // 0010 rrnn
#define CMD_WREG  0x40   // 0100 rrnn

// SPI MODE1 (CPOL=0, CPHA=1) es el que exige el ADS1220
static SPISettings adsSPI(1000000, MSBFIRST, SPI_MODE1);

inline void adsCmd(uint8_t c) {
  SPI.beginTransaction(adsSPI);
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(c);
  digitalWrite(PIN_CS, HIGH);
  SPI.endTransaction();
}

inline void adsWriteReg(uint8_t reg, uint8_t val) {
  SPI.beginTransaction(adsSPI);
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(CMD_WREG | (reg << 2));  // 1 byte
  SPI.transfer(val);
  digitalWrite(PIN_CS, HIGH);
  SPI.endTransaction();
}

inline uint8_t adsReadReg(uint8_t reg) {
  SPI.beginTransaction(adsSPI);
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(CMD_RREG | (reg << 2));
  uint8_t v = SPI.transfer(0xFF);
  digitalWrite(PIN_CS, HIGH);
  SPI.endTransaction();
  return v;
}

// Lee 24 bits en modo continuo y los extiende con signo a 32 bits
inline int32_t adsReadData() {
  SPI.beginTransaction(adsSPI);
  digitalWrite(PIN_CS, LOW);
  uint8_t b2 = SPI.transfer(0xFF);
  uint8_t b1 = SPI.transfer(0xFF);
  uint8_t b0 = SPI.transfer(0xFF);
  digitalWrite(PIN_CS, HIGH);
  SPI.endTransaction();

  int32_t v = ((int32_t)b2 << 16) | ((int32_t)b1 << 8) | b0;
  if (v & 0x800000) v |= 0xFF000000;   // signo 24 -> 32 bits
  return v;
}

inline void adsBegin() {
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);
  pinMode(PIN_DRDY, INPUT);
  SPI.begin(18, 19, 23, PIN_CS);   // SCLK, MISO, MOSI, SS
  delay(10);
  adsCmd(CMD_RESET);
  delay(10);                        // datasheet pide >50us tras RESET
}

// Espera a que DRDY baje. Devuelve false si se agota el tiempo.
inline bool adsWaitDRDY(uint32_t timeout_ms) {
  uint32_t t0 = millis();
  while (digitalRead(PIN_DRDY) == HIGH) {
    if (millis() - t0 > timeout_ms) return false;
  }
  return true;
}

#endif
