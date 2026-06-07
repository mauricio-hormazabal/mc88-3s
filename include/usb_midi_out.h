#ifndef USB_MIDI_OUT_H
#define USB_MIDI_OUT_H

#include <stdint.h>
#include <stdbool.h>

void init_usb_midi_out(void);
bool usb_send_midi_packet(uint8_t status, uint8_t data1, uint8_t data2);
void usb_midi_task(void);
bool usb_midi_connected(void);

#endif // USB_MIDI_OUT_H
