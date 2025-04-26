#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "ws2812_animations.h"
#include "ws2812_control.h"

static const char *TAG = "led_animations";

static TaskHandle_t animation_task_handle = NULL;
static QueueHandle_t animation_queue = NULL;
static animation_config_t current_config = {0};
static uint8_t led_buffer[NUM_LEDS * 3] = {0};

// Helper function for smooth sine wave
static float smooth_sin(float x) {
    return (sin(x) + 1.0f) / 2.0f;
}

// Animation task function
static void animation_task(void *pvParameters)
{
    uint32_t hue = 0;
    float brightness = 1.0f;
    bool increasing = true;
    uint8_t position = 0;
    float time = 0.0f;  // For time-based animations

    while (1) {
        switch (current_config.type) {
            case ANIMATION_RAINBOW:
                // Rainbow animation - cycle through all hues
                for (int i = 0; i < NUM_LEDS; i++) {
                    uint32_t led_hue = (hue + (i * 360 / NUM_LEDS)) % 360;
                    uint32_t r, g, b;
                    led_strip_hsv2rgb(led_hue, 100, current_config.brightness, &r, &g, &b);
                    led_buffer[i * 3] = g;
                    led_buffer[i * 3 + 1] = r;
                    led_buffer[i * 3 + 2] = b;
                }
                hue = (hue + 1) % 360;
                break;

            case ANIMATION_SOLID_COLOR:
                // Set all LEDs to the same color
                for (int i = 0; i < NUM_LEDS; i++) {
                    led_buffer[i * 3] = current_config.g;     // Green
                    led_buffer[i * 3 + 1] = current_config.r; // Red
                    led_buffer[i * 3 + 2] = current_config.b; // Blue
                }
                break;

            case ANIMATION_BREATHING:
                // Log values periodically (every 50 frames to avoid spam)
                static int frame_count = 0;
                if (frame_count++ % 50 == 0) {
                    ESP_LOGI(TAG, "Breathing animation - RGB: (%d, %d, %d), brightness: %.2f",
                            current_config.r, current_config.g, current_config.b, brightness);
                }
                
                // Breathing animation - fade in and out
                if (increasing) {
                    brightness += 0.01f;
                    if (brightness >= 1.0f) {
                        brightness = 1.0f;
                        increasing = false;
                    }
                } else {
                    brightness -= 0.01f;
                    if (brightness <= 0.0f) {
                        brightness = 0.0f;
                        increasing = true;
                    }
                }
                for (int i = 0; i < NUM_LEDS; i++) {
                    led_buffer[i * 3] = current_config.g * brightness;
                    led_buffer[i * 3 + 1] = current_config.r * brightness;
                    led_buffer[i * 3 + 2] = current_config.b * brightness;
                }
                break;

            case ANIMATION_CHASE:
                // Chase animation - moving dot
                memset(led_buffer, 0, sizeof(led_buffer));
                led_buffer[position * 3] = current_config.g;
                led_buffer[position * 3 + 1] = current_config.r;
                led_buffer[position * 3 + 2] = current_config.b;
                position = (position + 1) % NUM_LEDS;
                break;

            case ANIMATION_FIRE:
                // Fire animation - flickering orange/yellow
                for (int i = 0; i < NUM_LEDS; i++) {
                    uint8_t flicker = rand() % 55;
                    uint8_t r = 255;
                    uint8_t g = 50 + flicker;
                    uint8_t b = 0;
                    led_buffer[i * 3] = g;
                    led_buffer[i * 3 + 1] = r;
                    led_buffer[i * 3 + 2] = b;
                }
                break;

            case ANIMATION_LIGHTNING:
                // Lightning animation - random bright flashes
                if (rand() % 100 < 5) { // 5% chance of a flash
                    // Bright flash
                    for (int i = 0; i < NUM_LEDS; i++) {
                        uint8_t intensity = 200 + (rand() % 55); // Random intensity between 200-255
                        led_buffer[i * 3] = intensity;     // G
                        led_buffer[i * 3 + 1] = intensity; // R
                        led_buffer[i * 3 + 2] = 255;       // B (more blue for lightning effect)
                    }
                    // Short delay for the flash
                    vTaskDelay(pdMS_TO_TICKS(50));
                } else {
                    // Fade out
                    for (int i = 0; i < NUM_LEDS; i++) {
                        led_buffer[i * 3] = 0;     // G
                        led_buffer[i * 3 + 1] = 0; // R
                        led_buffer[i * 3 + 2] = 0; // B
                    }
                }
                break;

            case ANIMATION_OCEAN:
                // Ocean wave animation - gentle blue waves
                for (int i = 0; i < NUM_LEDS; i++) {
                    // Create a wave pattern with multiple frequencies
                    float wave1 = smooth_sin(time + i * 0.2f) * 0.5f;
                    float wave2 = smooth_sin(time * 0.7f + i * 0.1f) * 0.3f;
                    float wave3 = smooth_sin(time * 0.3f + i * 0.05f) * 0.2f;
                    float intensity = (wave1 + wave2 + wave3) * 0.7f;
                    
                    // Ocean blue color with varying intensity
                    led_buffer[i * 3] = 50 + (intensity * 50);     // G
                    led_buffer[i * 3 + 1] = 0;                     // R
                    led_buffer[i * 3 + 2] = 100 + (intensity * 100); // B
                }
                time += 0.05f; // Slow wave movement
                break;

            case ANIMATION_AURORA:
                // Aurora borealis effect - flowing green/purple waves
                for (int i = 0; i < NUM_LEDS; i++) {
                    // Create flowing aurora patterns
                    float pos = (float)i / NUM_LEDS;
                    float wave1 = smooth_sin(time + pos * 3.0f) * 0.5f;
                    float wave2 = smooth_sin(time * 0.7f + pos * 2.0f) * 0.3f;
                    float wave3 = smooth_sin(time * 0.3f + pos * 1.0f) * 0.2f;
                    float intensity = (wave1 + wave2 + wave3) * 0.8f;
                    
                    // Aurora colors (green and purple) with intensity modulation
                    float green = (0.7f + (wave1 * 0.3f)) * intensity;
                    float blue = (0.5f + (wave2 * 0.5f)) * intensity;
                    float red = (0.3f + (wave3 * 0.7f)) * intensity;
                    
                    led_buffer[i * 3] = green * 255;     // G
                    led_buffer[i * 3 + 1] = red * 255;   // R
                    led_buffer[i * 3 + 2] = blue * 255;  // B
                }
                time += 0.03f; // Slow aurora movement
                break;

            case ANIMATION_NONE:
            default:
                // No animation - keep LEDs off
                memset(led_buffer, 0, sizeof(led_buffer));
                break;
        }

        // Apply brightness
        for (int i = 0; i < NUM_LEDS * 3; i++) {
            led_buffer[i] = (led_buffer[i] * current_config.brightness) / 255;
        }

        // Update LEDs
        led_strip_set(led_buffer);

        // Wait for next frame
        vTaskDelay(pdMS_TO_TICKS(current_config.speed));
    }
}

