/** \file digital_pin.cpp
 *  \brief Class handling reading from and writing to a digital pin.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "digital_pin.h"

namespace eobsws::hardware {

    void DigitalPin::configure() {
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = this->pin_dir == PinDirection::Input ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT;
        io_conf.pull_down_en = static_cast<gpio_pulldown_t>(this->pull_down ? 1 : 0);
        io_conf.pull_up_en = static_cast<gpio_pullup_t>(this->pull_up ? 1 : 0);
        io_conf.pin_bit_mask = 1ULL << this->pin_num;
        gpio_config(&io_conf);
        this->initialized = true;
    }

    void DigitalPin::set_pin(const gpio_num_t pin_num) {
        this->pin_num = pin_num;
    }

    void DigitalPin::set_pin_direction(const PinDirection dir) {
        this->pin_dir = dir;
    }

    void DigitalPin::set_pull_down(const bool state) {
        this->pull_down = state;
    }

    void DigitalPin::set_pull_up(const bool state) {
        this->pull_up = state;
    }

    bool DigitalPin::read() const {
        if (!this->initialized) return false;
        return gpio_get_level(this->pin_num) == 0 ? false : true;
    }

    void DigitalPin::write(const bool value) {
        if (!this->initialized) return;
        gpio_set_level(this->pin_num, value ? 1 : 0);
    }

}
