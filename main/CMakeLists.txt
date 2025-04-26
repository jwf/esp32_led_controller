idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_wifi esp_http_server mdns spiffs json ws2812_rmt
)

# Create a custom target to copy the HTML file to the SPIFFS image
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/spiffs_image.bin
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/spiffs
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/data/index.html ${CMAKE_BINARY_DIR}/spiffs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/data/favicon.ico ${CMAKE_BINARY_DIR}/spiffs/
    COMMAND ${IDF_PATH}/components/spiffs/spiffsgen.py 0x100000 ${CMAKE_BINARY_DIR}/spiffs ${CMAKE_BINARY_DIR}/spiffs_image.bin
    DEPENDS ${CMAKE_SOURCE_DIR}/data/index.html
    COMMENT "Generating SPIFFS image with HTML file"
)

# Add the SPIFFS image as a custom target
add_custom_target(spiffs_image ALL DEPENDS ${CMAKE_BINARY_DIR}/spiffs_image.bin)

# Add the SPIFFS image to the list of files to flash
set(FLASH_FILES "${FLASH_FILES} ${CMAKE_BINARY_DIR}/spiffs_image.bin@0x110000" PARENT_SCOPE) 