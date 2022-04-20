/** \file gui.cpp
 *  \brief Implementation file for GUI definition.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */

#include "freertos/FreeRTOS.h"

#include <memory>
#include <mutex>

#include "setup.h"
#include "gui.h"

extern std::mutex mtx;

namespace eobsws::impl {

    namespace gi = gui::image;

    void load_wifi_icons(std::shared_ptr<storage::SPIFlash> spiflash, GUIData & data) {
        for (size_t n=0; n<4; n++) {
            data.wifi_imgs.emplace_back(std::make_shared<gi::LvImagePNG>(
                spiflash, "images/wifi_" + std::to_string(n) + ".png"));
        }
        data.wifi_icon = std::make_unique<gui::widgets::ImagePNG>(*data.root);
        data.wifi_icon->set_size(20, 20);
        data.wifi_icon->set_pos(295, 215);
    }


    void load_battery_icons(std::shared_ptr<storage::SPIFlash> spiflash, GUIData & data) {
        for (size_t n=0; n<6; n++) {
            data.battery_imgs.emplace_back(std::make_shared<gi::LvImagePNG>(
                    spiflash, "images/battery_" + std::to_string(n) + ".png"));
        }
        data.battery_icon = std::make_unique<gui::widgets::ImagePNG>(*data.root);
        data.battery_icon->set_size(11, 20);
        data.battery_icon->set_pos(278, 215);
    }


    void draw_buttons(std::shared_ptr<comm::DataBroker> db,
                      std::shared_ptr<storage::SPIFlash> spiflash,
                      std::vector<ButtonConfiguration> & cfgs, GUIData & data) {
        /* for now we have 6 buttons we simple functions. Their configuration is
         * stored in NVS in namespace 'button_n' and numbered from 0 to 5. For button n,
         * we have:
         *  type = ButtonType::PushButton or ButtonType::ToggleButton.
         *  image_off = path to image in toggled-off state or released state.
         *  image_on = path to image in toggled-on state or pressed state.
         *  command_on = command triggered when pressed or toggled on; for push buttons,
         *               this is the only command necessary.
         *  command_off = command triggered when toggled off (only for toggle buttons).
         */
        // transition parameters for click effect
        std::vector<lv_style_prop_t> props{
            LV_STYLE_IMG_RECOLOR, LV_STYLE_IMG_RECOLOR_OPA, LV_STYLE_PROP_INV};
        auto transition = std::make_shared<lvgl::misc::LinearStyleTransition>(props, 300, 0);
        data.lvgl_transitions.push_back(transition);
        // create buttons
        for (uint8_t ny=0; ny<2; ny++) {
            for (uint8_t nx=0; nx<3; nx++) {
                uint8_t n = ny*3 + nx;
                // create button instance
                std::shared_ptr<gui::widgets::ImageButtonPNG> btn;
                if (cfgs[n].type == ButtonType::PushButton) {
                    btn = std::make_shared<gui::widgets::ImageButtonPNG>(*data.root);
                } else if (cfgs[n].type == ButtonType::ToggleButton) {
                    btn = std::make_shared<gui::widgets::ImageToggleButtonPNG>(*data.root);
                }
                // image for off state
                auto img_bg_off = std::make_shared<gi::LvglDecorator<gi::ImagePNG>>(spiflash, cfgs[n].image_off);
                btn->set_src(gui::widgets::ImagePosition::Left, LV_IMGBTN_STATE_RELEASED, img_bg_off);
                btn->set_src(gui::widgets::ImagePosition::Left, LV_IMGBTN_STATE_PRESSED, img_bg_off);
                // for toggle button, set toggled-on image
                if (cfgs[n].type == ButtonType::ToggleButton) {
                    auto img_bg_on = std::make_shared<gi::LvglDecorator<gi::ImagePNG>>(spiflash, cfgs[n].image_on);
                    btn->set_src(gui::widgets::ImagePosition::Left, LV_IMGBTN_STATE_CHECKED_RELEASED, img_bg_on);
                    btn->set_src(gui::widgets::ImagePosition::Left, LV_IMGBTN_STATE_CHECKED_PRESSED, img_bg_on);
                    btn->add_flag(LV_OBJ_FLAG_CHECKABLE);
                }
                // set button functions
                // -> data broker in charge of processing events
                btn->set_data_broker(db);
                // -> on which channel
                btn->set_message_type(comm::MessageType::OutboundWireless);
                // -> event message(s)
                if (cfgs[n].type == ButtonType::ToggleButton) {
                    auto tgbtn = std::reinterpret_pointer_cast<gui::widgets::ImageToggleButtonPNG>(btn);
                    tgbtn->set_message_data(cfgs[n].command_on, true);
                    tgbtn->set_message_data(cfgs[n].command_off, false);
                    tgbtn->set_trigger(LV_EVENT_CLICKED);
                } else {
                    btn->set_message_data(cfgs[n].command_on);
                    btn->set_trigger(LV_EVENT_CLICKED);
                }
                // styling; buttons are 100x100 pixels
                btn->set_size(100, 100);
                // we keep a 5px horizontal gap and a 4px vertical gap
                btn->set_pos(5+105*nx,4+104*ny);
                // released style
                auto released_style = std::make_shared<lvgl::misc::Style>(); 
                // set transition profile 
                released_style->set_transition(*transition);
                // pressed style with configured color
                auto pressed_style = std::make_shared<lvgl::misc::Style>();
                pressed_style->set_img_recolor_opa(cfgs[n].event_opacity);
                pressed_style->set_img_recolor(cfgs[n].event_color);
                pressed_style->set_transition(*transition);
                // store styles in storage
                data.lvgl_styles.push_back(released_style);
                data.lvgl_styles.push_back(pressed_style);
                // apply style to button pressed state
                btn->add_style(*released_style, LV_STATE_DEFAULT);
                btn->add_style(*pressed_style, LV_STATE_PRESSED);
                // store button
                data.buttons.push_back(std::move(btn));
            }
        }
    }


