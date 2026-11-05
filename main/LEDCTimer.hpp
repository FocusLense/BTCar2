#pragma once
 
#include <cstdint>
#include "driver/ledc.h"
#include "esp_err.h"

class LEDCTimer {
    private:
        ledc_mode_t speed_mode;
        ledc_timer_t timer_num;
        ledc_timer_bit_t duty_res;
        uint32_t freq_hz;
    
    public:
        
        LEDCTimer(ledc_mode_t mode, ledc_timer_t timer_nr, ledc_timer_bit_t duty_res, uint32_t freq_hz)
            :speed_mode(mode), timer_num(timer_nr), duty_res(duty_res), freq_hz(freq_hz) {};

        //copy not allowed
        LEDCTimer(const LEDCTimer&) = delete;
        LEDCTimer& operator = (const LEDCTimer&) = delete;
        //mov_ok
        LEDCTimer(LEDCTimer&&) = default;
        LEDCTimer& operator = (LEDCTimer&) = default;

        ledc_mode_t getSpeedMode() const{
            return this->speed_mode;
        }
        ledc_timer_t getTimerNr() const{
            return this->timer_num;
        }
        ledc_timer_bit_t getDutyRes() const{
            return this->duty_res;
        }
        uint32_t getFreqHz() const{
            return this->freq_hz;
        }
        uint32_t get_max_duty() const { 
            return (1u << duty_res) - 1u; 
        }

        esp_err_t init(){
            ledc_timer_config_t timer_cfg = {};
                timer_cfg.speed_mode = this->getSpeedMode();
                timer_cfg.timer_num = this->getTimerNr();
                timer_cfg.duty_resolution = this->getDutyRes();
                timer_cfg.freq_hz = this->getFreqHz();
                timer_cfg.clk_cfg = LEDC_AUTO_CLK;
            return ledc_timer_config(&timer_cfg);
        }
};