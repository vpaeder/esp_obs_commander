/** \file esp_obs_cmd.cpp
 *  \brief Main routine.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include "freertos/FreeRTOS.h"

#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_system.h"

#include "impl/setup.h"
#include "impl/gui.h"
#include "impl/calibrate.h"
#include "hardware/digital_pin.h"

#include <mutex>

/** \var std::mutex mtx
 *  \brief Global mutex.
 */
std::mutex mtx;

using namespace eobsws;
using namespace eobsws::impl;

/** \fn void app_main(void)
 *  \brief Main routine.
 */
extern "C" void app_main(void) {
    // sets log level to info
    //esp_log_level_set("*", ESP_LOG_INFO);
    // initializes network interface
    esp_netif_init();
    esp_event_loop_create_default();
    // initializes non-volatile storage (NVS)
    auto nvs = std::make_shared<storage::NVStorage>(CONFIG_NVS_VOLUME_NAME);
    // loads configuration from NVS
    auto cfg = Configuration(nvs);
    // initializes LVGL
    lv_init();
    // sets up screen
    auto tft = setup_screen(cfg);
    // sets up touch panel
    auto touch = setup_touch(cfg);
    touch->set_display(tft->raw_ptr());
    // sets up GPIO pins
    auto gpio = setup_gpio();
    auto batchrg = hardware::DigitalPin();
    batchrg.set_pin(static_cast<gpio_num_t>(CONFIG_PIN_BATT_CHRG));
    batchrg.set_pin_direction(hardware::PinDirection::Input);
    batchrg.configure();
    // creates display update task
    xTaskCreate(display_task, "lv_display_task", 4096, nullptr, 10, nullptr);
    xTaskCreate(tick_task, "lv_tick_task", 1024, nullptr, 15, nullptr);
    // touch screen calibration
    if (!cfg.touch_calibrated) {
        auto tdev = std::dynamic_pointer_cast<hardware::input::ResistiveTouchPanel>(touch->device);
        calibrate_touch(*tdev, nvs);
        nvs->set_item<uint8_t>("calibration", "touch_ok", 1);
        esp_restart();
    }
    // sets up flash storage
    auto spiflash = setup_flash(cfg.storage_part_name);
    // initializes communication routines
    // -> creates data broker
    auto db = std::make_shared<comm::DataBroker>();
    // sets up UART handler
    UARTData udata;
    setup_uart(db, nvs, spiflash, udata);
    // potentiometers calibration
    // this is done after UART so that we can reset the calibration
    // flags in case of a faulty screen calibration
    if (!cfg.pots_calibrated) {
        calibrate_potentiometers(gpio, nvs);
        nvs->set_item<uint8_t>("calibration", "pots_ok", 1);
        esp_restart();
    }
    // sets up obs-websocket handler
    OBSData odata;
    setup_websocket(db, cfg, odata);
    // initializes GUI elements
    GUIData gdata;
    std::vector<ButtonConfiguration> bcfgs;
    bcfgs.reserve(6);
    for (uint8_t n=0; n<6; n++)
        bcfgs.emplace_back(ButtonConfiguration(nvs, n));
    auto screen_color = lvgl::misc::color::from_rgb(nvs->get_item<uint8_t>("screen","bg_color_r",0),
                                                    nvs->get_item<uint8_t>("screen","bg_color_g",0),
                                                    nvs->get_item<uint8_t>("screen","bg_color_b",0));
    // from that point on we'll modify the interface
    // => acquire the lock, otherwise LVGL may complain
    mtx.lock();
    gdata.root->set_style_bg_color(screen_color, LV_PART_MAIN);
    load_wifi_icons(spiflash, gdata);
    load_battery_icons(spiflash, gdata);
    draw_buttons(db, spiflash, bcfgs, gdata);
    draw_bars(cfg, gdata);
    mtx.unlock(); // unlocks here as the 2 next functions claim the lock
    draw_wifi_icon(gdata, 0);
    draw_battery_icon(gdata, 0, false);
    // creates WebSocket connection task
    auto fconnect = [](void* arg) {
      auto obj = reinterpret_cast<comm::pipe::WebSocketPipe*>(arg);
      obj->connect();
      vTaskDelete(nullptr);
    };
    xTaskCreate(fconnect, "ws_connect_task", 8192, static_cast<void*>(odata.ws_pipe.get()), 15, nullptr);
    // main loop - takes care of a number of basic things:
    //  1) reads potentiometers, updates bars and transmits data if necessary
    //  2) reads WiFi RSSI and updates icon
    //  3) reads battery level and updates icon
    char buffer[256]; // text buffer for outbound messages
    bool screen_active = true; // if true, screen is active
    for (;;) {
        // reads potentiometers
        for (uint8_t n = 0; n<2; n++) {
            if (gpio[n]->has_changed()) {
                // tells display to go active
                tft->trig_activity();
                // updates associated bar
                mtx.lock();
                gdata.bars[n]->set_value(gpio[n]->get_value(), LV_ANIM_OFF);
                mtx.unlock();
                // converts from raw value
                float value = static_cast<float>(gpio[n]->get_value()*(cfg.pots[n].obs_max - cfg.pots[n].obs_min))
                              /static_cast<float>((cfg.pots[n].raw_max - cfg.pots[n].raw_min)*cfg.pots[n].divider)
                              +static_cast<float>(cfg.pots[n].obs_min*cfg.pots[n].raw_max - cfg.pots[n].obs_max*cfg.pots[n].raw_min)
                              /static_cast<float>((cfg.pots[n].raw_max - cfg.pots[n].raw_min)*cfg.pots[n].divider);
                // generates command - if succesfull, sends data
                auto nchr = sprintf(buffer, cfg.pots[n].command.c_str(), value);
                if (nchr>0)
                    db->publish(comm::MessageType::OutboundWireless, std::string(buffer, nchr));
            }
        }
        // reads WiFi RSSI and updates icon
        draw_wifi_icon(gdata, odata.ws_pipe->get_rssi());
        // reads battery level and updates icon if necessary;
        // battery level gets read with a /2 voltage divider, and processed
        // by ADC with 6dB attenuation -> scale should be about (1000, 1100).
        if (gpio[2]->has_changed()) {
            draw_battery_icon(gdata,
                              (gpio[2]->get_value()-cfg.battery_min)*100/(cfg.battery_max-cfg.battery_min),
                              batchrg.read());
        }
        // checks display inactivity to dim backlight if necessary
        if (cfg.bl_dim_delay && tft->get_inactive_time()>cfg.bl_dim_delay && screen_active) {
            std::dynamic_pointer_cast<eobsws::hardware::screen::ST7789VI_TFT>(tft->screen)->set_backlight_level(cfg.bl_lvl_dimmed, 1000);
            screen_active = false;
        } else if (tft->get_inactive_time()<=cfg.bl_dim_delay && !screen_active) {
            std::dynamic_pointer_cast<eobsws::hardware::screen::ST7789VI_TFT>(tft->screen)->set_backlight_level(cfg.bl_lvl_act, 100);
            screen_active = true;
        }
        // waits for 100ms
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}
