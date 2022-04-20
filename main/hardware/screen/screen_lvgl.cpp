/** \file screen_lvgl.cpp
 *  \brief Class binding a generic screen driver with LVGL.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "screen_lvgl.h"
#include "esp_log.h"

namespace eobsws::hardware::screen {
    ScreenLVGL::ScreenLVGL(std::shared_ptr<Screen> screen)
        : Display(screen->get_screen_width(), screen->get_screen_height(), screen->get_framebuffer_size()) {
        this->screen = screen;
    }

    void ScreenLVGL::flush(const lv_area_t * area,  lv_color_t * color_map) {
        ESP_LOGI("LVGL::Screen", "flush started...");
    #if LV_COLOR_DEPTH == 32
        // if LVGL is configured to work with RGBA8888 data, buffer is already in the right format
        this->screen->paint_area(area->x1, area->y1, area->x2, area->y2, (uint32_t*)color_map);
    #else
        // Screen driver takes RGBA8888 data => need to convert
        std::size_t area_size = (area->x2-area->x1+1)*(area->y2-area->y1+1);
        uint32_t * buffer = new uint32_t[area_size--];
        do {
            buffer[area_size] = lv_color_to32(color_map[area_size]);
        } while (area_size--);
        this->screen->paint_area(area->x1, area->y1, area->x2, area->y2, buffer);
        delete [] buffer;
    #endif
        this->flush_ready();
        ESP_LOGI("LVGL::Screen", "flush done.");
    }

}