#ifndef PEDALS_H
#define PEDALS_H

#include <stdint.h>
#include <stdbool.h>

// Inicializa pines de pedales y estado interno
void init_pedals(uint8_t midi_channel);

// Llamar periódicamente (p. ej. cada 1 ms)
// Envía CC64, CC66, CC67 cuando hay cambios de estado (0/127)
void pedals_tick(void);

// (Opcional) Cambiar canal en runtime
void pedals_set_channel(uint8_t midi_channel);

#endif
