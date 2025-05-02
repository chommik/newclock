/*
    Newclock
    tasks/task_brightness.c

    Copyright (c) 2025 Rafal Macyszyn

    SPDX-License-Identifier: BSD-3-Clause
*/

#include "task_brightness.h"

#include "esp_err.h"
#include "hal/adc_types.h"
#include <driver/ledc.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <soc/soc_caps.h>

#include <stdint.h>

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_OUTPUT_IO 22

#define _STRINGIFY(x) #x
#define STRINGIFY(s) _STRINGIFY(s)

#define CLAMP(v, l, h) ((v) > (h) ? (h) : ((v) < (l) ? (l) : (v)))

#define LIGHT_SENSOR_GPIO_PIN 4

#define LIGHT_SENSOR_ADC_UNIT ADC_UNIT_1
#define LIGHT_SENSOR_ADC_CHANNEL ADC_CHANNEL_0
#define LIGHT_SENSOR_ADC_ATTEN ADC_ATTEN_DB_12

#define LIGHT_SENSOR_DARK_MV 150
#define LIGHT_SENSOR_LIGHT_MV 2700

#define LIGHT_SENSOR_BRIGHTNESS_MIN 1
#define LIGHT_SENSOR_BRIGHTNESS_MAX 29

#ifndef ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
#error "Curve Fitting ADC calibration scheme not supported"
#endif

QueueHandle_t g_brighness_msg_queue;

static esp_err_t ret;

static adc_oneshot_unit_handle_t g_adc_handle;
static adc_cali_handle_t g_adc_cali_handle;

static bool g_is_auto_brightness = false;
static uint8_t g_static_brightness_level = 7;
static uint8_t g_light_sensor_brightness = 0;

#define LIGHT_SENSOR_READING_BUFFER_LEN 32
static int g_light_sensor_readings[LIGHT_SENSOR_READING_BUFFER_LEN] = {0};
static size_t g_light_sensor_reading_buffer_idx = 0;

static const char *TAG = "task_brightness";

static void configure_display_backlight(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_TIMER_5_BIT,
        .timer_num = LEDC_TIMER,
        .freq_hz = 60 * 1000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ret = ledc_timer_config(&ledc_timer);
    ESP_ERROR_CHECK(ret);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO,
        .duty = 0,
        .hpoint = 0,
    };
    ret = ledc_channel_config(&ledc_channel);
    ESP_ERROR_CHECK(ret);
}

static void configure_adc_light_sensor(void)
{
    adc_oneshot_unit_init_cfg_t adc_init_config = {
        .unit_id = LIGHT_SENSOR_ADC_UNIT,
    };
    adc_oneshot_chan_cfg_t adc_chan_cfg = {
        .atten = LIGHT_SENSOR_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_init_config, &g_adc_handle));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(g_adc_handle, LIGHT_SENSOR_ADC_CHANNEL, &adc_chan_cfg));

    ESP_LOGD(TAG, "Attempting Line Fitting calibration");

    adc_cali_line_fitting_config_t adc_cali_config = {
        .unit_id = LIGHT_SENSOR_ADC_UNIT,
        .atten = LIGHT_SENSOR_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&adc_cali_config, &g_adc_cali_handle));
}

void brightness_get_current_setting(brightness_setting_t *setting)
{
    setting->auto_brightness = g_is_auto_brightness;
    setting->static_brightness = g_static_brightness_level;
    setting->light_sensor_brightness = g_light_sensor_brightness;
}

static uint8_t brightness_get_level_from_sensor(int sensor_reading_mv)
{
    if (sensor_reading_mv <= LIGHT_SENSOR_DARK_MV)
        return LIGHT_SENSOR_BRIGHTNESS_MIN;
    if (sensor_reading_mv >= LIGHT_SENSOR_LIGHT_MV)
        return LIGHT_SENSOR_BRIGHTNESS_MAX;

    uint32_t range = LIGHT_SENSOR_LIGHT_MV - LIGHT_SENSOR_DARK_MV;
    uint32_t value = sensor_reading_mv - LIGHT_SENSOR_DARK_MV;
    uint32_t level = (value * (LIGHT_SENSOR_BRIGHTNESS_MAX - LIGHT_SENSOR_BRIGHTNESS_MIN)) / range;
    uint8_t brightness_without_offset = LIGHT_SENSOR_BRIGHTNESS_MIN + level;

    return brightness_without_offset;
}

