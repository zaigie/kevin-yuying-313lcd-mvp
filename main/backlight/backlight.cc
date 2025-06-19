#include "backlight.h"
#include <esp_log.h>
#include <driver/ledc.h>

#define TAG "Backlight"
#define BACKLIGHT_LEDC_CHANNEL LEDC_CHANNEL_0
#define BACKLIGHT_LEDC_TIMER LEDC_TIMER_0

Backlight::Backlight() : brightness_(128), target_brightness_(128)
{
    // Create transition timer
    esp_timer_create_args_t timer_args = {
        .callback = [](void *arg)
        {
            static_cast<Backlight *>(arg)->OnTransitionTimer();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "backlight_transition",
        .skip_unhandled_events = false,
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &transition_timer_));
}

Backlight::~Backlight()
{
    if (transition_timer_)
    {
        esp_timer_stop(transition_timer_);
        esp_timer_delete(transition_timer_);
    }
}

void Backlight::RestoreBrightness()
{
    // Default brightness
    SetBrightness(204); // 80% brightness
}

void Backlight::SetBrightness(uint8_t brightness, bool permanent)
{
    target_brightness_ = brightness;

    if (brightness_ == target_brightness_)
    {
        return;
    }

    // Start transition timer
    step_ = (brightness_ < target_brightness_) ? 2 : -2;
    esp_timer_start_periodic(transition_timer_, 10000); // 10ms intervals
}

void Backlight::OnTransitionTimer()
{
    if (step_ > 0 && brightness_ >= target_brightness_)
    {
        brightness_ = target_brightness_;
        esp_timer_stop(transition_timer_);
    }
    else if (step_ < 0 && brightness_ <= target_brightness_)
    {
        brightness_ = target_brightness_;
        esp_timer_stop(transition_timer_);
    }
    else
    {
        brightness_ += step_;
    }

    SetBrightnessImpl(brightness_);
}

PwmBacklight::PwmBacklight(gpio_num_t pin, bool output_invert)
    : pin_(pin), output_invert_(output_invert)
{

    // Configure LEDC
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = BACKLIGHT_LEDC_TIMER,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num = pin_,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = BACKLIGHT_LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = BACKLIGHT_LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
        .flags = {
            .output_invert = static_cast<unsigned int>(output_invert_)}};
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));

    ESP_LOGI(TAG, "PWM backlight initialized on pin %d", pin_);
}

PwmBacklight::~PwmBacklight()
{
    // Stop LEDC
    ledc_stop(LEDC_LOW_SPEED_MODE, BACKLIGHT_LEDC_CHANNEL, 0);
}

void PwmBacklight::SetBrightnessImpl(uint8_t brightness)
{
    uint32_t duty = brightness;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, BACKLIGHT_LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, BACKLIGHT_LEDC_CHANNEL));
}