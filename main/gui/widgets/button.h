/** \file button.h
 *  \brief Header file for button widget class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include "lvglpp/widgets/button/button.h"
#include "widget.h"

namespace eobsws::gui::widgets {

    /** \class Button
     *  \brief LVGL Button class with event handler.
     */
    class Button : public Widget<lvgl::widgets::Button> {
    public:
        using Widget::Widget;

        /** \fn void publish(lv_event_t * e)
         *  \brief This is the action triggered when the button gets pressed.
         *  \param e: event data.
         */
        void publish(lv_event_t * e) override {
            ESP_LOGI("Button::publish", "Publishing: %s", this->message_data.c_str());
            lv_event_code_t code = lv_event_get_code(e);
            if(code == LV_EVENT_CLICKED || code == LV_EVENT_RELEASED) {
                this->db->publish(this->message_type,
                                  comm::parser::obs::add_request_id(this->message_data));
            }
        }

    };

}
