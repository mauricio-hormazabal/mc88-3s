# mc88-3s

## Introduction

`mc88-3s` is a firmware project for the Raspberry Pi Pico family that implements a MIDI controller featuring a keyboard matrix scanner with velocity estimation, pedal support, and UART MIDI output. It is optimized to read piano-style matrices, providing robust debouncing and accurate note velocity calculation for real-time MIDI output.

The project uses the Raspberry Pi Pico SDK and targets a Pico-compatible board. It integrates MIDI active sensing to keep connected MIDI hosts and devices synchronized.

## Features

- Scans a key matrix with debouncing
- Estimates velocity for dynamic MIDI note messages
- Processes pedal inputs and MIDI control change messages
- Sends MIDI data over UART at 31250 baud
- Supports MIDI active sensing
- Built using Raspberry Pi Pico SDK

## Repository Structure

- `CMakeLists.txt` - Build configuration for the Pico SDK
- `src/` - Implementation source code
  - `mc88-3s.c` - Main application loop and initialization
  - `debounce_matrix.c` - Debounce logic for matrix key scanning
  - `matrix_scan.c` - Matrix scanning routines
  - `midi_in.c` - MIDI input handling
  - `midi_out.c` - MIDI output handling
  - `midi_active_sense.c` - Active sense handling for MIDI
  - `midi_parser.c` - MIDI message parsing routines
  - `pedals.c` - Pedal input handling
  - `velocity_estimator.c` - Velocity estimation logic
- `include/` - Public headers for firmware modules

## Build Instructions

1. Install the Raspberry Pi Pico SDK and required toolchain.
2. Configure the project with CMake.
3. Build using `ninja`.

Example commands:

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

On successful build, the firmware output is generated in the `build/` directory and includes the `mc88-3s.uf2` file.

## Flashing

Flash the generated `.uf2` file to a Pico-compatible board using the standard USB bootloader method or your preferred flashing workflow.

## Hardware
<img width="4080" height="3072" alt="IMG_20250910_222124635" src="https://github.com/user-attachments/assets/ba522dfc-d61d-44a2-98f9-796dd1aabb6e" />

## Notes

- The default program name and version are set in `CMakeLists.txt`.
- USB stdio is enabled by default for diagnostic output.
- The firmware runs a 1 ms scan loop for the matrix and active sensing tasks.
- The current velocity estimation logic only supports 2 contacts per key, not 3, even though the matrix scanning architecture is built for 3-contact scanning.
- Some modules, such as `active_sense` and `pedals`, are implemented and functional but are not actively called in `main`.
- Hardware documentation is still pending, but any standard MIDI interface that can receive the Raspberry Pi Pico UART TX signal should work.

## Development

When extending the project, use the existing module structure to keep scanning, MIDI handling, and pedal logic separated. Update include headers and the executable target in `CMakeLists.txt` as new source files are added.
