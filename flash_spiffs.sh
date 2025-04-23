#!/bin/bash

# Get the port from command line or use default
PORT=${1:-/dev/ttyUSB0}

# ESP-IDF path
IDF_PATH=${IDF_PATH:-$HOME/Projects/esp/esp-idf}

# Source the ESP-IDF environment
. ./idf_env.sh

echo "Flashing SPIFFS partition to $PORT..."

# First, flash the partition table
echo "Flashing partition table..."
idf.py -p $PORT partition-table-flash

# Wait a moment for the device to reset
sleep 2

# Check if the storage partition exists
echo "Checking if storage partition exists..."
if python $IDF_PATH/components/partition_table/parttool.py -p $PORT get_partition_info --partition-name=storage; then
    echo "Storage partition found, flashing SPIFFS image..."
    
    # Check if the SPIFFS image exists
    if [ ! -f "build/spiffs_image.bin" ]; then
        echo "Error: SPIFFS image not found at build/spiffs_image.bin"
        echo "Make sure the build target completed successfully."
        exit 1
    fi
    
    # Flash the SPIFFS image
    python $IDF_PATH/components/partition_table/parttool.py -p $PORT write_partition --partition-name=storage --input build/spiffs_image.bin
    echo "SPIFFS partition flashed successfully"
else
    echo "Error: Storage partition not found"
    echo "Checking partition table..."
    python $IDF_PATH/components/partition_table/parttool.py -p $PORT get_partition_table
    echo "Please make sure your partition table includes a 'storage' partition of type 'spiffs'"
    exit 1
fi 