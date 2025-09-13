// Active Sense. Intento de conexión con UART de Megawin de la placa SAM5716B.
#include "hardware/uart.h"
#include "include/midi_out.h"
#include "pico/time.h"

#define MIDI_ACTIVE_SENSE 0xFE
#define MIDI_UART_ID uart0

static absolute_time_t last_active_sense = 0;

// inicializacion active sense
void init_active_sense(){
        last_active_sense = get_absolute_time();
}

// Funcion para envio de byte
void active_sense_send_byte(uint8_t data) {
    uart_putc_raw(MIDI_UART_ID, data);
    // cada vez que enviamos algo MIDI, reseteamos timer
    last_active_sense = get_absolute_time();
}

// Envio de active sense
void active_sense_send() {
    active_sense_send_byte(MIDI_ACTIVE_SENSE);
}

void active_sense_task() {
    absolute_time_t now = get_absolute_time();
    int64_t elapsed = absolute_time_diff_us(last_active_sense, now);

    if (elapsed >= 250000) { // 250 ms = 250000 us
        active_sense_send();
        last_active_sense = now;
    }
}

void active_sense_on_midi_received() {
    // Llamar desde tu ISR o parser cuando entre un byte MIDI válido
    last_active_sense = get_absolute_time();
}