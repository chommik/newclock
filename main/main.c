/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/idf_additions.h"
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/spi_common.h>
#include <driver/spi_master.h>
#include <esp_attr.h>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/FreeRTOSConfig_arch.h>
#include <freertos/portmacro.h>
#include <freertos/projdefs.h>
#include <freertos/task.h>
#include <hal/gpio_types.h>
#include <hal/ledc_types.h>
#include <soc/clk_tree_defs.h>

#include <sdkconfig.h>

#include <stdint.h>
#include <stdio.h>
#include <sys/_timeval.h>
#include <sys/time.h>
#include <time.h>

#include "button_handling.h"
#include "st7920_cmds.h"
#include "tasks/task_brightness.h"
#include "tasks/temperature.h"
#include "u8g2.h"
#include "u8g2_adapter/u8g2_d_st7920_esp32.h"
#include "u8g2_font_maniacbold.h"
#include "ui/ui.h"

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

    ESP_LOGI(TAG, "Configured SPI using pins: MOSI = %d, SCK = %d, CS = %d, RESET = %d", CONFIG_LCD_SPI_MOSI_PIN,
        CONFIG_LCD_SPI_SCK_PIN, CONFIG_LCD_SPI_CS_PIN, CONFIG_LCD_GPIO_RESET_PIN);
}

static u8g2_t u8g2;

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

    // -----------------------------------------------------

    // configure DS18B20
    ret =
        xTaskCreate(task_temperature_sensor, "temp_sensor", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY, NULL);
    assert(ret == pdPASS);

    ret = xTaskCreate(task_brightness, "brightness", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY, NULL);
    assert(ret == pdPASS);

    // -----------------------------------------------------

    setup_button_handler_queue();

    // -----------------------------------------------------

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();

    struct timeval best_starting_date = {
        .tv_sec = 1112470620,
    };
    settimeofday(&best_starting_date, NULL);

    // -----------------------------------------------------

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

    u8g2_SetupBuffer_st7920_128x64_esp32(&u8g2, &u8g2_cb_r0, &display);
    // u8g2_SetFont(&u8g2, u8g2_font_helvB18_tr);

    enum ui_state_t current_state = UI_STATE_INIT_SCREEN;
    enum ui_state_t next_state = UI_STATE_REDRAW;

    // struct ui_screen_t *current_screen = &ui_main_screen;
    // struct ui_screen_t *next_screen = &ui_main_screen;

    struct ui_screen_t *current_screen = &ui_config_screen;
    struct ui_screen_t *next_screen = &ui_config_screen;

    TickType_t last_refresh_time = xTaskGetTickCount();

    TickType_t t0 = xTaskGetTickCount();
    TickType_t t1 = xTaskGetTickCount();

    while (true) {
        switch (current_state) {
            case UI_STATE_INIT_SCREEN: next_state = current_screen->init(&u8g2); break;

            case UI_STATE_REDRAW:
                st7920_fb_clear(&display);
                u8g2_ClearBuffer(&u8g2);

                next_state = current_screen->draw(&u8g2);

                u8g2_SendBuffer(&u8g2);
                st7920_fb_redraw(&display);
                last_refresh_time = xTaskGetTickCount();
                break;

            case UI_STATE_PROCESS_EVENTS: next_state = current_screen->process_events(); break;

            case UI_STATE_DEINIT_SCREEN:
                next_screen = current_screen->deinit();
                next_state = UI_STATE_INIT_SCREEN;
                break;

            case UI_STATE_IDLE:
                TickType_t ui_task_wait_time = current_screen->wait_time();
                TickType_t now = xTaskGetTickCount();
                TickType_t time_to_refresh = ui_task_wait_time - (now - last_refresh_time);

                struct btn_event_t event;
                ret = xQueuePeek(g_button_event_queue, &event, time_to_refresh);
                if (ret == pdPASS)
                    next_state = UI_STATE_PROCESS_EVENTS;
                else
                    next_state = UI_STATE_REDRAW;
                break;
        }

        if (current_state != next_state) {
            t1 = xTaskGetTickCount();
            float state_change_time = 1000 * ((float)t1 - (float)t0) / configTICK_RATE_HZ;
            ESP_LOGD(TAG, "State change: %s -> %s (after %0.1fms)", ui_state_str(current_state),
                ui_state_str(next_state), state_change_time);
            t0 = xTaskGetTickCount();

            current_state = next_state;
        }

        if (current_screen != next_screen) {
            ESP_LOGD(TAG, "Screen change: %s -> %s", current_screen->name, next_screen->name);
            current_screen = next_screen;
        }
    }

    // for (uint32_t counter = 0;; counter++) {
    // ------------------- PROCESS EVENTS BEGIN

    // ------------------- PROCESS EVENTS END

    // ------------------- REDRAW BEGIN

    // ------------------- REDRAW END

    // for (uint8_t symbol = 0x40; symbol < 0x50; symbol++) {
    //     st7920_cmd(g_lcd_spi, 0, 1, symbol + counter);
    // }

    // st7920_data(g_lcd_spi, pixels, 8);

    // st7920_fb_put_pixel(&display, counter % 128, counter / 128);

    // for (int i = 0; i < 1024; i++) {
    //     display.framebuffer[i] = (counter & 1) ? 0xAA : 0x55;
    // }

    // st7920_fb_put_pixel(&display, counter / 64, counter % 64);

    // uint32_t t0 = xTaskGetTickCount();
    // st7920_fb_redraw(&display);
    // uint32_t t1 = xTaskGetTickCount();

    // float time_used = 1000 * ((float)t1 - (float)t0) / configTICK_RATE_HZ;

    // if (counter % 64 == 0)
    //     ESP_LOGI(TAG, "Time used: %0.2fms", time_used);

    // for (uint8_t i = 0; i < 128; i++) {
    //     st7920_fb_put_pixel(&display, i, 0);
    //     st7920_fb_redraw(&display);

    //     vTaskDelay(pdMS_TO_TICKS(10));
    // }
    // st7920_fb_redraw(&display);
    // }
}
