#include <Arduino.h>
#include <string>

#include "defs.h"
#include "ads_driver.h"

#define REG0_CELDA 0x0E
#define REG1_TEMP  0x06
#define REG1_CELDA 0x14        // 20 SPS  (modos 2 y 3, con filtro de red) --D
#define REG1_RAPIDO 0xC4       // 1000 SPS (modo 4, sin filtro de red) --D
#define REG2_CFG   0xD0
#define REG3_CFG   0x00

void configurar(uint8_t reg1) {
  adsWriteReg(0x00, REG0_CELDA);
  adsWriteReg(0x01, reg1);
  adsWriteReg(0x02, REG2_CFG);
  adsWriteReg(0x03, REG3_CFG);
  delay(10);
  adsCmd(CMD_START);            // arranca conversion continua --D
}

bool verificarRegistros(uint8_t reg1) {
  uint8_t esperado[4] = {REG0_CELDA, reg1, REG2_CFG, REG3_CFG};
  bool ok = true;
  Serial.println(F("Registro  escrito  leido"));
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t leido = adsReadReg(i);
    Serial.printf("  0x0%d      0x%02X    0x%02X   %s\n",
                  i, esperado[i], leido,
                  (leido == esperado[i]) ? "OK" : "<-- NO COINCIDE");
    if (leido != esperado[i]) ok = false;
  }
  return ok;
}

void wait_mode_confirmation() {
    while(true) {
        Serial.println("init_nn");
        if (Serial.available() > 0) {
            char command = Serial.read();
            if (command == 192){
                break;
            }
        }

        delay(50);
    }

    bool comm = false;
    while (true){
        if (Serial.available() > 0) {
            String d = Serial.readString();
            int index_d = d.indexOf('_');

            String nn = d.substring(0, index_d);
            if (nn == "mode"){
                String number = d.substring(index_d+1);
                Serial.printf("remote_mode%d \n", number.toInt());
                modo_actual = static_cast<modo_n>(number.toInt());
                break;
            }
                      
        }
        if (!comm){
            Serial.println("init_oo");
        }
        delay(50);
        
    }
}

void test_spi() {
    configurar(REG1_TEMP);

    if (verificarRegistros(REG1_TEMP)) {
        Serial.println("perfect_spi");
    } else {
        Serial.println("error_spi");
    }

    while (!is_stop()){
        Serial.print("temp_");
        Serial.println(get_temp());
    }
    /*Serial.println(F("--- Paso 2: sensor de temperatura interno ---"));
    Serial.println(F("Debe rondar la temperatura ambiente (~20-30 C).\n"));*/
}

bool is_stop() {
    if (Serial.available() > 0){
        String d = Serial.readString();
            int index_d = d.indexOf('_');

            String nn = d.substring(0, index_d);
            if (nn == "stop"){
                String number = d.substring(index_d+1);
                if (number == "please")
                    return true;
            }
    }
    return false;
}

float get_temp() {
    float temp_c = 0.0f;
    if (adsWaitDRDY(1000)) {
        int32_t raw = adsReadData();
        temp_c = (raw >> 10) * 0.03125f;
    } else {
        Serial.println("error_timeout");
    }
    return temp_c;
}

void raw_read() {
    Serial.println("perfect_raw");
    configurar(REG1_CELDA);
    
    /*Serial.println(F("Sin carga debe quedar estable. Presiona la celda: debe moverse."));
    Serial.println(F("Si baja al presionar, invierte verde y blanco (AIN0/AIN1).\n"));*/

    while (!is_stop()) {
        if (adsWaitDRDY(1000)) {
            int32_t raw = adsReadData();
            //D. Voltaje de entrada equivalente: FS = +-VREF/ganancia = +-3.3/128
            //float mV = (raw / 8388608.0f) * (3.3f / 128.0f) * 1000.0f;
            //Serial.printf("conteo=%9ld   entrada= %+8.4f mV\n", raw, mV);
            Serial.printf("output_%d\n", raw);
        } else {
            Serial.println("error_timeout");
        }
        delay(20);
    }
}

