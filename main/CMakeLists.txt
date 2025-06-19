set(SOURCES 
    "main.cc"
    "display/display.cc"
    "display/lcd_display.cc"
    "board/board.cc"
    "board/kevin_yuying_313lcd.cc"
    "backlight/backlight.cc"
    "esp_lcd_gc9503.c"
)

set(INCLUDE_DIRS "." "display" "board" "backlight")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES 
        driver
        esp_timer
        nvs_flash
        esp_event
        esp_pm
        esp_lcd
        lvgl
        esp_lvgl_port
) 