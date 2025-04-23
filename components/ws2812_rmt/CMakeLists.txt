idf_component_register(
    SRCS "ws2812_control.c" "ws2812_animations.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_common freertos
) 