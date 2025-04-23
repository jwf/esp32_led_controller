# ESP32-S2 LED Controller

A web-based LED controller for ESP32-S2 using WS2812B LED strips.

## Features

- Control multiple LEDs individually
- Set color and brightness for each LED
- Web interface accessible via mDNS (zoelights.local)
- Real-time updates
- Mobile-friendly design

## Hardware Requirements

- ESP32-S2 development board
- WS2812B LED strip
- Power supply for the LED strip

## Connections

- Connect the LED strip data pin to GPIO 17
- Connect the LED strip ground to the ESP32-S2 ground
- Connect the LED strip power to an external power supply (5V)

## Building and Flashing

### Prerequisites

- ESP-IDF v5.0 or later
- Python 3.7 or later
- CMake 3.16 or later

### Using the Makefile (Recommended)

The project includes a Makefile that simplifies the build and flash process:

1. Build the project:
   ```
   make
   ```

2. Flash the firmware and SPIFFS partition:
   ```
   make install
   ```
   The Makefile will automatically detect the correct port. If you need to specify a port:
   ```
   make install PORT=/dev/ttyUSB0
   ```

3. Monitor the serial output:
   ```
   make monitor
   ```
   Or with a specific port:
   ```
   make monitor PORT=/dev/ttyUSB0
   ```

4. Clean build files:
   ```
   make clean
   ```

5. Build and flash everything in one command:
   ```
   make all
   ```
   Or with a specific port:
   ```
   make all PORT=/dev/ttyUSB0
   ```

### Manual Build Steps

If you prefer to use ESP-IDF commands directly:

1. Set up the ESP-IDF environment:
   ```
   . $HOME/Projects/esp/esp-idf/export.sh
   ```

2. Build the project:
   ```
   idf.py build
   ```

3. Flash the firmware:
   ```
   idf.py flash
   ```

4. Flash the SPIFFS partition (contains the web interface):
   ```
   ./flash_spiffs.sh
   ```

5. Monitor the serial output:
   ```
   idf.py monitor
   ```

## Usage

1. Power on the ESP32-S2 board
2. The device will connect to the WiFi network "groucho"
3. Access the web interface at http://zoelights.local or using the IP address shown in the serial monitor
4. Use the web interface to control the LEDs

## Project Structure

- `main/main.c` - Main application code
- `main/index.html` - Web interface HTML/CSS/JavaScript
- `partitions.csv` - Custom partition table with SPIFFS partition
- `flash_spiffs.sh` - Script to flash the SPIFFS partition
- `Makefile` - Simplified build and flash commands

## Customization

### Changing WiFi Credentials

Edit the following defines in `main/main.c`:
```c
#define WIFI_SSID      "your_ssid"
#define WIFI_PASS      "your_password"
```

### Changing the mDNS Hostname

Edit the following define in `main/main.c`:
```c
#define MDNS_HOSTNAME  "your_hostname"
```

### Modifying the Web Interface

Edit the `main/index.html` file to customize the web interface.

## License

This project is licensed under the MIT License - see the LICENSE file for details. 