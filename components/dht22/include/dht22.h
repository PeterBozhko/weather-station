#pragma once
#include <driver/gpio.h>
#include <esp_log.h>

esp_err_t dht22_read(gpio_num_t pin, float *temperature, float *humidity);
