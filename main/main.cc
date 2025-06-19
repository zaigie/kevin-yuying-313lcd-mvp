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
#include "audio/dummy_audio_codec.h"

#define TAG "main"

// Simple display initialization - no complex locking or tasks needed
static void setup_simple_display()
{
    ESP_LOGI(TAG, "Setting up simple display...");

    auto &board = Board::GetInstance();

    // Set backlight
    auto *backlight = board.GetBacklight();
    if (backlight)
    {
        ESP_LOGI(TAG, "Setting backlight brightness to 80%%");
        backlight->SetBrightness(204); // 80% of 255
    }

    // Get display but only for info, don't use its methods
    auto *display = board.GetDisplay();
    if (display)
    {
        ESP_LOGI(TAG, "Display available: %dx%d", display->width(), display->height());

        // Wait for LVGL to initialize
        ESP_LOGI(TAG, "Waiting for LVGL to initialize...");
        vTaskDelay(pdMS_TO_TICKS(3000));

        ESP_LOGI(TAG, "Creating simple demo label...");
        // Create a simple demo label directly with LVGL - no Display wrapper
        lv_obj_t *label = lv_label_create(lv_screen_active());
        if (label)
        {
            lv_label_set_text(label, "Kevin Yuying 313 LCD\nMVP Demo\nESP32-S3 + LVGL\nRunning!");
            lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_style_text_color(label, lv_color_hex(0x0080FF), 0);
            ESP_LOGI(TAG, "Demo label created successfully");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to create demo label");
        }

        // Create a simple colored background
        lv_obj_t *bg = lv_obj_create(lv_screen_active());
        if (bg)
        {
            lv_obj_set_size(bg, 300, 100);
            lv_obj_align(bg, LV_ALIGN_BOTTOM_MID, 0, -50);
            lv_obj_set_style_bg_color(bg, lv_color_hex(0x004080), 0);
            lv_obj_set_style_border_width(bg, 2, 0);
            lv_obj_set_style_border_color(bg, lv_color_hex(0x0080FF), 0);
            ESP_LOGI(TAG, "Background element created");
        }
    }
    else
    {
        ESP_LOGE(TAG, "No display available");
    }

    ESP_LOGI(TAG, "Simple display setup completed");
}

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

    ESP_LOGI(TAG, "Initializing board...");
    // Get board instance - this will initialize the hardware
    auto &board = Board::GetInstance();
    ESP_LOGI(TAG, "Board type: %s", board.GetBoardType().c_str());

    // 初始化音频编解码器以确保PA引脚状态正确
    auto *codec = board.GetAudioCodec();
    if (codec)
    {
        ESP_LOGI(TAG, "Audio codec initialized - PA pin is now controlled properly");
        ESP_LOGI(TAG, "Audio output enabled: %s", codec->output_enabled() ? "yes" : "no");
    }
    else
    {
        ESP_LOGI(TAG, "No audio codec available - PA pin manually controlled");
    }

    ESP_LOGI(TAG, "Board initialization complete. Setting up display...");

    // Simple display setup - no separate task needed
    setup_simple_display();

    ESP_LOGI(TAG, "MVP initialization complete. System running.");

    // Simple main loop - just keep the system alive
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(60000)); // Check every minute
        ESP_LOGI(TAG, "MVP system running...");
    }
}