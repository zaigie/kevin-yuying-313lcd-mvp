#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "board/board.h"
#include "display/display.h"
#include "backlight/backlight.h"

#define TAG "main"

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Kevin Yuying 313 LCD MVP starting...");

    // Initialize the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Get board instance
    auto &board = Board::GetInstance();
    ESP_LOGI(TAG, "Board type: %s", board.GetBoardType().c_str());

    // Get display and show test content
    auto *display = board.GetDisplay();
    if (display)
    {
        ESP_LOGI(TAG, "Display initialized successfully: %dx%d", display->width(), display->height());

        // Test the display by showing some text
        display->SetStatus("Hello LVGL!");
        display->ShowNotification("Kevin Yuying 313 LCD MVP", 5000);

        // Test drawing some basic graphics with LVGL
        lv_obj_t *label = lv_label_create(lv_screen_active());
        lv_label_set_text(label, "LVGL Working!\nScreen: 376x960\nESP32-S3\nGC9503 Driver");
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFF0000), 0);

        // Create a simple rectangle
        lv_obj_t *rect = lv_obj_create(lv_screen_active());
        lv_obj_set_size(rect, 200, 100);
        lv_obj_align(rect, LV_ALIGN_BOTTOM_MID, 0, -50);
        lv_obj_set_style_bg_color(rect, lv_color_hex(0x00FF00), 0);
        lv_obj_set_style_border_width(rect, 2, 0);
        lv_obj_set_style_border_color(rect, lv_color_hex(0x0000FF), 0);

        ESP_LOGI(TAG, "LVGL graphics displayed");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to initialize display");
    }

    // Get backlight and set brightness
    auto *backlight = board.GetBacklight();
    if (backlight)
    {
        ESP_LOGI(TAG, "Setting backlight brightness to 80%%");
        backlight->SetBrightness(204); // 80% of 255
    }

    ESP_LOGI(TAG, "MVP initialization complete. Display should be working now.");

    // Main loop - just keep the system running
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}