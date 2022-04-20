/** \file touch_lvgl.h
 *  \brief Header file for bindings of touch panel driver with LVGL.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

#include "touch.h"
#include "lvglpp/core/indev.h"

namespace eobsws::hardware::input {

    /** \class TouchpadLVGL
     *  \brief Class binding a touchpad driver with LVGL touchpad input routines
     */
    class TouchpadLVGL : public lvgl::core::PointerInputDevice {
    private:
        /** \fn void read(lv_indev_data_t * data) override
         *  \brief Callback for LVGL driver to read touchpad data.
         *  \param data: pointer to the LVGL-provided data recipient.
         */
        void read(lv_indev_data_t * data) override;

    public:
        /** \property std::shared_ptr<TouchPanel> device
         *  \brief Pointer to the underlying device.
         */
        std::shared_ptr<TouchPanel> device;

        /** \fn TouchpadLVGL(std::shared_ptr<TouchPanel> device)
         *  \brief Constructor.
         *  \param device: pointer to a device driver instance.
         */
        TouchpadLVGL(std::shared_ptr<TouchPanel> device);

    };
}