esp_err_t animation_init(void)
{
    animation_queue = xQueueCreate(1, sizeof(animation_config_t));
    if (!animation_queue) {
        ESP_LOGE(TAG, "Failed to create animation queue");
        return ESP_ERR_NO_MEM;
    }

    // Initialize with no animation
    current_config.type = ANIMATION_NONE;
    current_config.speed = 50;
    current_config.brightness = 128;  // Default to 50% brightness in 0-255 range
    current_config.r = 255;
    current_config.g = 255;
    current_config.b = 255;

    return ESP_OK;
}

esp_err_t animation_start(const animation_config_t *config)
{
    if (!config || config->type >= ANIMATION_MAX) {
        return ESP_ERR_INVALID_ARG;
    }

    // Stop current animation if running
    if (animation_task_handle) {
        vTaskDelete(animation_task_handle);
        animation_task_handle = NULL;
    }

    // Update configuration
    memcpy(&current_config, config, sizeof(animation_config_t));
    
    // Log the RGB values
    ESP_LOGI(TAG, "Starting animation type %d with RGB: (%d, %d, %d), brightness: %d, speed: %d",
             current_config.type, current_config.r, current_config.g, current_config.b,
             current_config.brightness, current_config.speed);

    // Start new animation task
    BaseType_t ret = xTaskCreate(animation_task, "led_animation", 4096, NULL, 5, &animation_task_handle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create animation task");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

esp_err_t animation_stop(void)
{
    if (animation_task_handle) {
        vTaskDelete(animation_task_handle);
        animation_task_handle = NULL;
    }

    // Turn off LEDs
    memset(led_buffer, 0, sizeof(led_buffer));
    led_strip_set(led_buffer);

    current_config.type = ANIMATION_NONE;
    return ESP_OK;
}

esp_err_t animation_get_config(animation_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(config, &current_config, sizeof(animation_config_t));
    return ESP_OK;
} 