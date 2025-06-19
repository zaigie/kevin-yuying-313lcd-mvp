#ifndef DUMMY_AUDIO_CODEC_H
#define DUMMY_AUDIO_CODEC_H

#include <driver/gpio.h>
#include <esp_log.h>

// 简化的音频编解码器类，仅用于MVP中控制PA功放引脚
// 注意：Kevin Yuying 313 LCD使用ES8311通过I2C控制，PA引脚(GPIO45)只是功放使能
class DummyAudioCodec
{
private:
    gpio_num_t pa_pin_;
    bool output_enabled_;

public:
    DummyAudioCodec(gpio_num_t pa_pin)
        : pa_pin_(pa_pin), output_enabled_(false)
    {
        // 初始化PA功放控制引脚
        if (pa_pin_ != GPIO_NUM_NC)
        {
            gpio_config_t io_conf = {};
            io_conf.intr_type = GPIO_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_OUTPUT;
            io_conf.pin_bit_mask = (1ULL << pa_pin_);
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            gpio_config(&io_conf);

            // 默认关闭功放（与ES8311AudioCodec行为一致）
            SetPAEnabled(false);
        }
        ESP_LOGI("DummyAudioCodec", "PA amplifier control initialized on GPIO%d (disabled)", pa_pin_);
    }

    virtual ~DummyAudioCodec() {}

    // 控制功放使能引脚
    void SetPAEnabled(bool enable)
    {
        if (enable == output_enabled_)
        {
            return;
        }
        output_enabled_ = enable;

        if (pa_pin_ != GPIO_NUM_NC)
        {
            // PA引脚：高电平使能，低电平禁用（与ES8311AudioCodec一致）
            gpio_set_level(pa_pin_, enable ? 1 : 0);
            ESP_LOGI("DummyAudioCodec", "PA amplifier %s", enable ? "enabled" : "disabled");
        }
    }

    bool output_enabled() const { return output_enabled_; }
};

#endif // DUMMY_AUDIO_CODEC_H