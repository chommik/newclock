/*
    Newclock
    tasks/temperature.c

    Copyright (c) 2025 Rafal Macyszyn

    SPDX-License-Identifier: BSD-3-Clause
*/

#include "ds18x20.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "esp_check.h"
#include "onewire.h"
#include "portmacro.h"
#include <math.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>

#include "temperature.h"

const char* TAG = "temperature";

onewire_addr_t onewire_addrs[MAX_SENSORS];
float sensor_temperatures[MAX_SENSORS];
size_t sensor_count;

static esp_err_t ret;

static void temperature_sensor_gpio_setup(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1 << ONEWIRE_GPIO_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

float get_temperature_for_sensor(onewire_addr_t sensor_addr)
{
    for (size_t i = 0; i < sensor_count; i++) {
        if (sensor_addr == onewire_addrs[i]) {
            return sensor_temperatures[i];
        }
    }
    return NAN;
}

void task_temperature_sensor(void *_unused)
{
    temperature_sensor_gpio_setup();
    ESP_LOGI(TAG, "Temperature sensor started");

    while(true) {
        ESP_LOGI(TAG, "reading temperature");

        ret = ds18x20_scan_devices(ONEWIRE_GPIO_PIN, onewire_addrs, MAX_SENSORS, &sensor_count);
        ESP_ERROR_CHECK(ret);

        if (sensor_count == 0) {
            ESP_LOGW(TAG, "No sensors found!");
            goto next;
        } else if (sensor_count >= MAX_SENSORS) {
            sensor_count = MAX_SENSORS;
        }

        for (size_t i = 0; i < sensor_count; i++) {
            ret = ds18x20_measure(ONEWIRE_GPIO_PIN, onewire_addrs[i], false);
            ESP_GOTO_ON_ERROR(ret, next, TAG, "measure failed for sensor %" PRIx64, onewire_addrs[i]);

            vTaskDelay(750 / portTICK_PERIOD_MS);

            ret = ds18b20_read_temperature(ONEWIRE_GPIO_PIN, onewire_addrs[i], &sensor_temperatures[i]);
            ESP_GOTO_ON_ERROR(ret, next, TAG, "read failed for sensor %" PRIx64, onewire_addrs[i]);
        }

        ret = ds18x20_measure_and_read_multi(ONEWIRE_GPIO_PIN, onewire_addrs, sensor_count, sensor_temperatures);
        ESP_GOTO_ON_ERROR(ret, next, TAG, "reading sensors failed");

        for (size_t i = 0; i < sensor_count; i++) {
            ESP_LOGI(TAG, "sensor %" PRIx64 ", temperature: %0.2f", onewire_addrs[i], sensor_temperatures[i]);
        }

    next:
        xTaskNotifyWait(pdFALSE, pdFALSE, NULL, 5000 / portTICK_PERIOD_MS);
    }
}