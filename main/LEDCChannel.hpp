#pragma once
 
#include <cstdint>
#include <algorithm>
#include "driver/ledc.h"
#include "esp_err.h"
#include "LEDCTimer.hpp"

class LEDCChannel {
    private:
        LEDCTimer& timer;
        ledc_channel_t channel;
        uint8_t gpio_nr;

    public:
        LEDCChannel(LEDCTimer& timer, ledc_channel_t channel, uint8_t gpio_nr)
            : timer(timer), channel(channel), gpio_nr(gpio_nr) {}

        LEDCChannel(const LEDCChannel&) = delete;
        LEDCChannel& operator=(const LEDCChannel&) = delete;

        LEDCChannel(LEDCChannel&&) = default;
        LEDCChannel& operator=(LEDCChannel&&) = default;

        esp_err_t set_duty(uint32_t duty){
            duty = std::min(duty, static_cast<uint32_t>((1u << timer.getDutyRes()) - 1));
            esp_err_t err = ledc_set_duty(timer.getSpeedMode(),this->get_channel(), duty);
            if (err != ESP_OK) return err;
            return ledc_update_duty(timer.getSpeedMode(), this->get_channel());
        }

        esp_err_t setDutyPercent(float percent) {
                percent = std::clamp(percent, 0.0f, 100.0f);
                uint32_t duty = static_cast<uint32_t>(
                    (percent / 100.0f) * timer.get_max_duty()
                );
                return set_duty(duty);
        }

        esp_err_t setPulseWidthUs(uint32_t pulse_width_us) {
            if (timer.getFreqHz() == 0) {
                return ESP_ERR_INVALID_ARG;
            }

            const uint32_t period_us = 1000000u / timer.getFreqHz();
            if (period_us == 0) {
                return ESP_ERR_INVALID_ARG;
            }
            pulse_width_us = std::min(pulse_width_us, period_us);
            const uint32_t duty = static_cast<uint32_t>(
                (static_cast<uint64_t>(pulse_width_us) * timer.get_max_duty() + period_us / 2) /
                period_us
            );
            return set_duty(duty);
        }

        uint8_t get_gpio_num() const{
            return this->gpio_nr;
        }
        ledc_channel_t get_channel() const{
            return this->channel;
        }
        LEDCTimer& get_timer() const{
            return this->timer;
        }
        esp_err_t init(uint32_t initial_duty = 0){
            ledc_channel_config_t channel_cfg = {};
                channel_cfg.channel = this->get_channel();
                channel_cfg.speed_mode = this->get_timer().getSpeedMode();
                channel_cfg.gpio_num = this->get_gpio_num();
                channel_cfg.timer_sel = this->get_timer().getTimerNr();
                channel_cfg.duty = initial_duty;
                channel_cfg.hpoint = 0;
            
            return ledc_channel_config(&channel_cfg);
        }
        
};