/** \file types.cpp
 *  \brief Implementation file for data types.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "types.h"

namespace eobsws::impl {

    Configuration::Configuration(std::shared_ptr<storage::NVStorage> nvs) {
        // flash storage partition name
        this->storage_part_name = nvs->get_string("storage", "part_name", CONFIG_VFAT_VOLUME_NAME);
        // WiFi settings
        this->wifi_ssid = nvs->get_string("wifi", "ssid", CONFIG_WIFI_SSID);
        this->wifi_password = nvs->get_string("wifi", "password", CONFIG_WIFI_PASSWORD);
        // WebSockets settings
        this->websocket_host = nvs->get_string("websocket", "host", CONFIG_WEBSOCKET_HOST);
        this->websocket_port = nvs->get_item("websocket", "port", CONFIG_WEBSOCKET_PORT);
        this->websocket_path = nvs->get_string("websocket", "path", CONFIG_WEBSOCKET_PATH);
        // screen settings
        this->screen_orientation = static_cast<lv_disp_rot_t>(
            nvs->get_item<uint8_t>("screen", "orientation", 0) << 1);
        // backlight dimming
        this->bl_lvl_dimmed = 1024 - nvs->get_item<uint16_t>("screen", "bl_lvl_dimmed", 0);
        this->bl_lvl_act = 1024 - nvs->get_item<uint16_t>("screen", "bl_lvl_act", 0);
        this->bl_dim_delay = nvs->get_item<uint32_t>("screen", "bl_dim_delay", 10000);
        // touch panel calibration values
        this->touch_calibrated = nvs->get_item<uint8_t>("calibration", "touch_ok", 0) == 1;
        this->touch_scaling_x = nvs->get_item<int16_t>("screen", "touch_scaling_x", 1000);
        this->touch_scaling_y = nvs->get_item<int16_t>("screen", "touch_scaling_y", 1000);
        this->touch_offset_x = nvs->get_item<int16_t>("screen", "touch_offset_x", 0);
        this->touch_offset_y = nvs->get_item<int16_t>("screen", "touch_offset_y", 0);
        // battery calibration values
        this->battery_min = nvs->get_item<uint16_t>("battery", "raw_min", this->battery_min);
        this->battery_max = nvs->get_item<uint16_t>("battery", "raw_max", this->battery_max);
        // potentiometers settings
        this->pots_calibrated = nvs->get_item<uint8_t>("calibration", "pots_ok", 0) == 1;
        this->pots.reserve(2);
        for (size_t n=0; n<2; n++) {
            std::string idx_str = "potentiometer_" + std::to_string(n);
            this->pots.push_back(PotentiometerConfiguration());
            // calibration values
            this->pots[n].raw_min = nvs->get_item<uint16_t>(idx_str, "raw_min", 0);
            this->pots[n].raw_max = nvs->get_item<uint16_t>(idx_str, "raw_max", 1000);
            // range accepted by OBS
            this->pots[n].obs_min = nvs->get_item<int16_t>(idx_str, "obs_min", 0);
            this->pots[n].obs_max = nvs->get_item<int16_t>(idx_str, "obs_max", 20);
            // this is a divider if we want to target values smaller than 1;
            // e.g. if it's set to 1000 and obs_max to 1, we have an effective obs_max=1e-3
            this->pots[n].divider = nvs->get_item<uint16_t>(idx_str, "divider", 1);
            // command template in which value gets inserted
            this->pots[n].command = nvs->get_string(idx_str, "command", "%0.2f");
            // indicator bar colors
            using namespace lvgl::misc::color;
            this->pots[n].bg_color = from_rgb(nvs->get_item<uint8_t>(idx_str,"bg_color_r",0),
                                              nvs->get_item<uint8_t>(idx_str,"bg_color_g",0),
                                              nvs->get_item<uint8_t>(idx_str,"bg_color_b",0));
            this->pots[n].fg_color = from_rgb(nvs->get_item<uint8_t>(idx_str,"fg_color_r",255),
                                              nvs->get_item<uint8_t>(idx_str,"fg_color_g",255),
                                              nvs->get_item<uint8_t>(idx_str,"fg_color_b",255));
            this->pots[n].bg_opacity = static_cast<lv_opa_t>(nvs->get_item<uint8_t>(idx_str,"bg_color_a",255));
            this->pots[n].fg_opacity = static_cast<lv_opa_t>(nvs->get_item<uint8_t>(idx_str,"fg_color_a",255));
        }
    }


    ButtonConfiguration::ButtonConfiguration(std::shared_ptr<storage::NVStorage> nvs, uint8_t idx) {
        // namespace in NVS where to search
        auto idx_str = "button_" + std::to_string(idx);
        // image for released/toggled-off state
        this->image_off = nvs->get_string(idx_str, "image_off", "");
        // image for toggled-on state
        this->image_on = nvs->get_string(idx_str, "image_on", "");
        // command sent when pressed/toggled on
        this->command_on = nvs->get_string(idx_str, "command_on", "");
        // button type
        this->type = nvs->get_item(idx_str, "type", ButtonType::PushButton);
        // if it's a toggle button, get command when toggled off
        if (this->type == ButtonType::ToggleButton)
            this->command_off = nvs->get_string(idx_str, "command_off", "");
        // this is the color of the glow that is used as a visual cue when pressed
        this->event_color = lvgl::misc::color::from_rgb(
            nvs->get_item<uint8_t>(idx_str,"event_color_r",0),
            nvs->get_item<uint8_t>(idx_str,"event_color_g",0),
            nvs->get_item<uint8_t>(idx_str,"event_color_b",0));
        this->event_opacity = nvs->get_item<uint8_t>(idx_str,"event_color_a",LV_OPA_30);
    }


}