#include "dht22.h"
#include <driver/gpio.h>
#include <rom/ets_sys.h>
#include <esp_log.h>

static int dht_wait_level(gpio_num_t pin, int level, uint32_t timeout_us)
{
    while (gpio_get_level(pin) == level) {
        if (!timeout_us--) return -1;
        ets_delay_us(1);
    }
    return 0;
}

esp_err_t dht22_read(gpio_num_t pin, float *temperature, float *humidity)
{
    uint8_t data[5] = {0};

    // Start signal
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
    ets_delay_us(2000);              // 2 ms low (0.8-20 ms)
    gpio_set_level(pin, 1);
    ets_delay_us(40);                // 40 us high (20-200 us)
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    // Response
    if (dht_wait_level(pin, 0, 80) < 0) return ESP_ERR_TIMEOUT;
    if (dht_wait_level(pin, 1, 80) < 0) return ESP_ERR_TIMEOUT;

    // Read 40 bits
    for (int i = 0; i < 40; i++) {

        // wait for low to high edge
        if (dht_wait_level(pin, 0, 60) < 0) return ESP_ERR_TIMEOUT;

        // measure high pulse
        uint32_t t = 0;
        while (gpio_get_level(pin)) {
			if (++t > 100) return ESP_ERR_TIMEOUT;
            ets_delay_us(1);
        }

        int byte = i / 8;
        data[byte] <<= 1;
        if (t > 40)            // >40 us means bit = 1
            data[byte] |= 1;
    }
	
    // checksum
    uint8_t sum = data[0] + data[1] + data[2] + data[3];
    if ((sum & 0xFF) != data[4]) {
        return ESP_ERR_INVALID_CRC;
    }
	
    uint16_t raw_h = (data[0] << 8) | data[1];
    uint16_t raw_t = (data[2] << 8) | data[3];

    *humidity = raw_h / 10.0f;

    if (raw_t & 0x8000) {
        raw_t &= 0x7FFF;
        *temperature = -((float)raw_t / 10.0f);
    } else {
        *temperature = raw_t / 10.0f;
    }

    return ESP_OK;
}
