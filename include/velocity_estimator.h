#ifndef VELOCITY_ESTIMATOR_H
#define VELOCITY_ESTIMATOR_H

#include "pico/stdlib.h"
#include "pico/time.h"

#define NUM_KEYS 88
#define NOTA_MIDI_BASE 21

// Modos de curva de velocidad
#define VELOCITY_RAW_DT 99
#define VELOCITY_LINEAR 0
#define VELOCITY_LOG 1
#define VELOCITY_LOGARITHMIC 2
#define VELOCITY_EXPONENTIAL 3
#define VELOCITY_LOG_POW 4
#define VELOCITY_GAMMA 5

// Inicializa el estado del estimador
void init_velocity_estimator(void);

// Configura el modo de curva
void set_velocity_curve(int mode);

// Registra la activación de los contactos
void register_br_time(int note, absolute_time_t t);
void register_mk_time(int note, absolute_time_t t);


typedef struct {
    float min_ms;
    float max_ms;
} KeyCalibration;

//extern KeyCalibration key_calibration[NUM_KEYS];

// Inicialización
void init_velocity_calibration(float global_min_ms, float global_max_ms);

// Ajuste individual por tecla
void set_key_calibration(uint8_t key, float min_ms, float max_ms);

// Ajustes de tecla individual
void init_individual_keys();

// Estima la velocity MIDI de una nota (1–127)
int estimate_velocity(int note);

uint8_t midi_to_key(uint8_t midi);

#endif // VELOCITY_ESTIMATOR_H