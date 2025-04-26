#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "mdns.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "cJSON.h"
#include "ws2812_control.h"
#include "ws2812_animations.h"

#define WIFI_SSID      "groucho"
#define WIFI_PASS      "frankfamilywn"
#define MDNS_HOSTNAME  "zoelights"

static const char *TAG = "led_controller";
static httpd_handle_t server = NULL;

static uint8_t led_strip_pixels[NUM_LEDS * 3];

// Root page handler
static esp_err_t root_handler(httpd_req_t *req)
{
    // Open the HTML file
    FILE* file = fopen("/spiffs/index.html", "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open index.html");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to load page");
        return ESP_FAIL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file content
    char* html_content = malloc(file_size + 1);
    if (html_content == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for HTML content");
        fclose(file);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to load page");
        return ESP_FAIL;
    }

    // Read the file content
    size_t bytes_read = fread(html_content, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != file_size) {
        ESP_LOGE(TAG, "Failed to read the entire file");
        free(html_content);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to load page");
        return ESP_FAIL;
    }
    
    html_content[file_size] = '\0';

    // Send the HTML content
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_content, strlen(html_content));
    
    free(html_content);
    return ESP_OK;
}

// Root URI configuration
static httpd_uri_t root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_handler,
    .user_ctx  = NULL
};

// Function to set LED color and brightness
static void set_led_color(int index, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
    ESP_LOGI(TAG, "Received request to set LED %d to brightness: %d, color: (%d, %d, %d)", index, brightness, r, g, b);

    if (index < 0 || index >= NUM_LEDS) return;

    // Apply brightness
    led_strip_pixels[index * 3 + 0] = (uint8_t)(((float)g * (float)brightness) / 100.f);
    led_strip_pixels[index * 3 + 1] = (uint8_t)(((float)r * (float)brightness) / 100.f);
    led_strip_pixels[index * 3 + 2] = (uint8_t)(((float)b * (float)brightness) / 100.f);

    ESP_LOGI(TAG, "Setting LED %d to %d, %d, %d", index, r, g, b);
    // Update all LEDs
    led_strip_set(led_strip_pixels);
}

// HTTP GET handler
static esp_err_t led_get_handler(httpd_req_t *req)
{
    char resp[512];
    char *ptr = resp;
    ptr += sprintf(ptr, "[");
    for (int i = 0; i < NUM_LEDS; i++) {
        if (i > 0) ptr += sprintf(ptr, ",");
        ptr += sprintf(ptr, "{\"g\":%u,\"r\":%u,\"b\":%u,\"brightness\":100}", 
                      led_strip_pixels[i * 3 + 0], 
                      led_strip_pixels[i * 3 + 1], 
                      led_strip_pixels[i * 3 + 2]);
    }
    ptr += sprintf(ptr, "]");
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

// HTTP POST handler
static esp_err_t led_post_handler(httpd_req_t *req)
{
    char content[100];
    size_t recv_size = req->content_len;
    if (recv_size > sizeof(content) - 1) {
        recv_size = sizeof(content) - 1;
    }

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        return ESP_FAIL;
    }
    content[recv_size] = '\0';

    // Parse JSON using cJSON
    cJSON *root = cJSON_Parse(content);
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return ESP_FAIL;
    }

    // Get values from JSON
    int index = -1;
    int r = 0, g = 0, b = 0;
    int brightness = 100;

    cJSON *index_json = cJSON_GetObjectItem(root, "index");
    cJSON *r_json = cJSON_GetObjectItem(root, "r");
    cJSON *g_json = cJSON_GetObjectItem(root, "g");
    cJSON *b_json = cJSON_GetObjectItem(root, "b");
    cJSON *brightness_json = cJSON_GetObjectItem(root, "brightness");

    if (index_json) index = index_json->valueint;
    if (r_json) r = r_json->valueint;
    if (g_json) g = g_json->valueint;
    if (b_json) b = b_json->valueint;
    if (brightness_json) brightness = brightness_json->valueint;

    ESP_LOGI(TAG, "Parsed values - index: %d, r: %d, g: %d, b: %d, brightness: %d", 
             index, r, g, b, brightness);

    // Clean up
    cJSON_Delete(root);

    if (index >= 0 && index < NUM_LEDS) {
        set_led_color(index, r, g, b, brightness);
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", strlen("{\"status\":\"ok\"}"));
    return ESP_OK;
}

// HTTP server configuration
static httpd_uri_t led_get = {
    .uri       = "/api/led",
    .method    = HTTP_GET,
    .handler   = led_get_handler,
    .user_ctx  = NULL
};

