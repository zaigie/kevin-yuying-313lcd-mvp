#include "display.h"
#include <esp_log.h>
#include <esp_err.h>
#include <string>

#define TAG "Display"

Display::Display()
{
    // Notification timer
    esp_timer_create_args_t notification_timer_args = {
        .callback = [](void *arg)
        {
            Display *display = static_cast<Display *>(arg);
            DisplayLockGuard lock(display);
            if (display->notification_label_)
            {
                lv_obj_add_flag(display->notification_label_, LV_OBJ_FLAG_HIDDEN);
            }
            if (display->status_label_)
            {
                lv_obj_clear_flag(display->status_label_, LV_OBJ_FLAG_HIDDEN);
            }
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "notification_timer",
        .skip_unhandled_events = false,
    };
    ESP_ERROR_CHECK(esp_timer_create(&notification_timer_args, &notification_timer_));

    // Create a power management lock
    auto ret = esp_pm_lock_create(ESP_PM_APB_FREQ_MAX, 0, "display_update", &pm_lock_);
    if (ret == ESP_ERR_NOT_SUPPORTED)
    {
        ESP_LOGI(TAG, "Power management not supported");
    }
    else
    {
        ESP_ERROR_CHECK(ret);
    }
}

Display::~Display()
{
    if (notification_timer_ != nullptr)
    {
        esp_timer_stop(notification_timer_);
        esp_timer_delete(notification_timer_);
    }

    if (notification_label_ != nullptr)
    {
        lv_obj_del(notification_label_);
    }
    if (status_label_ != nullptr)
    {
        lv_obj_del(status_label_);
    }

    if (pm_lock_ != nullptr)
    {
        esp_pm_lock_delete(pm_lock_);
    }
}

void Display::SetStatus(const char *status)
{
    DisplayLockGuard lock(this);
    if (status_label_ == nullptr)
    {
        return;
    }
    lv_label_set_text(status_label_, status);
    lv_obj_clear_flag(status_label_, LV_OBJ_FLAG_HIDDEN);
    if (notification_label_)
    {
        lv_obj_add_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);
    }
}

void Display::ShowNotification(const std::string &notification, int duration_ms)
{
    ShowNotification(notification.c_str(), duration_ms);
}

void Display::ShowNotification(const char *notification, int duration_ms)
{
    DisplayLockGuard lock(this);
    if (notification_label_ == nullptr)
    {
        return;
    }
    lv_label_set_text(notification_label_, notification);
    lv_obj_clear_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);
    if (status_label_)
    {
        lv_obj_add_flag(status_label_, LV_OBJ_FLAG_HIDDEN);
    }

    esp_timer_stop(notification_timer_);
    ESP_ERROR_CHECK(esp_timer_start_once(notification_timer_, duration_ms * 1000));
}