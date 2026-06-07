#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "include/usb_midi_out.h"

#define USB_MIDI_BUFFER_SIZE 64

static uint8_t usb_midi_buffer[USB_MIDI_BUFFER_SIZE][3];
static int usb_midi_head = 0;
static int usb_midi_tail = 0;

void init_usb_midi_out(void) {
    tusb_rhport_init_t const usb_init_config = {
        .role  = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_FULL,
    };
    tusb_init(BOARD_TUD_RHPORT, &usb_init_config);
}

bool usb_send_midi_packet(uint8_t status, uint8_t data1, uint8_t data2) {
    int next_head = (usb_midi_head + 1) % USB_MIDI_BUFFER_SIZE;
    if (next_head == usb_midi_tail) {
        return false;
    }
    usb_midi_buffer[usb_midi_head][0] = status;
    usb_midi_buffer[usb_midi_head][1] = data1;
    usb_midi_buffer[usb_midi_head][2] = data2;
    usb_midi_head = next_head;
    return true;
}

void usb_midi_task(void) {
    tud_task();

    while (usb_midi_tail != usb_midi_head && tud_midi_mounted()) {
        if (tud_midi_stream_write(0, usb_midi_buffer[usb_midi_tail], 3) != 3) {
            break;
        }
        usb_midi_tail = (usb_midi_tail + 1) % USB_MIDI_BUFFER_SIZE;
    }

    while (tud_midi_available()) {
        uint8_t packet[4];
        tud_midi_packet_read(packet);
    }
}

bool usb_midi_connected(void) {
    return tud_midi_mounted();
}

// USB MIDI descriptors and callbacks

const tusb_desc_device_t desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0xCafe,
    .idProduct          = 0x4004,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01,
};

uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *)&desc_device;
}

enum {
    ITF_NUM_MIDI,
    ITF_NUM_MIDI_STREAMING,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN)
#define EPNUM_MIDI_OUT    0x01
#define EPNUM_MIDI_IN     0x81

uint8_t const desc_fs_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 0, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64),
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    return desc_fs_configuration;
}

enum {
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
};

char const *string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },
    "MC88",
    "MC88-3S USB MIDI",
    NULL,
};

static uint16_t _desc_str[32 + 1];

uint16_t const * tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;
    size_t chr_count;

    if (index == STRID_LANGID) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * 1 + 2);
        return _desc_str;
    }

    if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) {
        return NULL;
    }

    const char *str = string_desc_arr[index];
    chr_count = strlen(str);
    if (chr_count > 32) {
        chr_count = 32;
    }

    for (size_t i = 0; i < chr_count; i++) {
        _desc_str[1 + i] = str[i];
    }
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return _desc_str;
}
