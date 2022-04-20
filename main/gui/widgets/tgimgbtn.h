/** \file tgimgbtn.h
 *  \brief Header file for image toggle button widget class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include "esp_log.h"

#include "imgbtn.h"

namespace eobsws::gui::widgets {

    /** \brief LVGL ImageButton class with event handler. This is the toggle button version.
     *  \tparam ImageClass: image class supported by widget.
     */
    template <class ImageClass> class ImageToggleButton : public ImageButton<ImageClass> {
    protected:
        /** \property std::string message_data_off
         *  \brief Message issued when button is toggled off.
         */
        std::string message_data_off = "";

    public:
        using ImageButton<ImageClass>::ImageButton;

        /** \fn void publish(lv_event_t * e)
         *  \brief This is the action triggered when the button gets toggled.
         *  \param e: event data.
         */
        void publish(lv_event_t * e) override {
            if(this->get_state() & LV_STATE_CHECKED) {
                ESP_LOGI("ImageToggleButton", "got toggle-on event; sending data: %s", this->message_data.c_str());
                this->db->publish(this->message_type, this->message_data);
            } else {
                ESP_LOGI("ImageToggleButton", "got toggle-off event; sending data: %s", this->message_data_off.c_str());
                this->db->publish(this->message_type, this->message_data_off);
            }
        }

        /** \fn void set_message_data(const std::string & data)
         *  \brief Set message issued when widget action gets triggered.
         *  \param data: message content.
         *  \param toggle_state: if true, sets message data for on state, if false for off state. Defaut: true.
         */
        void set_message_data(const std::string & data, bool toggle_state) {
            if (toggle_state) {
                this->message_data = data;
            } else {
                this->message_data_off = data;
            }
        }

    };

    /** \typedef ImageToggleButtonPNG
     *  \brief Shorthand for ImageToggleButton<gui::image::ImagePNG>
     */
    using ImageToggleButtonPNG = ImageToggleButton<gui::image::ImagePNG>;
}
