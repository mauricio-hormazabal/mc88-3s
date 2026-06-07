#include "pico/time.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "include/velocity_estimator.h"
#include "include/midi_out.h"
#include "include/usb_midi_out.h"
//#include "include/midi_parser.h"

#define MIDI_UART_ID uart0
#define MIDI_TX_PIN 0
// Para enviar Active Sense
#define MIDI_RX_PIN 1

#define MIDI_BAUDRATE 31250

#define MIDI_BUFFER_SIZE 64

#define NOTA_MIDI_BASE 21

#define K_NW 0
#define K_W_PMK_F_BR 1
#define K_W_PMK_F_MD 2
#define K_W_RBR_F_MK 3

static uint8_t midi_buffer[MIDI_BUFFER_SIZE][3];
static int buffer_head = 0, buffer_tail = 0;

static uint8_t key_state[NUM_KEYS];


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
        // integracion USB-C 2026-06-06
        usb_send_midi_packet(status, data1, data2);
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



// manejo de la tecla

void init_key_state(){
    for (uint8_t nk =0; nk < NUM_KEYS; nk ++){
        key_state[nk] = K_NW;
    }
}

void handle_key_event(int row, int n_col, ColType type, int pressed) {
    
    int key = n_col * 8 + row; // cada columna son 8 teclas consecutivas conectadas cada una a una de añs 8 filas.

    int note = NOTA_MIDI_BASE + key;

    if (type == BR) {   // primer switch
      
        if (pressed) {  

            register_br_time(key, get_absolute_time());
            set_key_state(key, K_W_PMK_F_BR);

        } 
        else { // NOTE-OFF al liberar totalmente la tecla.

            // verifico que estoy esperando la liberacion de BR desde un MK
            // si es así, estimo la velocidad de note-off, cambio el estado a tecla liberada (Not Waiting)
            if (key_state[key] == K_W_RBR_F_MK){
                
                send_midi(0x80, note, 0);

                set_key_state(key, K_NW);

            }

        }
    }
    else if (type == MK) {  // tercer switch
       
        if (pressed) {  // Note ON

            register_mk_time(key, get_absolute_time());

            if (key_state[key] == K_W_PMK_F_BR){

                int vel = estimate_velocity(key);

                send_midi(0x90, note, vel);           // 0x90 = NOTE ON

                set_key_state(key, K_W_RBR_F_MK);

            }

        } else {  // Note OFF

            // MD: Registro el tiempo de liberación, register_mk_release_time(key, get_absolute_time());
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

void set_key_state(uint8_t key, uint8_t ks){

    if (key >= 0 && key < NUM_KEYS){
        key_state[key] = ks;     
    }

}