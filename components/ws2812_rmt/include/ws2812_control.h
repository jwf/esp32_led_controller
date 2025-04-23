#pragma once

#include <stdint.h>
#include "driver/rmt_encoder.h"
#include "esp_err.h"
#include "ws2812_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Number of LEDs in the strip
 */
#define NUM_LEDS 5

/**
 * @brief LED strip configuration structure
 */
typedef struct {
    uint32_t resolution_hz;    /*!< RMT resolution in Hz */
    uint8_t gpio_num;          /*!< GPIO number for LED data line */
    uint32_t mem_block_symbols;/*!< Number of RMT memory block symbols */
    uint32_t trans_queue_depth;/*!< RMT transaction queue depth */
} led_strip_config_t;

/**
 * @brief LED strip state structure
 */
typedef struct {
    uint8_t leds[NUM_LEDS * 3]; /*!< Array of LED RGB values (GRB format) */
} led_strip_state_t;

/**
 * @brief LED strip encoder configuration structure
 */
typedef struct {
    uint32_t resolution; /*!< Encoder resolution, in Hz */
} led_strip_encoder_config_t;

/**
 * @brief Convert HSV color to RGB
 * 
 * @param h Hue value (0-359)
 * @param s Saturation value (0-100)
 * @param v Value/brightness (0-100)
 * @param[out] r Red component (0-255)
 * @param[out] g Green component (0-255)
 * @param[out] b Blue component (0-255)
 */
void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b);

/**
 * @brief Initialize the LED strip
 * 
 * @param config LED strip configuration (NULL for default values)
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t led_strip_init(const led_strip_config_t *config);

/**
 * @brief Set LED strip colors
 * 
 * @param led_strip_state Pointer to LED state array (GRB format)
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t led_strip_set(uint8_t *led_strip_state);

#ifdef __cplusplus
}
#endif