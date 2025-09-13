
#ifndef DEBOUNCE_MATRIX_H
#define DEBOUNCE_MATRIX_H

#include <stdint.h>
#include <stdbool.h>

#define MATRIX_ROWS 8
#define MATRIX_COLS 33

// Estado de un contacto
typedef struct {
    bool stable_state;
    bool last_raw_state;
    uint8_t counter;
} DebounceState;

// Inicializa la matriz de rebote
void init_debounce_matrix(void);

// Procesa el estado crudo y devuelve true si hay un cambio estable
bool debounce_update(int row, int col, bool raw_state);

// Devuelve el estado estable actual
bool debounce_get_state(int row, int col);

#endif
