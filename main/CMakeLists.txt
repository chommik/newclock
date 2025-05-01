idf_component_register(
    SRCS
        main.c

        display/st7920_cmds.c

        utils/str_replace.c
        utils/date_format.c

        tasks/temperature.c
        tasks/button_handling.c
        tasks/task_brightness.c

        ui/main_display.c
        ui/config_screen.c
        ui/screen_brightness.c
        ui/screen_set_clock.c
        ui/screen_set_date.c
        ui/utils.c
        ui/buttons.c
        ui/progress_bar.c

        u8g2_fonts/u8g2_font_maniacbold_tn.c
        u8g2_fonts/u8g2_font_maniacbold_tr.c

        u8g2_adapter/u8g2_d_st7920_esp32.c

        u8g2_parts/u8g2_arc.c
        u8g2_parts/u8g2_bitmap.c
        u8g2_parts/u8g2_box.c
        u8g2_parts/u8g2_buffer.c
        u8g2_parts/u8g2_button.c
        u8g2_parts/u8g2_circle.c
        u8g2_parts/u8g2_cleardisplay.c
        u8g2_parts/u8g2_d_memory.c
        u8g2_parts/u8g2_d_setup.c
        u8g2_parts/u8g2_font.c
        u8g2_parts/u8g2_fonts.c
        u8g2_parts/u8g2_hvline.c
        u8g2_parts/u8g2_input_value.c
        u8g2_parts/u8g2_intersection.c
        u8g2_parts/u8g2_kerning.c
        u8g2_parts/u8g2_line.c
        u8g2_parts/u8g2_ll_hvline.c
        u8g2_parts/u8g2_message.c
        u8g2_parts/u8g2_polygon.c
        u8g2_parts/u8g2_selection_list.c
        u8g2_parts/u8g2_setup.c
        u8g2_parts/u8x8_8x8.c
        u8g2_parts/u8x8_byte.c
        u8g2_parts/u8x8_debounce.c
        u8g2_parts/u8x8_display.c
        u8g2_parts/u8x8_fonts.c
        u8g2_parts/u8x8_gpio.c
        u8g2_parts/u8x8_input_value.c
        u8g2_parts/u8x8_message.c
        u8g2_parts/u8x8_selection_list.c
        u8g2_parts/u8x8_setup.c
        u8g2_parts/u8x8_string.c
        u8g2_parts/u8x8_u16toa.c
        u8g2_parts/u8x8_u8toa.c
    INCLUDE_DIRS
        .
        u8g2_parts
        u8g2_adapter
        u8g2_fonts
        tasks
        display
    REQUIRES
        esp_adc
        esp_driver_ledc
        esp_driver_spi
        ds18x20
)