#ifndef MATRIX_SCAN_H
#define MATRIX_SCAN_H

#include <stdbool.h>
#include <stdint.h>

void init_matrix(void);
void scan_matrix(void);

// prueba nuevo modelo de escaneo
void scan_map_matrix(void);

void set_channel_active(uint8_t mux);
static inline void set_mux_channel(uint8_t channel);

typedef enum {
    BR,
    MD,
    MK
} ColType;

typedef struct {
    uint8_t mux;        // Número de multiplexor (0..4)
    uint8_t canal;      // Entrada del mux (0..7)
    uint8_t columna;    // Índice lógico en su grupo (0..10)
    ColType tipo;       // BR / MD / MK
} ColumnMap;

#endif
