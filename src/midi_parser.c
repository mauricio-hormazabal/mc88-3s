#include <stdint.h>
#include <stdbool.h>
#include "include/midi_parser.h"
#include "include/velocity_estimator.h"

static uint8_t status = 0;
static uint8_t data1 = 0;
static bool waiting_data1 = false;
static bool waiting_data2 = false;

void process_midi_byte(uint8_t byte) {
    if (byte & 0x80) {
        status = byte;
        waiting_data1 = true;
        waiting_data2 = false;
        return;
    }

    if (waiting_data1) {
        data1 = byte;
        waiting_data1 = false;
        waiting_data2 = true;
    } else if (waiting_data2) {
        waiting_data2 = false;
    
        if ((status & 0xF0) == 0xB0 && (status & 0x00) == 0x00) { //(4 MSB == 0xB0 0) -> CC  on channel 0 (4 LSB == 0x00 )
            // Controlador alta resolucion de velocidad
            if (data1 & 0x58 == 0x58) {
                set_velocity_curve(byte);
            }
        }
    }

}
