/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdint.h>
#include "FreeRTOSConfig.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "portmacro.h"
#include "sdkconfig.h"

#include <st7920_cmds.h>

static const char *TAG = "example";

#define LCD_SPI_HOST SPI2_HOST

static esp_err_t ret;

static spi_device_handle_t g_lcd_spi;

#define FRAMEBUFFER_SIZE 1024
static DRAM_ATTR uint8_t g_framebuffer[FRAMEBUFFER_SIZE];


static void configure_spi(void) 
{
    spi_bus_config_t bus_config = {
        .mosi_io_num = CONFIG_LCD_SPI_MOSI_PIN,
        .miso_io_num = -1,
        .sclk_io_num = CONFIG_LCD_SPI_SCK_PIN,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1,
        .max_transfer_sz = 280, // whatever
    };

    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = 1 * 1000 * 1000, // 1 MHz
        .mode = 3,
        .spics_io_num = CONFIG_LCD_SPI_CS_PIN,
        .queue_size = 7, // whatever
        .flags = SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_POSITIVE_CS,
        .cs_ena_pretrans = 1,
        .cs_ena_posttrans = 2,
        //.pre_cb = some_callback_tbd,
    };

    ret = spi_bus_initialize(LCD_SPI_HOST, &bus_config, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(LCD_SPI_HOST, &dev_config, &g_lcd_spi);
    ESP_ERROR_CHECK(ret);

    gpio_config_t lcd_config = {
        .pin_bit_mask = (1ULL << CONFIG_LCD_GPIO_RESET_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = true,
    };

    gpio_config(&lcd_config);

    // Reset the display
    gpio_set_level(CONFIG_LCD_GPIO_RESET_PIN, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(CONFIG_LCD_GPIO_RESET_PIN, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "Configured SPI using pins: MOSI = %d, SCK = %d, CS = %d, RESET = %d",
        CONFIG_LCD_SPI_MOSI_PIN,
        CONFIG_LCD_SPI_SCK_PIN,
        CONFIG_LCD_SPI_CS_PIN,
        CONFIG_LCD_GPIO_RESET_PIN
    );
}


void app_main(void)
{
    configure_spi();

    st7920_cmd(g_lcd_spi, 0, 0, ST7920_BASIC_CMD_SET);
    vTaskDelay(pdMS_TO_TICKS(1));
    st7920_cmd(g_lcd_spi, 0, 0, ST7920_BASIC_CMD_SET);
    vTaskDelay(pdMS_TO_TICKS(1));
    st7920_cmd(g_lcd_spi, 0, 0, ST7920_CLEAR);
    vTaskDelay(pdMS_TO_TICKS(15));
    st7920_cmd(g_lcd_spi, 0, 0, ST7920_INCREMENT_ADDR);
    vTaskDelay(pdMS_TO_TICKS(1));
    st7920_cmd(g_lcd_spi, 0, 0, ST7920_DISPLAY_ON);
    vTaskDelay(pdMS_TO_TICKS(1));

    st7920_cmd(g_lcd_spi, 0, 0, ST7920_MODE_GRAPHICS);
    vTaskDelay(pdMS_TO_TICKS(1));
    st7920_cmd(g_lcd_spi, 0, 0, ST7920_MODE_GRAPHICS);
    vTaskDelay(pdMS_TO_TICKS(1));

    struct st7920_display display = {
        .max_x = 128,
        .max_y = 64,
        .spi_handle = g_lcd_spi,
        .framebuffer = g_framebuffer,
        .framebuffer_len = FRAMEBUFFER_SIZE,
    };

    ESP_LOGI(TAG, "Hello.");

    st7920_fb_clear(&display);
    st7920_fb_redraw(&display);
    

    for (uint32_t counter = 0;; counter++) {
        // for (uint8_t symbol = 0x40; symbol < 0x50; symbol++) {
        //     st7920_cmd(g_lcd_spi, 0, 1, symbol + counter);
        // }

        // st7920_data(g_lcd_spi, pixels, 8);

        // st7920_fb_put_pixel(&display, counter % 128, counter / 128);
        
        // for (int i = 0; i < 1024; i++) {
        //     display.framebuffer[i] = (counter & 1) ? 0xAA : 0x55;
        // }

        st7920_fb_put_pixel(&display, counter / 64, counter % 64);

        uint32_t t0 = xTaskGetTickCount();
        st7920_fb_redraw(&display);
        uint32_t t1 = xTaskGetTickCount();

        float time_used = 1000 * ((float)t1 - (float)t0) / configTICK_RATE_HZ;

        if (counter % 64 == 0)
            ESP_LOGI(TAG, "Time used: %0.2fms", time_used);

        // for (uint8_t i = 0; i < 128; i++) {
        //     st7920_fb_put_pixel(&display, i, 0);
        //     st7920_fb_redraw(&display);

        //     vTaskDelay(pdMS_TO_TICKS(10));
        // }
        // st7920_fb_redraw(&display);

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}
 