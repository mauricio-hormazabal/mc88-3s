#include "pico/time.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "include/velocity_estimator.h"
#include "include/midi_out.h"
//#include "include/midi_parser.h"

#define MIDI_UART_ID uart0
#define MIDI_TX_PIN 0
// Para enviar Active Sense
#define MIDI_RX_PIN 1

#define MIDI_BAUDRATE 31250

#define MIDI_BUFFER_SIZE 64

#define NOTA_MIDI_BASE 21

static uint8_t midi_buffer[MIDI_BUFFER_SIZE][3];
static int buffer_head = 0, buffer_tail = 0;


void init_midi() {
    uart_init(MIDI_UART_ID, MIDI_BAUDRATE);
    gpio_set_function(MIDI_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(MIDI_RX_PIN, GPIO_FUNC_UART);
}

void send_midi(uint8_t status, uint8_t data1, uint8_t data2) {
    int next_head = (buffer_head + 1) % MIDI_BUFFER_SIZE;
    if (next_head != buffer_tail) {
        midi_buffer[buffer_head][0] = status;
        midi_buffer[buffer_head][1] = data1;
        midi_buffer[buffer_head][2] = data2;
        buffer_head = next_head;
    }
}

void midi_task() {
    if (buffer_tail != buffer_head && uart_is_writable(MIDI_UART_ID)) {
        uart_putc_raw(MIDI_UART_ID, midi_buffer[buffer_tail][0]);
        uart_putc_raw(MIDI_UART_ID, midi_buffer[buffer_tail][1]);
        uart_putc_raw(MIDI_UART_ID, midi_buffer[buffer_tail][2]);
        buffer_tail = (buffer_tail + 1) % MIDI_BUFFER_SIZE;
    }
}

//Aca debe estar la o las funcion que envia el OK en formato System Exclusive (SysEx)

// manejo de la tecla
void handle_key_event(int row, int n_col, ColType type, int pressed) {
    
    int key = n_col * 8 + row; // cada columna son 8 teclas consecutivas conectadas cada una a una fila distinta.

    int note = NOTA_MIDI_BASE + key;

    if (type == BR) {   // primer switch
      
        if (pressed) {  // solo guardamos cuando se presiona
            register_br_time(key, get_absolute_time());
        } 
        else { // prueba de mover el note-off al liberar totalmente la tecla.
            send_midi(0x80, note, 0);
        }
    }
    else if (type == MK) {  // tercer switch
       
        if (pressed) {  // Note ON

            register_mk_time(key, get_absolute_time());

            int vel = estimate_velocity(key);

            send_midi(0x90, note, vel);           // 0x90 = NOTE ON

        } else {  // Note OFF

            // send_midi(0x80, note, 0);             // 0x80 = NOTE OFF
            // MD: Registro el tiempo de liberación, register_mk_release_time(key, get_absolute_time());
            // se debería cambiar el envío de note_off por el envío de note con velocidad 0.
        }
    }
    else if (type == MD) { // switch intermedio
        // Este es el equivalente del MK, pero en  lógica inversa, es decir:
        // se libera MK, se mide el tiempo, se libera MDK, se mide el tiempo, se calcula la diferencia, y se envía
        // como velocidad de liberación con el mensaje note_off.
        // También se utiliza cuando se estima la velocidad del note_on. Si el tiempo registrado por MD es más
        // nuevo que el tiempo registrado por BR, significa que se activó MK, sin haber generado un nuevo BR, es decir
        // no se soltó completamente la tecla antes de volver a presionarla. Se debe calcular la velocidad entre MD y MK,
        // normalizada para esta nueva "distancia".
        // Cuando se libera el sensor medio, medimos el tiempo y enviamos el note_off.
        // if (!pressed){
        //    register_md_time(key, get_absolute_time());
        //    int rel_vel = estimate_release_velocity(key);
        //    send_midi(0x90, note, rel_vel);
        // }

    }
}
