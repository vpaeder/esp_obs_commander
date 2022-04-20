/** \file setup.h
 *  \brief Header file for global configuration routines.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "sdkconfig.h"
#include "hardware/screen/st7789vi.h"
#include "hardware/screen/screen_lvgl.h"
#include "hardware/input/res_touch.h"
#include "hardware/input/touch_lvgl.h"
#include "hardware/analog_pin.h"
#include "types.h"

/** \namespace eobsws::impl
 *  \brief User interface implementation.
 */
namespace eobsws::impl {

    /** \fn std::unique_ptr<hardware::screen::ScreenLVGL> setup_screen(const Configuration & cfg)
     *  \brief Sets up the screen driver.
     *  \param cfg: configuration storage instance.
     *  \returns a pointer to a screen driver instance.
     */
    std::unique_ptr<hardware::screen::ScreenLVGL> setup_screen(const Configuration & cfg);

    /** \fn std::unique_ptr<hardware::input::TouchpadLVGL> setup_touch(const Configuration & cfg)
     *  \brief Sets up the touch panel driver.
     *  \param cfg: configuration storage instance.
     *  \returns a pointer to a touch panel driver instance.
     */
    std::unique_ptr<hardware::input::TouchpadLVGL> setup_touch(const Configuration & cfg);

    /** \fn std::vector< std::unique_ptr<hardware::AnalogPin> > setup_gpio()
     *  \brief Sets up the analog GPIO pin drivers.
     *  \returns a container with analog pin driver instances.
     */
    std::vector<std::unique_ptr<hardware::AnalogPin>> setup_gpio();

    /** \fn std::shared_ptr<storage::SPIFlash> setup_flash(const std::string & part_name)
     *  \brief Sets up the flash storage partition.
     *  \param part_name: flash partition name.
     *  \returns a pointer to a flash partition driver instance.
     */
    std::shared_ptr<storage::SPIFlash> setup_flash(const std::string & part_name);

    /** \fn void setup_uart(std::shared_ptr<comm::DataBroker> db,
     *                      std::shared_ptr<storage::NVStorage> nvs,
     *                      std::shared_ptr<storage::SPIFlash> spiflash,
     *                      UARTData & udata)
     *  \brief Sets up the UART communication handler.
     *  \param db: data broker assigned to buttons to issue commands.
     *  \param nvs: pointer to a non-volatile storage accessor.
     *  \param spiflash: pointer to a flash storage partition containing image files.
     *  \param udata: container for UART handler.
     */
    void setup_uart(std::shared_ptr<comm::DataBroker> db,
                    std::shared_ptr<storage::NVStorage> nvs,
                    std::shared_ptr<storage::SPIFlash> spiflash,
                    UARTData & udata);

    /** \fn void setup_websocket(std::shared_ptr<comm::DataBroker> db,
     *                           const Configuration & cfg,
     *                           OBSData & odata)
     *  \brief Sets up the obs-websocket handler.
     *  \param db: data broker assigned to buttons to issue commands.
     *  \param cfg: configuration storage instance.
     *  \param odata: container for obs-websocket handler.
     */
    void setup_websocket(std::shared_ptr<comm::DataBroker> db,
                         const Configuration & cfg,
                         OBSData & odata);

}

