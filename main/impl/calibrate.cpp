/** \file calibrate.cpp
 *  \brief Implementation file for calibration routines.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */

#include "calibrate.h"

#include "lvglpp/core/display.h"
#include "lvglpp/widgets/line.h"
#include "lvglpp/widgets/label.h"
#include "lvglpp/widgets/button.h"
#include "lvglpp/misc/style.h"
#include "lvglpp/misc/anim.h"

#include <mutex>

extern std::mutex mtx;

namespace eobsws::impl {

    void calibrate_touch(hardware::input::ResistiveTouchPanel & touch, std::shared_ptr<storage::NVStorage> nvs) {
        using namespace lvgl::widgets;
        using namespace lvgl::core;
        using namespace lvgl::misc;
        // touch panel set-up for calibration
        touch.disable_touch_interrupt();
        // draw cross and text
        mtx.lock();
        auto root = scr_act();
        root.set_style_bg_color(palette::white(), LV_PART_MAIN);
        auto cont = std::make_unique<Container>(root);
        cont->remove_style_all();
        auto w = root.get_width(), h = root.get_height();
        cont->set_size(w, h);
        cont->center();
        auto label = Label(*cont);
        label.set_text("Calibration\nClick on cross centre.");
        label.set_align(LV_TEXT_ALIGN_CENTER);
        label.center();
        // line style
        Style style;
        style.set_line_width(3);
        style.set_line_color(palette::black());
        style.set_line_rounded(true);
        lv_coord_t cw = std::max(w/10, h/10), ch = cw; // cross size
        lv_coord_t cx = 5 + cw/2, cy = 5 + ch/2; // position of cross centre
        auto cross = Container(*cont);
        cross.remove_style_all();
        cross.set_size(cw, ch);
        cross.set_pos(cx-cw/2,cy-ch/2);
        auto line1 = Line(cross);
        auto line2 = Line(cross);
        std::vector<lv_point_t> l1_pts{{0, ch}, {cw, 0}};
        std::vector<lv_point_t> l2_pts{{0, 0}, {cw, ch}};
        line1.set_points(l1_pts);
        line2.set_points(l2_pts);
        line1.add_style(style, LV_PART_MAIN);
        line2.add_style(style, LV_PART_MAIN);
        mtx.unlock();
        // record tap positions; start with upper left corner
        while (touch.read_touch_pressure()<CONFIG_TOUCH_TRIG_PRESSURE)
            vTaskDelay(10/portTICK_PERIOD_MS);
        // store tap position
        auto p1x = touch.read_x_position();
        auto p1y = touch.read_y_position();
        // move cross to lower right corner
        cross.set_pos(w-cx-cw/2, h-cy-ch/2);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        while (touch.read_touch_pressure()<CONFIG_TOUCH_TRIG_PRESSURE)
            vTaskDelay(10/portTICK_PERIOD_MS);
        // turn background black
        root.set_style_bg_color(palette::black(), LV_PART_MAIN);
        // store tap position
        auto p2x = touch.read_x_position();
        auto p2y = touch.read_y_position();
        // compute new scaling and offset
        auto scaling_x = 1000*(w - 2*cx)/(p2x - p1x);
        auto scaling_y = 1000*(h - 2*cy)/(p2y - p1y);
        auto offset_x = ((w-cx)*p1x - cx*p2x)/(p1x - p2x);
        auto offset_y = ((h-cy)*p1y - cy*p2y)/(p1y - p2y);
        // save values
        nvs->set_item<int16_t>("screen", "touch_scaling_x", scaling_x);
        nvs->set_item<int16_t>("screen", "touch_scaling_y", scaling_y);
        nvs->set_item<int16_t>("screen", "touch_offset_x", offset_x);
        nvs->set_item<int16_t>("screen", "touch_offset_y", offset_y);
        // clean up
        mtx.lock();
        cont = nullptr;
        mtx.unlock();
    }


    void calibrate_potentiometers(
        std::vector<std::unique_ptr<hardware::AnalogPin>> & pins,
        std::shared_ptr<storage::NVStorage> nvs) {
        using namespace lvgl::widgets;
        using namespace lvgl::core;
        // draw text
        mtx.lock();
        auto root = scr_act();
        root.set_style_bg_color(palette::white(), LV_PART_MAIN);
        root.set_scrollbar_mode(LV_SCROLLBAR_MODE_OFF);
        auto cont = std::make_unique<Container>(root);
        cont->remove_style_all();
        cont->set_size(root.get_width(), root.get_height());
        cont->center();
        auto label = Label(*cont);
        label.set_text("Calibration\n"
                       "Set both potentiometers\n"
                       "to minimum (turn counter-clockwise)\n"
                       "and press button.");
        label.set_align(LV_TEXT_ALIGN_CENTER);
        label.align(LV_ALIGN_CENTER, 0, -30);
        // add button
        auto button = Button(*cont);
        button.align_to(label, LV_ALIGN_BOTTOM_MID, 0, 40);
        button.set_style_pad_all(10, LV_PART_MAIN);
        auto blabel = Label(button);
        blabel.set_text("Ok");
        blabel.center();
        // set button callback that sets a bool to true
        auto bt_click_cb = [](lv_event_t* e) {
            *reinterpret_cast<bool*>(lv_event_get_user_data(e)) = true;
        };
        bool is_ready = false;
        button.add_event_cb(bt_click_cb, LV_EVENT_CLICKED, static_cast<void*>(&is_ready));
        mtx.unlock();
        // wait that button gets pressed
        while (!is_ready)
            vTaskDelay(10/portTICK_PERIOD_MS);
        // store state as min.
        pins[0]->has_changed(); // need to call that or value isn't updated
        pins[1]->has_changed();
        uint16_t min0 = pins[0]->get_value();
        uint16_t min1 = pins[1]->get_value();
        nvs->set_item<uint16_t>("potentiometer_0", "raw_min", min0);
        nvs->set_item<uint16_t>("potentiometer_1", "raw_min", min1);
        // define animation for step change;
        // this slides content out of screen for a short while
        // to give a visual cue
        auto a = std::make_unique<Animation>();
        a->set_var(*cont);
        a->set_values(0, 320);
        auto anim_cb = [](Container & c, int32_t v) {
            c.set_x(v);
        };
        a->set_exec_cb<Container>(anim_cb);
        a->set_path_cb(lv_anim_path_ease_in_out);
        a->set_time(300);
        a->set_repeat_count(0);
        a->set_playback_delay(100);
        a->set_playback_time(300);
        a->start();
        // define text for second text
        mtx.lock();
        label.set_text("Calibration\n"
                       "Set both potentiometers\n"
                       "to maximum (turn clockwise)\n"
                       "and press button.");
        mtx.unlock();
        // wait that button gets pressed
        is_ready = false;
        while (!is_ready)
            vTaskDelay(10/portTICK_PERIOD_MS);
        // store state as max.
        pins[0]->has_changed();
        pins[1]->has_changed();
        nvs->set_item<uint16_t>("potentiometer_0", "raw_max", std::max(pins[0]->get_value(), (uint16_t)(min0+1)));
        nvs->set_item<uint16_t>("potentiometer_1", "raw_max", std::max(pins[1]->get_value(), (uint16_t)(min1+1)));
        // slide content out of screen
        a->set_playback_delay(0);
        a->set_playback_time(0);
        a->start();
        vTaskDelay(500/portTICK_PERIOD_MS);
        // clean up
        mtx.lock();
        a = nullptr;
        cont = nullptr;
        mtx.unlock();
    }

}