    void draw_bars(Configuration & cfg, GUIData & data) {
        for (uint8_t n=0; n<2; n++) {
            auto idx_str = std::to_string(n);
            data.bars.emplace_back(std::make_unique<lvgl::widgets::Bar>(*data.root));
            // set bar range
            data.bars[n]->set_range(cfg.pots[n].raw_min, cfg.pots[n].raw_max);
            // set position and size
            data.bars[n]->set_size(130, 20);
            data.bars[n]->set_pos(5 + 135*n, 215);
            // background style
            auto bg_style = std::make_shared<lvgl::misc::Style>();
            bg_style->set_bg_color(cfg.pots[n].bg_color);
            bg_style->set_bg_opa(cfg.pots[n].bg_opacity);
            data.bars[n]->add_style(*bg_style.get(), LV_PART_MAIN);
            data.lvgl_styles.push_back(bg_style);
            // foreground style
            auto fg_style = std::make_shared<lvgl::misc::Style>();
            fg_style->set_bg_color(cfg.pots[n].fg_color);
            fg_style->set_bg_opa(cfg.pots[n].fg_opacity);
            data.bars[n]->add_style(*fg_style, LV_PART_INDICATOR);
            data.lvgl_styles.push_back(fg_style);
        }
    }


    void draw_wifi_icon(GUIData & data, int8_t rssi) {
        // choose icon based on RSSI value
        uint8_t idx = 0; // by default, no connection
        if (rssi >= -90 && rssi < -70) {
            // poor connection
            idx = 1;
        } else if (rssi >= -70 && rssi < -50) {
            // average
            idx = 2;
        } else if (rssi >= -50 && rssi < 0) {
            // good
            idx = 3;
        }
        std::lock_guard<std::mutex> guard(mtx);
        data.wifi_icon->set_src(data.wifi_imgs[idx]);
    }


    void draw_battery_icon(GUIData & data,
                           uint8_t value,
                           bool charging) {
        // choose battery icon based on battery percentage level
        uint8_t idx = std::min(4, value / 20);
        if (charging) idx = 5;
        std::lock_guard<std::mutex> guard(mtx);
        data.battery_icon->set_src(data.battery_imgs[idx]);
    }


    void display_task(void* arg) {
        for (;;) {
            mtx.lock();
            lv_task_handler();
            mtx.unlock();
            vTaskDelay(10/portTICK_PERIOD_MS);
        }
    }

    void tick_task(void* arg) {
        for (;;) {
            lv_tick_inc(10);
            vTaskDelay(10/portTICK_PERIOD_MS);
        }
    }

}