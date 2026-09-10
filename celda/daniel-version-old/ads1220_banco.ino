
// ads1220_banco.ino — Banco de empuje: celda LCF-6-V + ADS1220 + ESP32
//
// COMO USAR: cambia el numero de MODO y vuelve a cargar.
//   MODO 1 -> Test de SPI (lee registros + sensor de temperatura interno)
//   MODO 2 -> Lectura cruda de la celda (conteos)
//   MODO 3 -> Calibracion y lectura en kg / Newtons
//   MODO 4 -> Ensayo rapido: captura a 1000 SPS en RAM y vuelca al terminar
//
// Carpeta del sketch debe llamarse "ads1220_banco" y contener tambien
// el archivo ads1220_lib.h
//
// Cableado: SCLK=18, MISO=19, MOSI=23, CS=21, DRDY=22
//           CLK del ADS1220 -> GND    REFP0/REFN0 -> sin conectar

#define MODO 4

#include "ads1220_lib.h"

// ---------------------------------------------------------------
// Configuracion de registros
// ---------------------------------------------------------------
// REG0 = 0x0E : MUX=AIN0/AIN1 diferencial, GAIN=128, PGA activo
// REG1 = 0x04 : 20 SPS, modo normal, conversion continua, temp OFF
// REG1 = 0x06 : igual pero con sensor de temperatura ON (modo 1)
// REG2 = 0xD0 : referencia = AVDD (ratiometrico), rechazo 50/60 Hz
// REG3 = 0x00 : IDACs apagados, DRDY solo por pin

#define REG0_CELDA 0x0E
#define REG1_TEMP  0x06
#define REG1_CELDA 0x04        // 20 SPS  (modos 2 y 3, con filtro de red)
#define REG1_RAPIDO 0xC4       // 1000 SPS (modo 4, sin filtro de red)
#define REG2_CFG   0xD0
#define REG3_CFG   0x00

// Calibracion — se ajustan tras calibrar
int32_t cero      = 0;         // conteo sin carga (tara)
float   factor_kg = 0.000438671f;      // kg por conteo

// ---------------------------------------------------------------
// Buffer de ensayo (solo MODO 4)
// ---------------------------------------------------------------
// 12000 muestras a 1000 SPS = 12 segundos = 48 KB de RAM.
// El ESP32 tiene ~320 KB disponibles, asi que sobra margen.
// Para ensayos mas largos, subir N_MUESTRAS (10000 muestras = 40 KB).
#if MODO == 4
  #define N_MUESTRAS 12000
  int32_t  buffer[N_MUESTRAS];
  uint32_t t_inicio, t_fin;
#endif

// ---------------------------------------------------------------

