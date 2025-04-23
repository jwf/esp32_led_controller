#pragma once

// LED strip configuration
#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us
#define RMT_LED_STRIP_GPIO_NUM      17
#define RMT_LED_STRIP_MEM_BLOCK_SYMBOLS 64
#define RMT_LED_STRIP_TRANS_QUEUE_DEPTH 4
#define RMT_LED_STRIP_CHASE_SPEED_MS 10

// WS2812 timing constants (in microseconds)
#define WS2812_T0H 0.3f
#define WS2812_T0L 0.9f
#define WS2812_T1H 0.9f
#define WS2812_T1L 0.3f
#define WS2812_RESET_DURATION 50.0f

// HSV to RGB conversion constants
#define HSV_RGB_MAX 2.55f
#define HSV_RGB_MIN_DIVISOR 100.0f
#define HSV_HUE_MAX 360
#define HSV_SATURATION_MAX 100
#define HSV_VALUE_MAX 100 