#include "lcd_display.h"
#include <vector>
#include <algorithm>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_lvgl_port.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#define TAG "LcdDisplay"

static SemaphoreHandle_t lvgl_mux = nullptr;

LcdDisplay::LcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, int width, int height)
    : panel_io_(panel_io), panel_(panel)
{
    width_ = width;
    height_ = height;

    // Create LVGL mutex if not already created
    if (lvgl_mux == nullptr)
    {
        lvgl_mux = xSemaphoreCreateMutex();
    }
}

LcdDisplay::~LcdDisplay()
{
    if (notification_label_ != nullptr)
    {
        lv_obj_del(notification_label_);
    }
    if (status_label_ != nullptr)
    {
        lv_obj_del(status_label_);
    }
}

bool LcdDisplay::Lock(int timeout_ms)
{
    if (lvgl_mux == nullptr)
        return false;
    return xSemaphoreTake(lvgl_mux, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void LcdDisplay::Unlock()
{
    if (lvgl_mux != nullptr)
    {
        xSemaphoreGive(lvgl_mux);
    }
}

void LcdDisplay::SetupUI()
{
    ESP_LOGI(TAG, "Setting up basic UI components");

    // Create status label in the top center
    status_label_ = lv_label_create(lv_screen_active());
    if (status_label_)
    {
        lv_label_set_text(status_label_, "Ready");
        lv_obj_align(status_label_, LV_ALIGN_TOP_MID, 0, 10);
        // Set style immediately during creation
        lv_obj_set_style_text_color(status_label_, lv_color_white(), 0);
        ESP_LOGI(TAG, "Status label created and styled");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to create status label");
        return;
    }

    // Create notification label
    notification_label_ = lv_label_create(lv_screen_active());
    if (notification_label_)
    {
        lv_label_set_text(notification_label_, "");
        lv_obj_align(notification_label_, LV_ALIGN_TOP_MID, 0, 50);
        lv_obj_add_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);
        // Set style immediately during creation
        lv_obj_set_style_text_color(notification_label_, lv_color_hex(0x00FF00), 0);
        ESP_LOGI(TAG, "Notification label created and styled");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to create notification label");
        return;
    }

    ESP_LOGI(TAG, "Basic UI components created and styled successfully");
}

// RGB LCD实现
RgbLcdDisplay::RgbLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                             int width, int height, int offset_x, int offset_y,
                             bool mirror_x, bool mirror_y, bool swap_xy)
    : LcdDisplay(panel_io, panel, width, height)
{

    ESP_LOGI(TAG, "Initializing RGB LCD Display %dx%d", width, height);

    // draw white background first
    std::vector<uint16_t> buffer(width_, 0xFFFF);
    for (int y = 0; y < height_; y++)
    {
        esp_lcd_panel_draw_bitmap(panel_, 0, y, width_, y + 1, buffer.data());
    }

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();

    ESP_LOGI(TAG, "Initialize LVGL port");
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    port_cfg.task_priority = 4;
    port_cfg.timer_period_ms = 20; // Further increase timer period to reduce load
    ESP_ERROR_CHECK(lvgl_port_init(&port_cfg));

    ESP_LOGI(TAG, "Adding RGB LCD display to LVGL");
    const lvgl_port_display_cfg_t display_cfg = {
        .io_handle = panel_io_,
        .panel_handle = panel_,
        .buffer_size = static_cast<uint32_t>(width_ * 10), // Further reduce buffer size
        .double_buffer = true,
        .hres = static_cast<uint32_t>(width_),
        .vres = static_cast<uint32_t>(height_),
        .rotation = {
            .swap_xy = swap_xy,
            .mirror_x = mirror_x,
            .mirror_y = mirror_y,
        },
        .flags = {
            .buff_dma = 1,
            .swap_bytes = 0,
            .full_refresh = 1,
            .direct_mode = 1,
        },
    };

    const lvgl_port_display_rgb_cfg_t rgb_cfg = {
        .flags = {
            .bb_mode = true,
            .avoid_tearing = true,
        }};

    display_ = lvgl_port_add_disp_rgb(&display_cfg, &rgb_cfg);
    if (display_ == nullptr)
    {
        ESP_LOGE(TAG, "Failed to add RGB display to LVGL");
        return;
    }

    if (offset_x != 0 || offset_y != 0)
    {
        lv_display_set_offset(display_, offset_x, offset_y);
    }

    ESP_LOGI(TAG, "Setting up basic UI");
    // Setup the basic UI first - styles are now set immediately during creation
    SetupUI();

    ESP_LOGI(TAG, "RGB LCD display initialization complete");
}