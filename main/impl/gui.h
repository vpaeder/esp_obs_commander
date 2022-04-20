/** \file gui.h
 *  \brief Header file for GUI definition.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "lvglpp/widgets/image.h"
#include "types.h"

namespace eobsws::impl {
    /** \fn void load_wifi_icons(std::shared_ptr<storage::SPIFlash> spiflash, GUIData & data)
     *  \brief Loads WiFi indicator icons from flash.
     *  \param spiflash: pointer to a flash storage partition.
     *  \param data: storage object for GUI data.
     */
    void load_wifi_icons(std::shared_ptr<storage::SPIFlash> spiflash, GUIData & data);

    /** \fn void load_battery_icons(std::shared_ptr<storage::SPIFlash> spiflash, GUIData & data)
     *  \brief Loads battery level indicator icons from flash.
     *  \param spiflash: pointer to a flash storage partition.
     *  \param data: storage object for GUI data.
     */
    void load_battery_icons(std::shared_ptr<storage::SPIFlash> spiflash, GUIData & data);

    /** \fn void draw_buttons(std::shared_ptr<comm::DataBroker> db,
     *                        std::shared_ptr<storage::SPIFlash> spiflash,
     *                        std::vector<ButtonConfiguration> & cfgs, GUIData & data);
     *  \brief Draw buttons.
     *  \param db: data broker assigned to buttons to issue commands.
     *  \param spiflash: pointer to a flash storage partition containing image files.
     *  \param cfgs: storage object for buttons configuration data.
     *  \param data: storage object for GUI data.
     */
    void draw_buttons(std::shared_ptr<comm::DataBroker> db,
                      std::shared_ptr<storage::SPIFlash> spiflash,
                      std::vector<ButtonConfiguration> & cfgs, GUIData & data);

    /** \fn void draw_bars(Configuration & cfg, GUIData & data)
     *  \brief Draws indicator bars.
     *  \param cfg: configuration storage instance.
     *  \param data: storage object for GUI data.
     */
    void draw_bars(Configuration & cfg, GUIData & data);

    /** \fn void draw_wifi_icon(GUIData & data, int8_t rssi)
     *  \brief Draws WiFi indicator icon for given RSSI value.
     *  \param data: storage object for GUI data.
     *  \param rssi: WiFi RSSI value.
     */
    void draw_wifi_icon(GUIData & data, int8_t rssi);

    /** \fn void draw_battery_icon(GUIData & data, uint8_t value, bool charging)
     *  \brief Draws battery level indicator icon for given value.
     *  \param data: storage object for GUI data.
     *  \param value: raw battery level value.
     *  \param charging: set to true if battery is charging, false otherwise.
     */
    void draw_battery_icon(GUIData & data,
                           uint8_t value,
                           bool charging);

    /** \fn void display_task(void* arg)
     *  \brief Display update task.
     *  \param arg: user-defined argument.
     */
    void display_task(void* arg);

    /** \fn void tick_task(void* arg)
     *  \brief LVGL tick update task.
     *  \param arg: user-defined argument.
     */
    void tick_task(void* arg);
}