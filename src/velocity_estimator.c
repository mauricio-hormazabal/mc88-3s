#include "include/velocity_estimator.h"
#include "pico/stdlib.h"
#include <math.h>

#define KEY_OUT_OF_RANGE 64
#define KEY_NO_BR_OR_MK 48 
#define KEY_NOTE_PLAUSIBLE 32

#include "include/velocity_estimator.h"
#include <math.h>
/**
 * Revisar el rango de dt_ms. Calcular un promedio de cuan lento se puede tocar una tecla 
 * de cada una las 4 graduaciones del teclado -> Definirlo como MAX_MS
 * Con una fuerza uniforme calcular un DT para cada tecla, calcular promedio y desviación en porcentaje
 * Calibrar cada tecla, en su mínimo y máximo, de acuerdo a esa desviación
 */
// Variables internas al módulo, ocultas al resto del programa
static absolute_time_t mk_time[NUM_KEYS];
static absolute_time_t br_time[NUM_KEYS];

static int velocity_curve_mode = VELOCITY_LOGARITHMIC; // default: logarítmica

// Tabla de calibración para cada tecla (88 teclas)
KeyCalibration key_calibration[NUM_KEYS];

void init_velocity_estimator(void) {

    for (int i = 0; i < NUM_KEYS; i++) {

        mk_time[i] = at_the_end_of_time;
        br_time[i] = at_the_end_of_time;
    }

    init_velocity_calibration(5.0f, 40.0f); 

    init_individual_keys();
}

void set_velocity_curve(int mode) {
    velocity_curve_mode = mode;
}

void register_mk_time(int key, absolute_time_t t) {
    if (key >= 0 && key < NUM_KEYS) {
        mk_time[key] = t;
    }
}

void register_br_time(int key, absolute_time_t t) {
    if (key >= 0 && key < NUM_KEYS) {
        br_time[key] = t;
    }
}

// Velocidad mínima: 6ms
// Velocidad máxima: [40ms,50ms]

// si el tiempo de MDK es más reciente que el de BR, significa que la estimación de velocidad activada
// por MK se realizó sin generar un timepo de BR nuevo, es decir, no se soltó completamente la tecla.
// en este caso, se debe calcular el tiempo normalizando para la "distancia" entre MDK y MK. Quizas es mejor
// construir una función nueva que derive a la función de estimación de velocidad usada hasta ahora (BR-MK) o a 
// la nueva función de estimación de velocidad entre MDK y MK.

