#ifndef MIDI_ACTIVE_SENSE_H
#define MIDI_ACTIVE_SENSE_H

#include "hardware/timer.h"

void init_active_sense();
void active_sense_send();
void active_sense_send_byte(uint8_t data);
void active_sense_task();
void active_sense_on_midi_received();

#endif