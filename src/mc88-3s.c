#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "include/matrix_scan.h"
#include "include/debounce_matrix.h"
#include "include/midi_in.h"
#include "include/midi_out.h"
#include "include/midi_active_sense.h"
#include "include/midi_parser.h"
#include "include/velocity_estimator.h"
#include "include/pedals.h"


#define SCAN_INTERVAL_US 1000

int64_t last_scan_time = 0;

int main() {
    stdio_init_all();

    // inicializa la matriz antirebotes
    init_debounce_matrix();

    // inicializa pines para la matriz de escaneo
    //init_matrix();
    init_matrix();

    // modulo midi
    init_midi();

    // inicializa el estado de las teclas
    init_key_state();

    // active sense 
    //init_active_sense();

    // inicializa el estimador de velocidad
    init_velocity_estimator();

    // configura curva de velocidad por defecto
    set_velocity_curve(VELOCITY_LOGARITHMIC); 

    sleep_ms(100); 

    while (true) {

        int64_t now = time_us_64();

        if (now - last_scan_time >= SCAN_INTERVAL_US) {

            scan_map_matrix();

            //pedals_tick(); // anti-rebote y envío CC si hay cambios

            // envio de active sense
            //active_sense_task();  

            //revisar mensajes UART entrantes.
            //midi_in_check_uart();

            // escaneo de teclado y pedales cada 1ms (1000us)
            last_scan_time = now;
        }

        midi_task();
    }
    return 0;
}
