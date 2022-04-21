/** \file setup.cpp
 *  \brief Implementation file for global configuration routines.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */

#include "setup.h"
#include "esp_log.h"

namespace eobsws::impl {

    std::unique_ptr<hardware::screen::ScreenLVGL> setup_screen(const Configuration & cfg) {
        // set up screen driver; settings can be changed with idf.py menuconfig
        auto tft_cfg = std::make_unique<hardware::screen::ST7789VI_Configuration>();
        tft_cfg->spi_host = HSPI_HOST;
        tft_cfg->pin_reset = static_cast<gpio_num_t>(CONFIG_PIN_TFT_RESX),
        tft_cfg->pin_clock = static_cast<gpio_num_t>(CONFIG_PIN_TFT_DCX);
        tft_cfg->pin_miso = static_cast<gpio_num_t>(CONFIG_PIN_TFT_SDO);
        tft_cfg->pin_mosi = static_cast<gpio_num_t>(CONFIG_PIN_TFT_SDA);
        tft_cfg->pin_chip_select = static_cast<gpio_num_t>(CONFIG_PIN_TFT_CSX);
        tft_cfg->pin_dc = static_cast<gpio_num_t>(CONFIG_PIN_TFT_WRX);
        tft_cfg->pin_backlight = static_cast<gpio_num_t>(CONFIG_PIN_TFT_BKLT);
        tft_cfg->spi_clock_rate = CONFIG_SPI_CLOCK;
        tft_cfg->spi_max_transfer_size = CONFIG_SPI_MAX_TRANSFER_SIZE;
        tft_cfg->spi_queue_length = CONFIG_SPI_QUEUE_LENGTH;
        tft_cfg->screen_width = CONFIG_TFT_SCREEN_WIDTH;
        tft_cfg->screen_height = CONFIG_TFT_SCREEN_HEIGHT;
        // create screen driver instance
        auto tft = std::make_unique<hardware::screen::ST7789VI_TFT>(tft_cfg);
        tft->initialize();
        // screen orientation
        if (cfg.screen_orientation == LV_DISP_ROT_NONE) {
            // with potentiometers on the right
            tft->set_orientation(2);
        } else {
            // with potentiometers on the left
            tft->set_orientation(1);
        }
        // fill with black
        tft->paint_screen(0);
        // set to active backlight level
        tft->set_backlight_level(cfg.bl_lvl_act, 500);
        // create LVGL wrapper
        auto ret = std::make_unique<hardware::screen::ScreenLVGL>(std::move(tft));
        ret->set_rotation(cfg.screen_orientation);
        ret->set_default();
        return ret;
    }


    std::unique_ptr<hardware::input::TouchpadLVGL> setup_touch(const Configuration & cfg) {
        // create touch panel driver instance
        auto tpad = std::make_unique<hardware::input::ResistiveTouchPanel>(
                        static_cast<gpio_num_t>(CONFIG_PIN_TOUCH_XL),
                        static_cast<gpio_num_t>(CONFIG_PIN_TOUCH_XR),
                        static_cast<gpio_num_t>(CONFIG_PIN_TOUCH_YD),
                        static_cast<gpio_num_t>(CONFIG_PIN_TOUCH_YU));
        // set parameters
        tpad->set_scale(cfg.touch_scaling_x, cfg.touch_scaling_y);
        tpad->set_offset(cfg.touch_offset_x, cfg.touch_offset_y);
        tpad->set_orientation(true);
        tpad->initialize();
        #if CONFIG_TOUCH_WITH_INT
        tpad->enable_touch_interrupt();
        #endif
        // return driver instance wrapped in LVGL wrapper
        return std::make_unique<hardware::input::TouchpadLVGL>(std::move(tpad));
    }


