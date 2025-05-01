#pragma once

/*
    Low-level commands

    Note: transaction goes by sencding START_BYTE, then SPI_WORD in one CS sequence
*/

#include "driver/spi_master.h"

#define ST7920_BASIC_CMD_SET 0x30
#define ST7920_CLEAR 0x01
#define ST7920_INCREMENT_ADDR 0x06
#define ST7920_DISPLAY_ON 0x0C
#define ST7920_DISPLAY_OFF 0x08

#define ST7920_MODE_TEXT 0x34
#define ST7920_MODE_GRAPHICS 0x36
#define ST7920_SET_GRAPHIC_RAM_ADDR 0x80

#define ST7920_START_BYTE(rw, rs) (0b11111000 | (rw << 2) | (rs << 1))
#define ST7920_SPI_LOW(data) ((data & 0x0F) << 4)
#define ST7920_SPI_HIGH(data) (data & 0xF0)

struct st7920_display {
    spi_device_handle_t spi_handle;
    uint16_t max_x;
    uint16_t max_y;
    uint8_t *framebuffer;
    size_t framebuffer_len;
};

void st7920_cmd(spi_device_handle_t spi, const uint8_t rw, const uint8_t rs, const uint8_t data);

void st7920_data(spi_device_handle_t spi, const uint8_t data[], const size_t data_len);

void st7920_set_graphic_ram_addr(const struct st7920_display *disp, uint16_t x, uint16_t y);

void st7920_fb_put_pixel(struct st7920_display *display, const uint16_t x, const uint16_t y);

void st7920_fb_redraw(const struct st7920_display *display);

void st7920_fb_clear(struct st7920_display *display);