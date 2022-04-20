/** \file touch_lvgl.cpp
 *  \brief Class binding a touch panel driver with LVGL.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "touch_lvgl.h"
#include "esp_log.h"

namespace eobsws::hardware::input {

    TouchpadLVGL::TouchpadLVGL(std::shared_ptr<TouchPanel> device) : device(device) {}

    void TouchpadLVGL::read(lv_indev_data_t * data) {
        if (this->device->get_touch_occurred()) {
            uint16_t tp_data[3];
            this->device->get_data(tp_data);
            data->point.x = tp_data[0];
            data->point.y = tp_data[1];
            ESP_LOGI("LVGL::Touchpad", "click position: %d, %d", tp_data[0], tp_data[1]);
            this->device->reset_touch_occurred_flag();
            data->state = LV_INDEV_STATE_PRESSED;
        } else {
            data->state = LV_INDEV_STATE_RELEASED;
        }
        data->continue_reading = false;
    }

}
