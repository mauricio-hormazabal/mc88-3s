#include "include/midi_in.h"
#include "include/midi_out.h"
#include "include/midi_parser.h"
#include "include/midi_active_sense.h"


void midi_in_check_uart(){

    if (uart_is_readable(uart0)) {
        uint8_t byte = uart_getc(uart0);
        process_midi_byte(byte);
        active_sense_on_midi_received(); // reinicia temporizador
    }
}