void configurar(uint8_t reg1) {
  adsWriteReg(0x00, REG0_CELDA);
  adsWriteReg(0x01, reg1);
  adsWriteReg(0x02, REG2_CFG);
  adsWriteReg(0x03, REG3_CFG);
  delay(10);
  adsCmd(CMD_START);            // arranca conversion continua
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

// ---------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.printf("=== Banco de empuje — MODO %d ===\n\n", MODO);

  adsBegin();

#if MODO == 1
  configurar(REG1_TEMP);
  Serial.println(F("--- Paso 1: verificacion de registros ---"));
  if (verificarRegistros(REG1_TEMP)) {
    Serial.println(F("\nSPI OK: el ADS1220 responde y guarda lo que se le escribe.\n"));
  } else {
    Serial.println(F("\nFALLA DE SPI. Revisa MOSI/MISO/SCLK/CS y la alimentacion.\n"));
    Serial.println(F("Si todo lee 0x00 o 0xFF, es cableado o el chip no tiene 3.3V."));
  }
  Serial.println(F("--- Paso 2: sensor de temperatura interno ---"));
  Serial.println(F("Debe rondar la temperatura ambiente (~20-30 C).\n"));

#elif MODO == 2
  configurar(REG1_CELDA);
  Serial.println(F("--- Lectura cruda (conteos) ---"));
  Serial.println(F("Sin carga debe quedar estable. Presiona la celda: debe moverse."));
  Serial.println(F("Si baja al presionar, invierte verde y blanco (AIN0/AIN1).\n"));

#elif MODO == 3
  configurar(REG1_CELDA);
  Serial.println(F("--- Calibracion ---"));
  Serial.println(F("Deja la celda SIN CARGA. Tarando en 3 segundos..."));
  delay(3000);

  int64_t suma = 0;
  const int N = 40;                       // ~2 s a 20 SPS
  for (int i = 0; i < N; i++) {
    adsWaitDRDY(500);
    suma += adsReadData();
  }
  cero = suma / N;
  Serial.printf("Cero (tara) = %ld\n\n", cero);

  if (factor_kg == 0.0f) {
    Serial.println(F("FALTA EL FACTOR DE ESCALA."));
    Serial.println(F("1) Coloca una masa conocida (ej. 10 kg)."));
    Serial.println(F("2) Anota el 'neto' que se imprime abajo."));
    Serial.println(F("3) factor = kg_conocidos / neto"));
    Serial.println(F("4) Escribe ese valor en 'factor_kg' y recarga.\n"));
  }

#elif MODO == 4
  // ---------- ENSAYO RAPIDO: captura en RAM, volcado al final ----------
  configurar(REG1_RAPIDO);                 // 1000 SPS

  if (factor_kg == 0.0f) {
    Serial.println(F("AVISO: factor_kg = 0. Calibra primero con el MODO 3."));
    Serial.println(F("El ensayo igual corre, pero solo se guardan conteos.\n"));
  }
  Serial.printf("Capacidad: %d muestras = %.1f s a 1000 SPS (%d KB de RAM)\n\n",
                N_MUESTRAS, N_MUESTRAS / 1000.0, (N_MUESTRAS * 4) / 1024);

  // --- Tara ---
  Serial.println(F("Deja la celda SIN CARGA. Tarando en 3 segundos..."));
  delay(3000);
  int64_t suma = 0;
  const int N_TARA = 1000;                 // ~1 s a 1000 SPS
  for (int i = 0; i < N_TARA; i++) {
    adsWaitDRDY(500);
    suma += adsReadData();
  }
  cero = suma / N_TARA;
  Serial.printf("Cero (tara) = %ld\n\n", cero);

  // --- Espera del disparo ---
  Serial.println(F("LISTO PARA EL ENSAYO."));
  Serial.println(F("Envia cualquier tecla para empezar a capturar."));
  Serial.println(F("La captura arranca de inmediato: deja 2-3 s de linea"));
  Serial.println(F("base antes de encender el motor.\n"));
  while (!Serial.available()) { delay(10); }
  while (Serial.available()) Serial.read();   // vacia el buffer serie

  // --- Captura (nada lento debe interponerse aqui) ---
  Serial.println(F("CAPTURANDO..."));
  delay(50);                                  // deja salir el mensaje
  uint32_t perdidas = 0;
  t_inicio = micros();
  for (uint32_t i = 0; i < N_MUESTRAS; i++) {
    uint32_t t0 = micros();
    while (digitalRead(PIN_DRDY) == HIGH) {
      if (micros() - t0 > 50000) { perdidas++; break; }   // 50 ms de guardia
    }
    buffer[i] = adsReadData();
    yield();                                  // deja respirar al RTOS
  }
  t_fin = micros();

  // --- Volcado ---
  float dur_s = (t_fin - t_inicio) / 1e6f;
  float sps   = N_MUESTRAS / dur_s;
  Serial.println(F("\n--- FIN DE CAPTURA ---"));
  Serial.printf("Duracion real : %.3f s\n", dur_s);
  Serial.printf("Tasa efectiva : %.1f SPS\n", sps);
  Serial.printf("Muestras tarde: %lu\n", perdidas);
  Serial.printf("Cero usado    : %ld\n", cero);
  Serial.printf("factor_kg     : %.9f\n\n", factor_kg);

  Serial.println(F("Copia desde la linea siguiente y guardala como .csv"));
  Serial.println(F("t_ms,conteo,neto,kg,N"));

  float dt_ms = (dur_s * 1000.0f) / N_MUESTRAS;
  for (uint32_t i = 0; i < N_MUESTRAS; i++) {
    int32_t neto = buffer[i] - cero;
    float   kg   = neto * factor_kg;
    Serial.printf("%.3f,%ld,%ld,%.4f,%.3f\n",
                  i * dt_ms, buffer[i], neto, kg, kg * 9.81f);
  }
  Serial.println(F("\n--- FIN DE DATOS ---"));
  Serial.println(F("Pulsa RESET en el ESP32 para otro ensayo."));
#endif
}

// ---------------------------------------------------------------

void loop() {
#if MODO == 1
  if (adsWaitDRDY(1000)) {
    int32_t raw = adsReadData();
    float tempC = (raw >> 10) * 0.03125f;   // resultado de 14 bits
    Serial.printf("raw=%8ld   temperatura= %.2f C\n", raw, tempC);
  } else {
    Serial.println(F("Timeout de DRDY: no llega el 'dato listo' (GPIO 22)."));
  }
  delay(500);

#elif MODO == 2
  if (adsWaitDRDY(1000)) {
    int32_t raw = adsReadData();
    // Voltaje de entrada equivalente: FS = +-VREF/ganancia = +-3.3/128
    float mV = (raw / 8388608.0f) * (3.3f / 128.0f) * 1000.0f;
    Serial.printf("conteo=%9ld   entrada= %+8.4f mV\n", raw, mV);
  } else {
    Serial.println(F("Timeout de DRDY."));
  }
  delay(200);

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
#endif
}