    std::vector< std::unique_ptr<hardware::AnalogPin> > setup_gpio() {
        // configure analog GPIO pins
        std::vector< std::unique_ptr<hardware::AnalogPin> > v;
        constexpr gpio_num_t pins[]{
            static_cast<gpio_num_t>(CONFIG_PIN_POT_1),
            static_cast<gpio_num_t>(CONFIG_PIN_POT_2),
            static_cast<gpio_num_t>(CONFIG_PIN_BATT_MON)
            };
        for (auto pin: pins) {
            auto pot = std::make_unique<hardware::AnalogPin>();
            pot->set_pin(pin);
            pot->set_attenuation(ADC_ATTEN_6db);
            // set averaging with 10 measurements, and a tolerance of 10,
            // otherwise noise triggers updates
            pot->set_measurement_count(10);
            pot->set_tolerance(10);
            v.emplace_back(std::move(pot));
        }
        return v;
    }


    std::shared_ptr<storage::SPIFlash> setup_flash(const std::string & part_name) {
        // open flash partition and assign it to path /data
        auto spiflash = std::make_shared<storage::SPIFlash>(part_name.c_str(), "/data");
        spiflash->mount();
        return spiflash;
    }


    void setup_uart(std::shared_ptr<comm::DataBroker> db,
                    std::shared_ptr<storage::NVStorage> nvs,
                    std::shared_ptr<storage::SPIFlash> spiflash,
                    UARTData & udata) {
        // load UART handler blocks: pipe, parser with stubs
        namespace cps = comm::parser::serial;
        udata.uart_pipe = std::make_shared<comm::pipe::UARTPipe>(db);
        udata.uart_parser = std::make_shared<comm::parser::SerialParser>(db);
        udata.uart_stubs.emplace_back(std::make_shared<cps::PutFileParserStub>(spiflash));
        udata.uart_stubs.emplace_back(std::make_shared<cps::GetFileParserStub>(spiflash));
        udata.uart_stubs.emplace_back(std::make_shared<cps::DeleteFileParserStub>(spiflash));
        udata.uart_stubs.emplace_back(std::make_shared<cps::MakedirParserStub>(spiflash));
        udata.uart_stubs.emplace_back(std::make_shared<cps::ListDirParserStub>(spiflash));
        udata.uart_stubs.emplace_back(std::make_shared<cps::SetConfigParserStub>(nvs));
        udata.uart_stubs.emplace_back(std::make_shared<cps::GetConfigParserStub>(nvs));
        udata.uart_stubs.emplace_back(std::make_shared<cps::DelConfigParserStub>(nvs));
        udata.uart_stubs.emplace_back(std::make_shared<cps::GetBufSizeParserStub>());
        udata.uart_stubs.emplace_back(std::make_shared<cps::GetFirmwareVersionParserStub>());
        // register loaded stubs with parser
        for (auto & stub: udata.uart_stubs)
            udata.uart_parser->register_parser_stub(stub);
    }


    void setup_websocket(std::shared_ptr<comm::DataBroker> db, const Configuration & cfg, OBSData & odata) {
        // load obs-websocket handler blocks: pipe, parser with stubs
        odata.ws_pipe = std::make_shared<comm::pipe::WebSocketPipe>(db,
            cfg.wifi_ssid, cfg.wifi_password,
            cfg.websocket_host, cfg.websocket_port, cfg.websocket_path);
        odata.obs_parser = std::make_shared<comm::parser::OBSParser>(db);
        odata.obs_reply_parser = std::make_shared<comm::parser::OBSReplyParser>(db);
        odata.ws_stubs.emplace_back(std::make_shared<comm::parser::obs::OBSHello>(cfg.websocket_password));
        odata.ws_stubs.emplace_back(std::make_shared<comm::parser::obs::OBSIdentified>());
        odata.ws_stubs.emplace_back(std::make_shared<comm::parser::obs::OBSEvent>());
        odata.ws_stubs.emplace_back(std::make_shared<comm::parser::obs::OBSRequestResponse>());
        odata.ws_stubs.emplace_back(std::make_shared<comm::parser::obs::OBSRequestBatchResponse>());
        // register loaded stubs with parser
        for (auto & stub: odata.ws_stubs)
            odata.obs_parser->register_parser_stub(stub);
    }

}
