/*
    Newclock
    display/st7920_cmds.c

    Copyright (c) 2025 Rafal Macyszyn

    SPDX-License-Identifier: BSD-3-Clause
*/

#include <st7920_cmds.h>

#include <string.h>
#include <driver/spi_master.h>
#include <esp_err.h>

static esp_err_t ret;

#define BUF_SIZE 280
static DRAM_ATTR uint8_t buf[BUF_SIZE];

void st7920_cmd(spi_device_handle_t spi, const uint8_t rw, const uint8_t rs, const uint8_t data)
{
    spi_transaction_t transaction = {
        .length = 24,
        .tx_data = {ST7920_START_BYTE(rw, rs), ST7920_SPI_HIGH(data), ST7920_SPI_LOW(data)},
        .flags = SPI_TRANS_USE_TXDATA,
    };

    ret = spi_device_polling_transmit(spi, &transaction);
    ESP_ERROR_CHECK(ret);
}

void st7920_data(spi_device_handle_t spi, const uint8_t data[], const size_t data_len)
{
    assert(BUF_SIZE >= 2 * data_len + 1);

    buf[0] = 0xFA; // start word
    size_t buf_idx = 1;
    for (size_t i = 0; i < data_len; i++) {
        buf[buf_idx++] = data[i] & 0xF0;
        buf[buf_idx++] = data[i] << 4;
    }

    spi_transaction_t transaction = {
        .length = 8 * buf_idx,
        .tx_buffer = buf,
    };

    ret = spi_device_polling_transmit(spi, &transaction);
    ESP_ERROR_CHECK(ret);
}

void st7920_set_graphic_ram_addr(const struct st7920_display *disp, uint16_t x, uint16_t y)
{
    st7920_cmd(disp->spi_handle, 0, 0, ST7920_SET_GRAPHIC_RAM_ADDR | y);
    st7920_cmd(disp->spi_handle, 0, 0, ST7920_SET_GRAPHIC_RAM_ADDR | x);
}

void st7920_fb_put_pixel(struct st7920_display *display, const uint16_t x, const uint16_t y)
{
    if (x < 0 || x > display->max_x || y < 0 || y > display->max_y)
        return;

    size_t fb_tile = (y * 128 + x) / 8;
    display->framebuffer[fb_tile] |= (1 << (7 - x % 8));
}

void st7920_fb_redraw(const struct st7920_display *display)
{
    size_t tile_offset;

    for (uint8_t y = 0; y < 32; y++) {
        // * y goes from 0 to 32, which is the half of the display
        // * we draw both halves (top and bottom) in the next loop

        // * tile_nr is a 16px-wide block
        // * if we write 8 tiles, then next 8, we jump into (y+32)'th row

        // First, top half
        tile_offset = y * 16;
        st7920_cmd(display->spi_handle, 0, 0, 0x80 | y);
        st7920_cmd(display->spi_handle, 0, 0, 0x80 | 0);
        st7920_data(display->spi_handle, display->framebuffer + tile_offset, 16);

        // Then, second half
        tile_offset = (y + 32) * 16;
        st7920_cmd(display->spi_handle, 0, 0, 0x80 | y);
        st7920_cmd(display->spi_handle, 0, 0, 0x80 | 8);
        st7920_data(display->spi_handle, display->framebuffer + tile_offset, 16);
    }
}

void st7920_fb_clear(struct st7920_display *display)
{
    memset(display->framebuffer, 0, display->framebuffer_len);
}