int estimate_velocity(int key) {

    float MIN_MS = key_calibration[key].min_ms;
    float MAX_MS = key_calibration[key].max_ms;

    if (key < 0 || key >= NUM_KEYS) return KEY_OUT_OF_RANGE;

    // mientras existan dos sensores, se debería retornar velocidad 0 si no hay BR (No afecta a si no hay MK)
    // Ya no debería ser necesario si se está verificando que la "tecla" esté en estado de espera de MK desde
    // un BR

    if (is_at_the_end_of_time(br_time[key]) ||
        is_at_the_end_of_time(mk_time[key])) {
        return KEY_NO_BR_OR_MK; // neutro si falta BR o MK
    }

    int dt_us = absolute_time_diff_us(br_time[key], mk_time[key]);
    if (dt_us <= 0) return KEY_NOTE_PLAUSIBLE;


    float dt_ms = dt_us / 1000.0f;

    if (dt_ms < MIN_MS) dt_ms = MIN_MS;
    if (dt_ms > MAX_MS) dt_ms = MAX_MS;

    float velocity = 64.0f;

    switch (velocity_curve_mode) {
        
        case VELOCITY_RAW_DT: {
            velocity = dt_ms/2.0f;
            break;
        }

        case VELOCITY_LINEAR: {
            //velocity = 127.0f * (1.0f - (dt_ms - MIN_MS) / (MAX_MS - MIN_MS));
            // y = 45* (-126 / (b - a)) + (1 - (-126 / (b - a))*b)   where b=45 and a = 6
            velocity = dt_ms * (-126 / (MAX_MS - MIN_MS)) + (1 - (-126 / (MAX_MS - MIN_MS))*MAX_MS);
            break;
        }

        case VELOCITY_LOG: {
            //  Y=x*(0.9/(45-6)) + (1.0 - (0.9/(45-6))*45),  6 < x  < 45
            float c = 1.0f - (0.9f / (MAX_MS - MIN_MS)) * MAX_MS;
            float m = (0.9f / (MAX_MS - MIN_MS));
            float dtn = m * dt_ms + c;
            // y=((1-log10(x)) -1)*126 +1
            velocity = 126 * ((1 - log10f(dtn)) - 1 ) + 1;
            break;
        }

        case VELOCITY_LOGARITHMIC: {
            // 2.06 para ln
            // 1.xx para log10
            float kmh = 1.73f; //1.92
            //float norm = logf(dt_ms + 1.0f) / logf(MAX_MS + 1.0f);  
            float norm = log10f(dt_ms) / log10f(MAX_MS);
            velocity = 127.0f * (1.0f - norm) * kmh;
            break;
        }

        case VELOCITY_LOG_POW: {
            float e = 0.4f;
            float norm = logf(dt_ms + 1.0f) / logf(MAX_MS + 1.0f);
            velocity = 127.0f * pow((1.0f - norm), e);
            break;

        }

        case VELOCITY_GAMMA: {
            float gamma = 4.0f;
            float norm = (dt_ms - MIN_MS)/ (MAX_MS - MIN_MS);  
            velocity = 127.0f * pow((1.0f - norm), gamma);
            break;
        }
        case VELOCITY_EXPONENTIAL: {
            float k = 0.1f; // ajustable
            velocity = 127.0f * expf(-k * dt_ms);
            break;
        }

        default:
            velocity = 64.0f;
    }

    
    if (velocity < 6.0f) velocity = 6.0f;
    if (velocity > 126.0f) velocity = 126.0f;

    return (int)(velocity + 0.5f); // redondeo
}

// CALIBRACIONES

// Inicializa la tabla con valores globales por defecto
void init_velocity_calibration(float global_min_ms, float global_max_ms) {
    for (int i = 0; i < NUM_KEYS; i++) {
        key_calibration[i].min_ms = global_min_ms;
        key_calibration[i].max_ms = global_max_ms;
    }
}

// Permite ajustar una tecla individual
void set_key_calibration(uint8_t key, float min_ms, float max_ms) {

    if (key >= NUM_KEYS) return;

    key_calibration[key].min_ms = min_ms;
    key_calibration[key].max_ms = max_ms;

}

void init_individual_keys(){

    // las negras parecen ser 5.0f min y 35.0f max 
    // las blancas entre 6.0f y 45.0f
    // tecla 70, 73 y 75 muy fuerte --> Valor - NOTA_MIDI_BASE
    // mk5 o br5 puede estar mal conectado (a un mdk5)
    /**
    set_key_calibration(midi_to_key(70), 1.0f, 15.0f);  // 20.0f
    set_key_calibration(midi_to_key(73), 1.0f, 17.0f);  // 22.0f
    set_key_calibration(midi_to_key(75), 1.0f, 15.0f);  // 20.0f

    set_key_calibration(midi_to_key(69), 1.0f, 22.0f);
    set_key_calibration(midi_to_key(71), 1.0f, 22.0f);  
    set_key_calibration(midi_to_key(72), 1.0f, 22.0f);  
    set_key_calibration(midi_to_key(74), 1.0f, 22.0f);  
    set_key_calibration(midi_to_key(76), 1.0f, 22.0f);
    */  
}

 uint8_t midi_to_key(uint8_t midi){
     uint8_t key = midi - NOTA_MIDI_BASE;
    if (key < 0 || key > 88){
         return 0;
    }
    return key;
}