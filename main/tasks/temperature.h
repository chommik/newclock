#pragma once

#include "onewire.h"

#define ONEWIRE_GPIO_PIN 13
#define MAX_SENSORS 8

extern onewire_addr_t onewire_addrs[MAX_SENSORS];
extern float sensor_temperatures[MAX_SENSORS];
extern size_t sensor_count;

void task_temperature_sensor(void *_unused);

float get_temperature_for_sensor(onewire_addr_t sensor_addr);