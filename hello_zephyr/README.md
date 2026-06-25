# Hello Zephyr — Fundamentals Demo

A minimal "Hello World" application that demonstrates the basic Zephyr project structure: three files, one build command, one serial output.

## What This Demo Does

- Prints `"Hello from Zephyr on rpi_pico!"` to the USB serial console
- Demonstrates the mandatory three-file Zephyr project structure
- Shows how `main()` integrates with the Zephyr kernel

## Prerequisites

- Docker container: `zephyrprojectrtos/zephyr-build:latest`
- Initialized west workspace (see master README Quick Start)
- Raspberry Pi Pico connected via USB

## Project Structure

```
demo/
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Kconfig selections
├── src/
│   └── main.c              # Application entry point
└── README.md               # This file
```

## How to Build

```bash
# Inside Docker container, from zephyr workspace root
cd /workdir/zephyr

west build -b rpi_pico /path/to/this/demo --pristine
```

Or if you placed this demo inside the Zephyr tree:

```bash
west build -b rpi_pico samples/hello_zephyr
```

## Build Output

Successful build produces:
```
build/
├── zephyr/
│   ├── zephyr.elf      # ELF binary with debug symbols
│   ├── zephyr.bin      # Raw binary
│   ├── zephyr.hex      # Intel HEX format
│   ├── zephyr.uf2      # UF2 for drag-and-drop flash
│   └── zephyr.map      # Linker map: see where every symbol lives
```

## How to Flash (UF2 Method — No Debug Probe Needed)

1. Hold `BOOTSEL` on the Pico while plugging in USB
2. It appears as a drive called `RPI-RP2`
3. Copy the UF2 file:
   ```bash
   cp build/zephyr/zephyr.uf2 /Volumes/RPI-RP2/   # macOS
   # Or drag and drop in Finder
   ```
4. The Pico reboots automatically and runs your firmware

## Expected Output (Serial Console)

Connect to the Pico's USB serial port:
```bash
# macOS
screen /dev/tty.usbmodem* 115200

# Linux
screen /dev/ttyACM0 115200
```

You should see:
```
*** Booting Zephyr OS build v3.x.x ***
Hello from Zephyr on rpi_pico!
```
