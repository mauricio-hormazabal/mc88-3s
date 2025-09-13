#ifndef MIDI_OUT_H
#define MIDI_OUT_H

#include <stdint.h>
#include "include/matrix_scan.h"

void init_midi(void);
void send_midi(uint8_t status, uint8_t data1, uint8_t data2);
void midi_task(void);
void handle_key_event(int row, int col, ColType columnn, int raw_state);

#endif
