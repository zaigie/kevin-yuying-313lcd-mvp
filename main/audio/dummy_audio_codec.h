#ifndef DUMMY_AUDIO_CODEC_H
#define DUMMY_AUDIO_CODEC_H

#include <driver/gpio.h>
#include <esp_log.h>

// 简化的音频编解码器类，仅用于MVP中控制PA引脚
class DummyAudioCodec
{
private:
    gpio_num_t pa_pin_;
    bool pa_inverted_;
    bool output_enabled_;

public:
    DummyAudioCodec(gpio_num_t pa_pin, bool pa_inverted = false)
        : pa_pin_(pa_pin), pa_inverted_(pa_inverted), output_enabled_(false)
    {
        // 初始化PA引脚
        if (pa_pin_ != GPIO_NUM_NC)
        {
            gpio_config_t io_conf = {};
            io_conf.intr_type = GPIO_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_OUTPUT;
            io_conf.pin_bit_mask = (1ULL << pa_pin_);
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            gpio_config(&io_conf);

            // 默认禁用输出（模拟原项目行为）
            EnableOutput(false);
        }
        ESP_LOGI("DummyAudioCodec", "Initialized with PA pin GPIO%d, inverted=%d", pa_pin_, pa_inverted_);
    }

    virtual ~DummyAudioCodec() {}

    // 控制音频输出使能（主要是为了控制PA引脚）
    void EnableOutput(bool enable)
    {
        if (enable == output_enabled_)
        {
            return;
        }
        output_enabled_ = enable;

        if (pa_pin_ != GPIO_NUM_NC)
        {
            int level = output_enabled_ ? 1 : 0;
            gpio_set_level(pa_pin_, pa_inverted_ ? !level : level);
            ESP_LOGI("DummyAudioCodec", "PA pin set to %s (level=%d)",
                     output_enabled_ ? "enabled" : "disabled",
                     pa_inverted_ ? !level : level);
        }
    }

    bool output_enabled() const { return output_enabled_; }
};

#endif // DUMMY_AUDIO_CODEC_H