static int adc_get_voltage(void)
{
    int adc_out;
    int adc_voltage_out;

    ret = adc_oneshot_read(g_adc_handle, LIGHT_SENSOR_ADC_CHANNEL, &adc_out);
    ESP_ERROR_CHECK(ret);

    ret = adc_cali_raw_to_voltage(g_adc_cali_handle, adc_out, &adc_voltage_out);
    ESP_ERROR_CHECK(ret);

    ESP_LOGV(TAG,
        "ADC: " STRINGIFY(LIGHT_SENSOR_ADC_UNIT) " " STRINGIFY(
            LIGHT_SENSOR_ADC_CHANNEL) ", raw data = %d, voltage = %dmV",
        adc_out, adc_voltage_out);

    return adc_voltage_out;
}

static void brightness_update_setting(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(
        LEDC_MODE, LEDC_CHANNEL, g_is_auto_brightness ? g_light_sensor_brightness : g_static_brightness_level));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

static int brightness_average_value(void)
{
    size_t idx = g_light_sensor_reading_buffer_idx;

    float alpha = 0.4;
    float ema = g_light_sensor_readings[idx];

    for (size_t i = 0; i < LIGHT_SENSOR_READING_BUFFER_LEN; i++) {
        ema = alpha * g_light_sensor_readings[idx] + (1 - alpha) * ema;
        if (idx == 0)
            idx = LIGHT_SENSOR_READING_BUFFER_LEN - 1;
        else
            idx--;
    }

    return ema;
}

static void brightness_push_measurement(int adc_voltage_mv)
{
    g_light_sensor_readings[g_light_sensor_reading_buffer_idx++] = adc_voltage_mv;
    if (g_light_sensor_reading_buffer_idx >= LIGHT_SENSOR_READING_BUFFER_LEN)
        g_light_sensor_reading_buffer_idx = 0;
}

void task_brightness(void *_unused)
{
    g_brighness_msg_queue = xQueueCreate(16, sizeof(brightness_msg_t));

    configure_display_backlight();
    configure_adc_light_sensor();

    brightness_update_setting();

    brightness_msg_t brightness_msg;
    uint8_t previous_brightness = 255;
    int sensor_reading_mv;

    while (true) {
        ret = xQueueReceive(g_brighness_msg_queue, &brightness_msg, BRIGHTNESS_LIGHT_SENSOR_REFRESH_DELAY);

        if (g_is_auto_brightness)
            previous_brightness = g_light_sensor_brightness;
        else
            previous_brightness = g_static_brightness_level;

        if (ret == pdPASS) {
            // Got a message
            switch (brightness_msg.msg_type) {
                case MSG_BRIGHTNESS_SET: g_static_brightness_level = (uint8_t)brightness_msg.msg_arg; break;
                case MSG_BRIGHTNESS_CHANGE: g_static_brightness_level += (uint8_t)brightness_msg.msg_arg; break;
                case MSG_BRIGHTNESS_AUTO_SET: g_is_auto_brightness = (bool)brightness_msg.msg_arg; break;
                case MSG_BRIGHTNESS_AUTO_TOGGLE: g_is_auto_brightness = !g_is_auto_brightness; break;
            }
        } else if (ret == errQUEUE_EMPTY) {
            // Timeout, read the ADC
            sensor_reading_mv = adc_get_voltage();
            brightness_push_measurement(sensor_reading_mv);
            g_light_sensor_brightness = brightness_get_level_from_sensor(brightness_average_value());
        }

        if ((g_is_auto_brightness && g_light_sensor_brightness != previous_brightness) ||
            (!g_is_auto_brightness && g_static_brightness_level != previous_brightness))
            brightness_update_setting();
    }
}