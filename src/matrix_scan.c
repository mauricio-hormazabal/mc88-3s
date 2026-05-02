#include "include/matrix_scan.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include "include/debounce_matrix.h"
#include "include/midi_out.h"

#define NUM_ROWS 8 
#define NUM_MUX 5
#define MUX_CHANNELS 8  // 74HC4051 tiene 8 entradas
#define NUM_COLUMNS 33  // 11 BR + 11 MD + 11 MK


// Pines RP2350
static const uint ROW_PINS[NUM_ROWS] = {2, 3, 4, 5, 6, 7, 8, 9};
static const uint MUX_SELECT_PINS[3] = {10, 11, 12}; 
static const uint MUX_Z_PINS[NUM_MUX] = {13, 14, 15, 16, 17};


// Escaneo en "sentido inverso"
void init_matrix(void) {
    // Configurar filas como entradas con pull up
    // pines de "observacion". 
    for (int i = 0; i < NUM_ROWS; i++) {
        gpio_init(ROW_PINS[i]);
        gpio_set_dir(ROW_PINS[i], GPIO_IN);
        gpio_pull_up(ROW_PINS[i]);
    }

    // Pines de selección S0, S1, S2
    for (int i = 0; i < 3; i++) {
        gpio_init(MUX_SELECT_PINS[i]);
        gpio_set_dir(MUX_SELECT_PINS[i], GPIO_OUT);
        gpio_put(MUX_SELECT_PINS[i], 0);
    }

    // Pines Z delos multiplexores conectados a los pines de salida
    // del MC.
    // Todos en HIGH 

    for (int i = 0; i < NUM_MUX; i++) {
        gpio_init(MUX_Z_PINS[i]);
        gpio_set_dir(MUX_Z_PINS[i], GPIO_OUT);
        gpio_put(MUX_Z_PINS[i], 1);
    }

}

// Matriz que mapea canales de los multiplexores, con su respectiva columna y tipo asociado.
// Mux, Channel, Column, Type

const ColumnMap mux_map[5][8] = {
    // MUX 0
    {
        {0, 0, 0, BR}, {0, 1, 1, BR}, {0, 2, 2, BR},
        {0, 3, 3, BR}, {0, 4, 4, BR}, {0, 5, 5, BR},
        {0, 6, 6, BR}, {0, 7, 7, BR},
    },

    // MUX 1
    {
        {1, 0, 8, BR}, {1, 1, 9, BR}, {1, 2, 10, BR},
        {1, 3, 0, MD}, {1, 4, 1, MD}, {1, 5, 2, MD},
        {1, 6, 3, MD}, {1, 7, 4, MD}, 
    },

    // MUX 2
    {
        {2, 0, 5, MD}, {2, 1, 6, MD}, {2, 2, 7, MD},
        {2, 3, 8, MD}, {2, 4, 9, MD}, {2, 5, 10, MD},
        {2, 6, 0, MK}, {2, 7, 1, MK},
    },

    // MUX 3
    {
        {3, 0, 2, MK}, {3, 1, 3, MK}, {3, 2, 4, MK},
        {3, 3, 5, MK}, {3, 4, 6, MK}, {3, 5, 7, MK},
        {3, 6, 8, MK}, {3, 7, 9, MK},
    },

    // MUX 4
    {
        {4, 0, 10, MK}, {}, {},
        {},{},{},
        {},{},
    }

};


void scan_map_matrix(void){

    for (int mux = 0; mux < NUM_MUX ; mux ++){
        
		for (int channel = 0; channel < MUX_CHANNELS ; channel++){

            // Se cuenta cuantas columnas se han leido. Si son >= 33, paramos.
            uint col = mux * MUX_CHANNELS + channel;
            if (col >= NUM_COLUMNS) continue; 

            set_mux_channel(channel);

            set_channel_active(mux);

            int column = mux_map[mux][channel].columna;
            ColType type = mux_map[mux][channel].tipo;

            for (int row = 0 ; row < NUM_ROWS; row++){

            int raw_state = (!gpio_get(ROW_PINS[row])) ? 1: 0; 

			int debounced = debounce_update(row, col, raw_state);

                if (debounced){
                        bool pressed = raw_state;
                        handle_key_event(row, column, type, pressed);
                }

            }

            gpio_put(MUX_Z_PINS[mux], 1);

		}

	}
}

// selección del canal del multiplexor
static inline void set_mux_channel(uint8_t channel) {
    gpio_put(MUX_SELECT_PINS[0], (channel>>0) & 1);
    gpio_put(MUX_SELECT_PINS[1], (channel>>1) & 1);
    gpio_put(MUX_SELECT_PINS[2], (channel>>2) & 1);
}

// activacion del canal del multiplexor
void set_channel_active(uint8_t mux){

	for (int m = 0; m < NUM_MUX; m++) {
        gpio_put(MUX_Z_PINS[m], 1);  
    }

    gpio_put(MUX_Z_PINS[mux], 0);
   
    sleep_us(5);
}