static httpd_uri_t led_post = {
    .uri       = "/api/led",
    .method    = HTTP_POST,
    .handler   = led_post_handler,
    .user_ctx  = NULL
};

// Animation API handler
static esp_err_t animation_api_handler(httpd_req_t *req)
{
    char content[100];
    size_t recv_size = req->content_len;
    if (recv_size > sizeof(content) - 1) {
        recv_size = sizeof(content) - 1;
    }

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        return ESP_FAIL;
    }
    content[recv_size] = '\0';

    // Parse JSON
    cJSON *root = cJSON_Parse(content);
    if (!root) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    // Get animation type
    cJSON *type_json = cJSON_GetObjectItem(root, "type");
    if (!type_json) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing animation type");
        return ESP_FAIL;
    }

    animation_config_t config = {0};
    config.type = (animation_type_t)type_json->valueint;
    
    // Get optional parameters
    cJSON *speed_json = cJSON_GetObjectItem(root, "speed");
    if (speed_json) config.speed = speed_json->valueint;
    else config.speed = 50;

    cJSON *brightness_json = cJSON_GetObjectItem(root, "brightness");
    if (brightness_json) config.brightness = brightness_json->valueint;
    else config.brightness = 100;

    cJSON *color_json = cJSON_GetObjectItem(root, "color");
    if (color_json) {
        cJSON *r_json = cJSON_GetObjectItem(color_json, "r");
        cJSON *g_json = cJSON_GetObjectItem(color_json, "g");
        cJSON *b_json = cJSON_GetObjectItem(color_json, "b");
        if (r_json) config.r = r_json->valueint;
        if (g_json) config.g = g_json->valueint;
        if (b_json) config.b = b_json->valueint;
    } else {
        config.r = 255;
        config.g = 255;
        config.b = 255;
    }

    cJSON_Delete(root);

    // Start animation
    esp_err_t err = animation_start(&config);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to start animation");
        return ESP_FAIL;
    }

    httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    return ESP_OK;
}

// Add animation URI configuration
static httpd_uri_t animation_api = {
    .uri       = "/api/animation",
    .method    = HTTP_POST,
    .handler   = animation_api_handler,
    .user_ctx  = NULL
};

// Initialize mDNS service with error handling
static bool init_mdns(void)
{
    esp_err_t err = mdns_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mDNS init failed: %d", err);
        return false;
    }

    err = mdns_hostname_set(MDNS_HOSTNAME);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mDNS hostname set failed: %d", err);
        mdns_free();
        return false;
    }

    err = mdns_instance_name_set("Zoe's LED Controller");
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mDNS instance name set failed: %d", err);
        mdns_free();
        return false;
    }

    err = mdns_service_add("zoelights", "_http", "_tcp", 80, NULL, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mDNS service add failed: %d", err);
        mdns_free();
        return false;
    }

    ESP_LOGI(TAG, "mDNS initialized successfully");
    return true;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        if (!init_mdns()) {
            ESP_LOGW(TAG, "mDNS initialization failed. Device will only be accessible via IP");
        }
    }
}

// Initialize WiFi
static void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

// Initialize SPIFFS
static esp_err_t init_spiffs(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");
    
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_INVALID_STATE) {
            ESP_LOGI(TAG, "SPIFFS already mounted");
        } else {
            ESP_LOGE(TAG, "Failed to mount SPIFFS (%s)", esp_err_to_name(ret));
            return ret;
        }
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    
    // Copy the HTML file to SPIFFS if it doesn't exist
    struct stat st;
    if (stat("/spiffs/index.html", &st) != 0) {
        ESP_LOGI(TAG, "HTML file not found, copying from flash");
        // Here we would copy the file from flash to SPIFFS
        // For now, we'll just log an error
        ESP_LOGE(TAG, "HTML file not found in SPIFFS and copying from flash is not implemented");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

// Start HTTP server
static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &led_get);
        httpd_register_uri_handler(server, &led_post);
        httpd_register_uri_handler(server, &animation_api);
        return server;
    }
    return NULL;
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize SPIFFS
    ESP_ERROR_CHECK(init_spiffs());

    // Initialize LED strip with default configuration
    ESP_ERROR_CHECK(led_strip_init(NULL));

    // Initialize animations
    ESP_ERROR_CHECK(animation_init());

    // Start with no animation
    ESP_ERROR_CHECK(animation_stop());

    // Initialize WiFi
    wifi_init_sta();

    // Start HTTP server
    start_webserver();
} 