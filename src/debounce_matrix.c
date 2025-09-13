#include "include/debounce_matrix.h"

#define DEBOUNCE_THRESHOLD 10  // era 3 número de ciclos consecutivos necesarios para confirmar un cambio

static DebounceState debounce_matrix[MATRIX_ROWS][MATRIX_COLS];

void init_debounce_matrix(void) {
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            debounce_matrix[row][col].stable_state = false;
            debounce_matrix[row][col].last_raw_state = false; 
            debounce_matrix[row][col].counter = 0;
        }
    }
}

bool debounce_update(int row, int col, bool raw_state) {
    DebounceState* state = &debounce_matrix[row][col];

    if (raw_state != state->last_raw_state) {
        state->counter = 1;
        state->last_raw_state = raw_state;
    } else if (state->counter > 0 && state->counter < DEBOUNCE_THRESHOLD) {
        state->counter++;
        if (state->counter >= DEBOUNCE_THRESHOLD) {
            state->stable_state = raw_state;
            return true;  // cambio confirmado
        }
    }

    return false;  // sin cambio, o sin cambio estable
}

bool debounce_get_state(int row, int col) {
    return debounce_matrix[row][col].stable_state;
}
