#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "ws2812_control.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Animation types
 */
typedef enum {
    ANIMATION_NONE = 0,
    ANIMATION_RAINBOW,
    ANIMATION_BREATHING,
    ANIMATION_CHASE,
    ANIMATION_FIRE,
    ANIMATION_LIGHTNING,
    ANIMATION_OCEAN,
    ANIMATION_AURORA,
    ANIMATION_SOLID_COLOR,
    ANIMATION_MAX
} animation_type_t;

/**
 * @brief Animation configuration structure
 */
typedef struct {
    animation_type_t type;     /*!< Type of animation */
    uint32_t speed;            /*!< Animation speed (ms between updates) */
    uint32_t brightness;       /*!< LED brightness (0-255) */
    uint8_t r, g, b;          /*!< Base color for animations that use a single color */
} animation_config_t;

/**
 * @brief Initialize the animation system
 * 
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t animation_init(void);

/**
 * @brief Start a new animation
 * 
 * @param config Animation configuration
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t animation_start(const animation_config_t *config);

/**
 * @brief Stop the current animation
 * 
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t animation_stop(void);

/**
 * @brief Get the current animation configuration
 * 
 * @param[out] config Pointer to store the current configuration
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t animation_get_config(animation_config_t *config);

#ifdef __cplusplus
}
#endif 