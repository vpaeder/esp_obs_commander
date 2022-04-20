/** \file screen_lvgl.h
 *  \brief Header file for bindings of generic screen driver with LVGL.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "lvglpp/core/display.h"
#include "lvglpp/misc/color.h"
#include "screen.h"

namespace eobsws::hardware::screen {

    /** \class ScreenLVGL
     *  \brief Class binding a screen driver with LVGL display routines
     */
    class ScreenLVGL : public lvgl::core::Display {
    private:
        /** \fn void flush(const lv_area_t * area, lv_color_t * color_map)
         *  \brief Flush data to screen in the specified area.
         *  \param area: display area over which to flush given data
         *  \param color_map: data to flush to display
         */
        void flush(const lv_area_t * area, lv_color_t * color_map) override;

    public:
        /** \property std::shared_ptr<Screen> screen
         *  \brief Pointer to a screen driver instance
         */
        std::shared_ptr<Screen> screen;
        
        /** \fn ScreenLVGL(std::shared_ptr<Screen> screen)
         *  \brief Constructor.
         *  \param screen : pointer to a screen driver instance
         */
        ScreenLVGL(std::shared_ptr<Screen> screen);

    };

}
