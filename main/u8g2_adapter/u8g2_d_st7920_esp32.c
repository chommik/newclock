
#include "u8g2_d_st7920_esp32.h"

#include <esp_log.h>

#include <stdint.h>

#include "st7920_cmds.h"
#include "u8g2.h"

/*==========================================*/
/* tga procedures */

static const u8x8_display_info_t u8x8_st7920_128x64_esp32_info = {
    /* chip_enable_level = */ 0,
    /* chip_disable_level = */ 1,

    /* post_chip_enable_wait_ns = */ 0,
    /* pre_chip_disable_wait_ns = */ 0,
    /* reset_pulse_width_ms = */ 0,
    /* post_reset_wait_ms = */ 0,
    /* sda_setup_time_ns = */ 0,
    /* sck_pulse_width_ns = */ 0,
    /* sck_clock_hz = */ 4000000UL, /* since Arduino 1.6.0, the SPI bus speed in Hz. Should be
                                       1000000000/sck_pulse_width_ns */
    /* spi_mode = */ 1,
    /* i2c_bus_clock_100kHz = */ 0,
    /* data_setup_time_ns = */ 0,
    /* write_pulse_width_ns = */ 0,
    /* tile_width = */ 16,
    /* tile_hight = */ 8,
    /* default_x_offset = */ 0,
    /* flipmode_x_offset = */ 0,
    128,
    64};

static const char *TAG = "u8g2_d_st7920";

static struct st7920_display *g_display = NULL;

static uint8_t spi_buf[128];

uint8_t u8x8_d_st7920_128x64_esp32(u8x8_t *u8g2, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch (msg) {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8g2, &u8x8_st7920_128x64_esp32_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8g2);
      //   tga_init(DEFAULT_WIDTH, DEFAULT_HEIGHT);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      break;
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:

      u8x8_tile_t *tile = (u8x8_tile_t *)arg_ptr;

      ESP_LOGV(TAG, "arg_int = %d, tile = { .cnt = %d, .x_pos = %d, .y_pos = %d, .ptr = %p }", arg_int, tile->cnt,
               tile->x_pos, tile->y_pos, tile->tile_ptr);

      for (uint8_t tile_counter = 0; tile_counter < tile->cnt; tile_counter++) {
        uint8_t *current_tile_ptr = tile->tile_ptr + 8 * tile_counter;

        for (uint8_t tile_bit = 0; tile_bit < 8; tile_bit++) {
          for (uint8_t tile_byte = 0; tile_byte < 8; tile_byte++) {

            // size_t framebuffer_offset = ((tile->y_pos * 8 + tile_bit) * 128 + (64 * tile_counter)) / 8; // (y * 128 +
            // x) / 8
            size_t framebuffer_offset = 16 * (tile_bit + 8 * tile->y_pos) + tile_counter;

            uint8_t fb_bit_select_mask = (1 << (7 - tile_byte));
            uint8_t tile_bit_select_mask = (1 << tile_bit);

            uint8_t current_tile_byte = current_tile_ptr[tile_byte];
            if (current_tile_byte & tile_bit_select_mask)
              g_display->framebuffer[framebuffer_offset] |= fb_bit_select_mask;
          }
        }

        // for (uint8_t row_counter = 0; row_counter < 8; row_counter++) {
        //   size_t framebuffer_offset = 16 * (row_counter + 8 * tile->y_pos) + tile_counter;
        //   size_t tile_ptr_offset = row_counter;
        //   g_display->framebuffer[framebuffer_offset] = current_tile_ptr[tile_ptr_offset];
        // }
      }

      // uint8_t constant_offset = (tile->x_pos * u8g2->display_info->tile_width) / 8;

      // for (uint8_t l_tile_offset = 0; l_tile_offset < tile->cnt; l_tile_offset++) {
      //   uint8_t *l_src_tile_ptr = l_tile->tile_ptr + l_tile_offset;
      // }

      // x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      // // printf("U8X8_MSG_DISPLAY_DRAW_TILE x=%d, ", x);
      // x *= 8;
      // x += u8g2->x_offset;

      // y = ((u8x8_tile_t *)arg_ptr)->y_pos;
      // // printf("y=%d, c=%d\n", y, ((u8x8_tile_t *)arg_ptr)->cnt);
      // y *= 8;

      // do {
      //   c = ((u8x8_tile_t *)arg_ptr)->cnt;
      //   ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
      //   tga_set_multiple_8pixel(x, y, c * 8, ptr);

      //   arg_int--;
      // } while (arg_int > 0);

      break;
    default:
      return 0;
  }
  return 1;
}

void u8x8_Setup_st7920_128x64_esp32(u8x8_t *u8x8)
{
  /* setup defaults */
  u8x8_SetupDefaults(u8x8);

  /* setup specific callbacks */
  u8x8->display_cb = u8x8_d_st7920_128x64_esp32;

  /* setup display info */
  u8x8_SetupMemory(u8x8);
}

void u8g2_SetupBuffer_st7920_128x64_esp32(u8g2_t *u8g2, const u8g2_cb_t *u8g2_cb, struct st7920_display *display)
{
  static uint8_t buf[2 * 128 * 64 / 8];

  u8x8_Setup_st7920_128x64_esp32(u8g2_GetU8x8(u8g2));
  u8g2_SetupBuffer(u8g2, buf, 8, u8g2_ll_hvline_vertical_top_lsb, u8g2_cb);

  g_display = display;
}
