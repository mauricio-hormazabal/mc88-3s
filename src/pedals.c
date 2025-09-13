
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/uart.h"
#include "include/pedals.h"

// ---- Ajustes ----
#define PEDAL_DEBOUNCE_MS   8      // anti-rebote robusto
#define MIDI_UART_ID        uart0
#define MIDI_BAUD           31250

// Pines (cámbialos si lo necesitas)
static const uint PEDAL_PINS[3] = {18, 19, 20}; // Sustain, Sostenuto, UnaCorda
// Control Change asignado a cada pedal
static const uint8_t PEDAL_CC [3] = {64, 66, 67}; // Sustain, Sostenuto, Soft

// Estado interno
typedef struct {
    bool      stable;          // estado estable actual (true = ON)
    bool      last_sample;     // última lectura
    uint16_t  counter_ms;      // contador para debounce
} DebounceState;

static DebounceState st[3];
static uint8_t g_midi_channel = 0; // canal 0 = MIDI canal 1

// --- MIDI helpers ---
static inline void midi_send_cc(uint8_t ch, uint8_t cc, uint8_t val) {
    uart_putc_raw(MIDI_UART_ID, 0xB0 | (ch & 0x0F)); // mensaje CC (4 bits MSB)| Numero de Canal (4 bits LSB)
    uart_putc_raw(MIDI_UART_ID, cc & 0x7F); // mascara para 7 bits
    uart_putc_raw(MIDI_UART_ID, val & 0x7F); // mascara para 7 bits
}

// --- Init ---
void pedals_init(uint8_t midi_channel) {
    g_midi_channel = (midi_channel & 0x0F);

    // Configuración de UART (Ya está en en otro módulo)
    //uart_init(MIDI_UART_ID, MIDI_BAUD);
    //gpio_set_function(0, GPIO_FUNC_UART); // TX en GP0 (ajusta si usas otro)

    for (int i = 0; i < 3; i++) {
        gpio_init(PEDAL_PINS[i]);
        gpio_set_dir(PEDAL_PINS[i], GPIO_IN);
        gpio_pull_up(PEDAL_PINS[i]); // reposo = HIGH (no pisado)
        st[i].stable = false;        // estado MIDI enviado (off)
        st[i].last_sample = false;
        st[i].counter_ms = 0;
    }
}

void pedals_set_channel(uint8_t midi_channel) {
    g_midi_channel = (midi_channel & 0x0F);
}

// Lee GPIO (activo en LOW) y devuelve booleano ON/OFF
static inline bool read_pedal_on(uint gpio_pin) {
    // Con pull-up: 1 = no pisado, 0 = pisado
    return gpio_get(gpio_pin) ? false : true;
}

// Llamar cada ~1 ms
void pedals_tick(void) {
    for (int i = 0; i < 3; i++) {
        bool cur = read_pedal_on(PEDAL_PINS[i]);

        if (cur != st[i].last_sample) {
            // Cambio detectado: reinicia contador
            st[i].last_sample = cur;
            st[i].counter_ms = 0;
        } else {
            // Mismo nivel consecutivo: incrementa contador
            if (st[i].counter_ms < 0xFFFF) st[i].counter_ms++;
            // ¿Estable?
            if (st[i].counter_ms >= PEDAL_DEBOUNCE_MS) {
                if (st[i].stable != cur) {
                    st[i].stable = cur;
                    // Enviar CC solo si cambió estado estable
                    uint8_t val = st[i].stable ? 127 : 0;
                    midi_send_cc(g_midi_channel, PEDAL_CC[i], val);
                }
            }
        }
    }
}
/*
Conexión sugerida (on/off conmutadores a masa)
Cada pedal es un interruptor normalmente abierto que cierra a GND. Usaremos pull-ups internos del Pico (3.3 V).
Pines sugeridos (puedes cambiarlos fácil en el código):

Sustain → GP18

Sostenuto → GP19

Una Corda → GP20

Esquema (texto):

Punta del pedal (o el cierre del switch) → GPIO (GP18 / GP19 / GP20)

El otro terminal del pedal → GND

(Opcional pero recomendable en cables largos) Serie 1 kΩ entre jack y GPIO + 100 nF a GND en el lado del GPIO para un pequeño RC hardware (igual tendremos debounce por software).

Activa GPIO Pull-Up por software → la línea queda en “1” en reposo; al pisar baja a “0”.

Notas rápidas
Lógica: interruptor a GND, pull-up interno → “pisado = LOW” → lo invertimos a true para enviar 127.

Debounce: 8 ms es seguro y no se siente “lento” en pedales. Puedes bajarlo a 5 ms sin problema.

Seguridad: si usas cables largos a pedales, el pequeño RC (1 k + 100 nF) cerca del GPIO reduce rebotes/EMI.

Compatibilidad: los CC elegidos son los estándar de piano.
*/