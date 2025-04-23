cmake_minimum_required(VERSION 3.5)

# Set the partition table file
set(PARTITION_TABLE_CSV ${CMAKE_CURRENT_LIST_DIR}/partitions.csv)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(esp32_led_controller)

# Add custom flash target that uses IDF's flash command
add_custom_target(flash-all
    COMMAND ${CMAKE_COMMAND} -E echo "Flashing all partitions..."
    COMMAND cd ${CMAKE_SOURCE_DIR} && ${IDF_PATH}/tools/idf.py flash --extra-args "0x110000 ${CMAKE_BINARY_DIR}/spiffs_image.bin"
    DEPENDS ${PROJECT_NAME}.bin spiffs_image
    COMMENT "Flashing all partitions"
) 