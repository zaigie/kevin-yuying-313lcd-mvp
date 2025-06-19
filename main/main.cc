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

// Task to setup display and graphics after board initialization
static void display_setup_task(void *param)
{
    ESP_LOGI(TAG, "Display setup task starting...");

    // Wait longer for the board to fully initialize
    vTaskDelay(pdMS_TO_TICKS(1000));

    auto &board = Board::GetInstance();
    ESP_LOGI(TAG, "Got board instance");

    // Get backlight first and set brightness
    auto *backlight = board.GetBacklight();
    if (backlight)
    {
        ESP_LOGI(TAG, "Setting backlight brightness to 80%%");
        backlight->SetBrightness(204); // 80% of 255
    }
    else
    {
        ESP_LOGI(TAG, "No backlight available");
    }

    // Get display and test basic functionality
    auto *display = board.GetDisplay();
    if (display)
    {
        ESP_LOGI(TAG, "Display available: %dx%d", display->width(), display->height());

        // Wait much longer for LVGL to be completely ready
        ESP_LOGI(TAG, "Waiting for LVGL to be ready...");
        vTaskDelay(pdMS_TO_TICKS(2000));

        ESP_LOGI(TAG, "Attempting to set status...");
        // Use DisplayLockGuard for thread safety
        {
            DisplayLockGuard lock(display);
            display->SetStatus("Hello LVGL!");
        }

        ESP_LOGI(TAG, "Waiting before notification...");
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_LOGI(TAG, "Attempting to show notification...");
        {
            DisplayLockGuard lock(display);
            display->ShowNotification("Kevin Yuying 313 LCD MVP", 10000);
        }

        ESP_LOGI(TAG, "Waiting before graphics...");
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_LOGI(TAG, "Attempting to create label...");
        // Test drawing some basic graphics with LVGL using DisplayLockGuard
        {
            DisplayLockGuard lock(display);
            lv_obj_t *label = lv_label_create(lv_screen_active());
            if (label)
            {
                ESP_LOGI(TAG, "Label created successfully");
                lv_label_set_text(label, "LVGL Working!\nESP32-S3");
                lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
                lv_obj_set_style_text_color(label, lv_color_hex(0xFF0000), 0);
                ESP_LOGI(TAG, "Label styled and positioned");
            }
            else
            {
                ESP_LOGE(TAG, "Failed to create label");
            }
        }

        ESP_LOGI(TAG, "Waiting before rectangle...");
        vTaskDelay(pdMS_TO_TICKS(500));

        ESP_LOGI(TAG, "Attempting to create rectangle...");
        // Create a simple rectangle using DisplayLockGuard
        {
            DisplayLockGuard lock(display);
            lv_obj_t *rect = lv_obj_create(lv_screen_active());
            if (rect)
            {
                ESP_LOGI(TAG, "Rectangle created successfully");
                lv_obj_set_size(rect, 150, 80);
                lv_obj_align(rect, LV_ALIGN_BOTTOM_MID, 0, -50);
                lv_obj_set_style_bg_color(rect, lv_color_hex(0x00FF00), 0);
                ESP_LOGI(TAG, "Rectangle configured");
            }
            else
            {
                ESP_LOGE(TAG, "Failed to create rectangle");
            }
        }

        ESP_LOGI(TAG, "LVGL graphics test completed");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to get display instance");
    }

    ESP_LOGI(TAG, "Display setup task completed");

    // Delete this task
    vTaskDelete(NULL);
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

    ESP_LOGI(TAG, "Board initialization complete. Starting display setup task...");

    // Create display setup task to avoid blocking main task
    xTaskCreate(display_setup_task, "display_setup", 8192, NULL, 3, NULL);

    ESP_LOGI(TAG, "Main initialization complete. System running.");

    // Main loop - keep the system running with reduced logging to avoid resource conflicts
    int counter = 0;
    while (1)
    {
        // Use longer delay to reduce system load and avoid conflicts with display task
        vTaskDelay(pdMS_TO_TICKS(10000));

        // Reduce logging frequency and ensure thread safety
        if (counter % 6 == 0)
        { // Log every minute instead of every 10 seconds
            ESP_LOGI(TAG, "System running... %d minutes", counter / 6);
        }
        counter++;

        // Yield to allow other tasks to run properly
        taskYIELD();
    }
}