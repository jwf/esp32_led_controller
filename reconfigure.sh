#!/bin/bash

# Source the ESP-IDF environment
. ./idf_env.sh

echo "Cleaning build directory..."
idf.py fullclean

echo "Reconfiguring project with custom partition table..."
idf.py reconfigure

echo "Building project..."
idf.py build

echo "Done! Now you can flash the partition table and SPIFFS image."
echo "Run: make flash_partitions"
echo "Then: make flashfs" 