# ESP32-S2 LED Controller Makefile

# Default port
PORT ?= /dev/cu.usbserial-230

# ESP-IDF path
IDF_PATH ?= $(HOME)/Projects/esp/esp-idf

# Default target
all: build

# Build the project
build:
	@echo "Building project..."
	@. ./idf_env.sh && idf.py build

# Flash the firmware
flash:
	@echo "Flashing firmware to port $(PORT)..."
	@. ./idf_env.sh && idf.py -p $(PORT) flash

# Flash the partition table
flash_partitions:
	@echo "Flashing partition table to port $(PORT)..."
	@. ./idf_env.sh && idf.py -p $(PORT) partition-table-flash && \
	echo "Verifying partition table..." && \
	. ./idf_env.sh && python $(IDF_PATH)/components/partition_table/parttool.py -p $(PORT) get_partition_info --partition-name=storage

# Flash the SPIFFS partition
flashfs: build flash_partitions
	@echo "Flashing SPIFFS partition to port $(if $(PORT),$(PORT),auto-detected)..."
	@echo "Checking if SPIFFS image exists..."
	@if [ -f "build/spiffs_image.bin" ]; then \
		echo "SPIFFS image found, size: $$(ls -lh build/spiffs_image.bin | awk '{print $$5}')"; \
		echo "Writing SPIFFS image to flash..."; \
		. ./idf_env.sh && \
		python -m esptool --chip esp32s2 $(if $(PORT),-p $(PORT),) -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 80m --flash_size 4MB 0x110000 build/spiffs_image.bin; \
	else \
		echo "Error: SPIFFS image not found at build/spiffs_image.bin"; \
		exit 1; \
	fi

# Monitor the serial output
monitor:
	@echo "Starting serial monitor on port $(PORT)..."
	@. ./idf_env.sh && idf.py -p $(PORT) monitor

# Clean build files
clean:
	@echo "Cleaning build files..."
	@. ./idf_env.sh && idf.py clean

# Full clean (including CMake files)
fullclean:
	@echo "Performing full clean..."
	@. ./idf_env.sh && idf.py fullclean

# Flash everything
flash-all: flash_partitions flash flashfs

.PHONY: all build flash flash_partitions flashfs monitor clean fullclean flash-all 