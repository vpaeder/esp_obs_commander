/** \file calibrate.h
 *  \brief Header file for calibration routines.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "storage/nvs.h"
#include "hardware/analog_pin.h"
#include "hardware/input/res_touch.h"
#include <vector>

namespace eobsws::impl {
    /** \fn void calibrate_touch(hardware::input::ResistiveTouchPanel & touch, std::shared_ptr<storage::NVStorage> nvs)
     *  \brief Calibrates touch screen.
     * 
     *  This displays a cross in the upper left corner, then in the
     *  lower right corner, and asks the user to tap in the centre of it
     *  to calculate the scaling functions between raw values and pixels.
     * 
     *  \param touch: the touch screen instance to calibrate.
     *  \param nvs: pointer to a non-volatile storage accessor.
     */
    void calibrate_touch(hardware::input::ResistiveTouchPanel & touch, std::shared_ptr<storage::NVStorage> nvs);

    /** \fn void calibrate_potentiometers(std::vector<std::unique_ptr<hardware::AnalogPin>> & pins, std::shared_ptr<storage::NVStorage> nvs)
     *  \brief Starts potentiometers calibration procedure. This requires a calibrated screen.
     * 
     *  This asks the user to first set potentiometers to minimum, then to maximum.
     * 
     *  \param pins: a vector containing pointers to two analog pin drivers.
     *  \param nvs: pointer to a non-volatile storage accessor.
     */
    void calibrate_potentiometers(std::vector<std::unique_ptr<hardware::AnalogPin>> & pins, std::shared_ptr<storage::NVStorage> nvs);
}
