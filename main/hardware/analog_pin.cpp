/** \file analog_pin.cpp
 *  \brief Class handling reading an analog pin value.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "driver/rtc_io.h"
#include "analog_pin.h"

namespace eobsws::hardware {

    AnalogPin::~AnalogPin() {
        rtc_gpio_deinit(this->pin_num);
        adc_power_release();
        this->initialized = false;
    }

    void AnalogPin::initialize() {
        if (this->initialized) return;
        adc_power_acquire();
        this->initialized = true;
    }

    void AnalogPin::set_pin(const gpio_num_t pin_num) {
        esp_err_t ret;
        bool channel_found = false;
        // search for ADC channel corresponding to pin, first in ADC1 channels
        for (uint8_t chan=(uint8_t)ADC1_CHANNEL_0; chan<(uint8_t)ADC1_CHANNEL_MAX; chan++) {
            gpio_num_t pad;
            ret = adc1_pad_get_io_num((adc1_channel_t)chan, &pad);
            ESP_ERROR_CHECK(ret);
            if (pad == pin_num) {
                this->adc_channel = (adc_channel_t)chan;
                this->adc_unit = ADC_UNIT_1;
                channel_found = true;
                break;
            }
        }
        // if channel not found, search in ADC2 channels
        if (!channel_found) {
            for (uint8_t chan=(uint8_t)ADC2_CHANNEL_0; chan<(uint8_t)ADC2_CHANNEL_MAX; chan++) {
                gpio_num_t pad;
                ret = adc2_pad_get_io_num((adc2_channel_t)chan, &pad);
                ESP_ERROR_CHECK(ret);
                if (pad == pin_num) {
                    this->adc_channel = (adc_channel_t)chan;
                    this->adc_unit = ADC_UNIT_2;
                    channel_found = true;
                    break;
                }
            }
        }
        assert(channel_found);
        this->pin_num = pin_num;
        if (!this->initialized) this->initialize();
    }

    void AnalogPin::set_width(const adc_bits_width_t width) {
        this->adc_width = width;
    }

    void AnalogPin::set_attenuation(const adc_atten_t atten) {
        this->adc_atten = atten;
    }

    void AnalogPin::set_measurement_count(const uint16_t n_meas) {
        assert(n_meas>0);
        this->n_meas = n_meas;
    }

    void AnalogPin::set_tolerance(const uint16_t meas_tolerance) {
        this->meas_tolerance = meas_tolerance;
    }

    bool AnalogPin::read(int * result) {
        if (this->adc_unit == ADC_UNIT_1) {
            adc1_config_width(this->adc_width);
            adc1_config_channel_atten((adc1_channel_t)this->adc_channel, this->adc_atten);
            if (this->n_meas>1) {
                uint16_t cnt = this->n_meas;
                uint32_t meas = 0;
                do { meas += adc1_get_raw((adc1_channel_t)this->adc_channel); } while (cnt--);
                *result = meas/this->n_meas;
            } else {
                *result = adc1_get_raw((adc1_channel_t)this->adc_channel);
            }
        } else if (this->adc_unit == ADC_UNIT_2) {
            adc2_config_channel_atten((adc2_channel_t)this->adc_channel, this->adc_atten);
            if (this->n_meas>1) {
                adc2_get_raw((adc2_channel_t)this->adc_channel, this->adc_width, result);
            } else {
                int buff = 0;
                uint16_t cnt = this->n_meas;
                uint32_t meas = 0;
                do {
                    adc2_get_raw((adc2_channel_t)this->adc_channel, this->adc_width, &buff);
                    meas += buff;
                } while (cnt--);
                *result = meas/this->n_meas;
            }
        }
        if ((this->value < (uint16_t)(*result) - this->meas_tolerance) || (this->value > (uint16_t)(*result) + this->meas_tolerance)) {
            this->value = (uint16_t)(*result);
            return true;
        }
        return false;
    }

    bool AnalogPin::has_changed() {
        int result = 0;
        return this->read(&result);
    }

    uint16_t AnalogPin::get_value() {
        return this->value;
    }

}
