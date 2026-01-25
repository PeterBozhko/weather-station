#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "dht22.h"

#define DHT22_GPIO 23

void app_main(void)
{
    float temperature, humidity = 0;
    while (true){
        if (dht22_read(DHT22_GPIO, &temperature, &humidity) == ESP_OK)
            ESP_LOGI("main", "Temperature = %0.1f, Humidity = %0.1f", temperature, humidity);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
