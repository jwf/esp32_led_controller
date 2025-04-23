#!/bin/bash

# ESP-IDF path
IDF_PATH=${IDF_PATH:-$HOME/Projects/esp/esp-idf}

# Check if IDF_PATH exists
if [ ! -d "$IDF_PATH" ]; then
    echo "Error: ESP-IDF path not found at $IDF_PATH"
    echo "Please set the IDF_PATH environment variable to point to your ESP-IDF installation"
    exit 1
fi

# Source the ESP-IDF environment
. "$IDF_PATH/export.sh"

# Print confirmation
echo "ESP-IDF environment set up successfully"
echo "IDF_PATH: $IDF_PATH"
echo "PATH includes ESP-IDF tools: $(which idf.py)"

# Execute the command passed as arguments
if [ $# -gt 0 ]; then
    exec "$@"
fi 