void calibration() {
    configurar(REG1_CELDA);
    

    int64_t suma = 0;
    const int N = 40;                       // ~2 s a 20 SPS
    for (int i = 0; i < N; i++) {
        adsWaitDRDY(500);
        suma += adsReadData();
    }
    cero = suma / N;
    //Serial.printf("Cero (tara) = %ld\n\n", cero);
    Serial.print("tara_");
    Serial.println(cero);

    if (factor_kg == 0.0f) {
        //Falta factor escala
        Serial.println("error_no-calibration");
    }

    Serial.println("perfect_calibration");
}

void ensayo() {
    configurar(REG1_RAPIDO);                 // 1000 SPS

    if (factor_kg == 0.0f) {
        //Serial.println(F("AVISO: factor_kg = 0. Calibra primero con el MODO 3."));
        //Serial.println(F("El ensayo igual corre, pero solo se guardan conteos.\n"));
        Serial.println("error_no-calibration");
    }
    /*Serial.printf("Capacidad: %d muestras = %.1f s a 1000 SPS (%d KB de RAM)\n\n",
        N_MUESTRAS, N_MUESTRAS / 1000.0, (N_MUESTRAS * 4) / 1024);*/

    //TARA (comentado porque si):
    /*Serial.println(F("Deja la celda SIN CARGA. Tarando en 3 segundos..."));
    delay(3000);
    int64_t suma = 0;
    const int N_TARA = 1000;                 // ~1 s a 1000 SPS
    for (int i = 0; i < N_TARA; i++) {
        adsWaitDRDY(500);
        suma += adsReadData();
    }
    cero = suma / N_TARA;
    Serial.printf("Cero (tara) = %ld\n\n", cero);*/

    while (!Serial.available()) { delay(10); }
    while (Serial.available()) Serial.read();

    // --- Captura (nada lento debe interponerse aqui) ---
    uint32_t perdidas = 0;
    t_inicio = micros();
    for (uint32_t i = 0; i < N_MUESTRAS; i++) {
        uint32_t t0 = micros();
        while (digitalRead(PIN_DRDY) == HIGH) {
            if (micros() - t0 > 50000) { perdidas++; break; }   // 50 ms de guardia
        }
        buffer[i] = adsReadData();
        yield();                                                // deja respirar al RTOS
    }
    
    t_fin = micros();

    float dur_s = (t_fin - t_inicio) / 1e6f;
    float sps   = N_MUESTRAS / dur_s;
    Serial.printf("duracion-real_%.3f\n", dur_s);
    Serial.printf("tasa-efectiva_%.1f\n", sps);
    Serial.printf("muestras-tarde_%lu\n", perdidas);
    Serial.printf("cero-usado_%ld\n", cero);
    Serial.printf("factor_kg_%.9f\n", factor_kg);

  /*float dt_ms = (dur_s * 1000.0f) / N_MUESTRAS;
  for (uint32_t i = 0; i < N_MUESTRAS; i++) {
    int32_t neto = buffer[i] - cero;
    float   kg   = neto * factor_kg;
    Serial.printf("%.3f,%ld,%ld,%.4f,%.3f\n",
                  i * dt_ms, buffer[i], neto, kg, kg * 9.81f);
  }*/
}

void set_all()  {
    wait_mode_confirmation();

    delay(10);
    adsCmd(CMD_RESET);
    delay(10); 

    switch(modo_actual){
        case modo_n::TEST_SPI:
            test_spi();
            break;
        case modo_n::RAW_READ:
            raw_read();
            break;
        case modo_n::CALIBRATION:
            calibration();
            break;
        case modo_n::ENSAYO:
            ensayo();
            break;
    }
}

void setup() {
    Serial.begin(115200);

    adsBegin(); //Supongo que esto empieza la conexion con el sensor
    while (true) {
        set_all();
        delay(30);
    }
}


void loop() {
/*
#elif MODO == 3
  if (adsWaitDRDY(1000)) {
    int32_t raw  = adsReadData();
    int32_t neto = raw - cero;
    if (factor_kg == 0.0f) {
      Serial.printf("neto=%9ld   (falta calibrar)\n", neto);
    } else {
      float kg = neto * factor_kg;
      Serial.printf("neto=%9ld   %8.3f kg   %8.2f N\n", neto, kg, kg * 9.81f);
    }
  }
  delay(200);

#elif MODO == 4
  // El ensayo completo ocurre una sola vez en setup(). Aqui no se hace nada.
#endif*/
}
