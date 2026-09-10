#include <Arduino.h>
#include <string>

enum modo_n{
    TEST_SPI,
    RAW_READ,
    CALIBRATION,
    ENSAYO
};

enum tipo_s{
    KILO,
    NEWTON,
    RAW
};

enum modo_n modo_actual = RAW_READ;
const char serial_read_start = 192;
const char serial_read_stop = 100;

int32_t cero = 0;
float   factor_kg = 0.000438671f; //Numeero mágico, gracias Daniel

const int N_MUESTRAS = 12000;
int32_t buffer[N_MUESTRAS];
uint32_t t_inicio, t_fin;

void configurar(uint8_t reg1);
bool verificarRegistros(uint8_t reg1);
void wait_mode_confirmation();
bool is_stop();
void set_all();

float get_temp();

void test_spi();
void raw_read();
void calibration();
void ensayo();