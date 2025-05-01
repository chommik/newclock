#pragma once

#include <freertos/FreeRTOS.h>

#include <stdint.h>

#define BRIGHTNESS_MIN_VALUE 0
#define BRIGHTNESS_MAX_VALUE (1 << 5)

#define BRIGHTNESS_LIGHT_SENSOR_REFRESH_DELAY (200 / portTICK_PERIOD_MS)

typedef enum
{
    MSG_BRIGHTNESS_SET,
    MSG_BRIGHTNESS_CHANGE,
    MSG_BRIGHTNESS_AUTO_SET,
    MSG_BRIGHTNESS_AUTO_TOGGLE,
} brightness_msg_type_t;

typedef struct
{
    brightness_msg_type_t msg_type;
    uint8_t msg_arg;
} brightness_msg_t;

typedef struct {
    bool auto_brightness;
    uint8_t light_sensor_brightness;
    uint8_t static_brightness;
} brightness_setting_t;

extern QueueHandle_t g_brighness_msg_queue;

void brightness_get_current_setting(brightness_setting_t *setting);

void task_brightness(void *_unused);