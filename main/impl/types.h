/** \file types.h
 *  \brief Header file for data types.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "comm/data_broker.h"
#include "comm/pipe/uart_pipe.h"
#include "comm/parser/serial_parser.h"
#include "comm/parser/serial_parser_stub.h"
#include "comm/pipe/websocket_pipe.h"
#include "comm/parser/obs_parser.h"
#include "comm/parser/obs_reply_parser.h"
#include "comm/parser/obs_parser_stub.h"

#include "storage/nvs.h"
#include "storage/spi_flash.h"

#include "lvglpp/widgets/bar/bar.h"
#include "lvglpp/misc/style.h"

#include "gui/image/image_lvgl.h"
#include "gui/widgets/imgbtn.h"
#include "gui/widgets/tgimgbtn.h"
#include "gui/widgets/image.h"

#include <string>
#include <vector>
#include <memory>


namespace eobsws::impl {
    /** \class PotentiometerConfiguration
     *  \brief Container class for potentiometer configuration.
     */
    struct PotentiometerConfiguration {
        /** \property uint16_t raw_min
         *  \brief Raw minimum value.
         */
        uint16_t raw_min;

        /** \property uint16_t raw_max
         *  \brief Raw maximum value.
         */
        uint16_t raw_max;

        /** \property int16_t obs_min
         *  \brief Minimum value accepted by obs-websocket command
         */
        int16_t obs_min;

        /** \property int16_t obs_max
         *  \brief Maximum value accepted by obs-websocket command
         */
        int16_t obs_max;

        /** \property uint16_t divider
         *  \brief Divider for obs_min and obs_max values
         */
        uint16_t divider;

        /** \property std::string command
         *  \brief Command string used to format data. It must contain
         *  a format tag for a float number (e.g. %f).
         */
        std::string command = "%0.2f";

        /** \property lv_color_t bg_color
         *  \brief Color used for bar background.
         */
        lv_color_t bg_color;

        /** \property lv_color_t fg_color
         *  \brief Color used for bar foreground.
         */
        lv_color_t fg_color;

        /** \property lv_opa_t bg_opacity
         *  \brief Opacity of bar background.
         */
        lv_opa_t bg_opacity;

        /** \property lv_opa_t fg_opacity
         *  \brief Opacity of bar foreground.
         */
        lv_opa_t fg_opacity;
    };


    /** \class Configuration
     *  \brief Container class for general configuration.
     */
    struct Configuration {
        /** \property std::string storage_part_name
         *  \brief Flash storage partition name.
         */
        std::string storage_part_name;

        /** \property std::string wifi_ssid
         *  \brief WiFi network SSID.
         */
        std::string wifi_ssid;

        /** \property std::string wifi_password
         *  \brief WiFi network password.
         */
        std::string wifi_password;

        /** \property std::string websocket_host
         *  \brief WebSocket host address.
         */
        std::string websocket_host;

        /** \property uint16_t websocket_port
         *  \brief WebSocket host port.
         */
        uint16_t websocket_port;

        /** \property std::string websocket_path
         *  \brief Path on WebSocket host.
         */
        std::string websocket_path;

        /** \property std::string websocket_password
         *  \brief Password for obs-websocket server.
         */
        std::string websocket_password;

        /** \property lv_disp_rot_t screen_orientation
         *  \brief Screen orientation: LV_DISP_ROT_NONE or LV_DISP_ROT_180
         */
        lv_disp_rot_t screen_orientation = LV_DISP_ROT_NONE;

        /** \property uint16_t bl_lvl_act
         *  \brief Screen backlight level when active (0=minimum, 1024=maximum)
         */
        uint16_t bl_lvl_act = 1024;

        /** \property uint16_t bl_lvl_dimmed
         *  \brief Screen backlight level when dimmed (0=minimum, 1024=maximum)
         */
        uint16_t bl_lvl_dimmed = 100;

        /** \property uint32_t bl_dim_delay
         *  \brief Screen backlight dimming delay, in ms (0 = no dimming)
         */
        uint32_t bl_dim_delay = 10000;

        /** \property bool touch_calibrated
         *  \brief True if touch panel calibration has been done
         */
        bool touch_calibrated = false;

        /** \property int16_t touch_scaling_x
         *  \brief Touch panel horizontal scaling
         */
        int16_t touch_scaling_x;

        /** \property int16_t touch_scaling_y
         *  \brief Touch panel vertical scaling
         */
        int16_t touch_scaling_y;

        /** \property int16_t touch_offset_x
         *  \brief Touch panel horizontal offset
         */
        int16_t touch_offset_x = 0;

        /** \property int16_t touch_offset_y
         *  \brief Touch panel vertical offset
         */
        int16_t touch_offset_y = 0;

        /** \property uint16_t battery_min
         *  \brief Minimum raw battery value
         */
        uint16_t battery_min = 1000;

        /** \property uint16_t battery_max
         *  \brief Maximum raw battery value
         */
        uint16_t battery_max = 1100;

        /** \property std::vector<PotentiometerConfiguration> pots
         *  \brief Storage for configuration data for potentiometers.
         */
        std::vector<PotentiometerConfiguration> pots;

        /** \property bool pots_calibrated
         *  \brief True if potentiometers calibration has been done
         */
        bool pots_calibrated = false;

        /** \fn Configuration(std::shared_ptr<storage::NVStorage> nvs)
         *  \brief Constructor.
         *  \param nvs: pointer to a non-volatile storage accessor.
         */
        Configuration(std::shared_ptr<storage::NVStorage> nvs);
    };


    /** \class UARTData
     *  \brief Container class for UART handler.
     */
    struct UARTData {
        /** \property std::shared_ptr<comm::pipe::UARTPipe> uart_pipe
         *  \brief Pointer to an UART communication pipe instance.
         */
        std::shared_ptr<comm::pipe::UARTPipe> uart_pipe;

        /** \property std::shared_ptr<comm::parser::SerialParser> uart_parser
         *  \brief Pointer to a serial command parser instance.
         */
        std::shared_ptr<comm::parser::SerialParser> uart_parser;

        /** \property std::vector< std::shared_ptr<comm::parser::ParserStub> > uart_stubs
         *  \brief Container for serial command parser stub instances.
         */
        std::vector< std::shared_ptr<comm::parser::ParserStub> > uart_stubs;
    };


    /** \class OBSData
     *  \brief Container class for OBS handler.
     */
    struct OBSData {
        /** \property std::shared_ptr<comm::pipe::WebSocketPipe> ws_pipe
         *  \brief Pointer to a WebSocket communication pipe instance.
         */
        std::shared_ptr<comm::pipe::WebSocketPipe> ws_pipe;

        /** \property std::shared_ptr<comm::parser::OBSParser> obs_parser
         *  \brief Pointer to an obs-websocket command parser instance.
         */
        std::shared_ptr<comm::parser::OBSParser> obs_parser;

        /** \property std::shared_ptr<comm::parser::OBSReplyParser> obs_reply_parser
         *  \brief Pointer to an obs-websocket reply parser instance.
         */
        std::shared_ptr<comm::parser::OBSReplyParser> obs_reply_parser;

        /** \property std::vector< std::shared_ptr<comm::parser::ParserStub> > ws_stubs
         *  \brief Container for obs-websocket command parser stub instances.
         */
        std::vector< std::shared_ptr<comm::parser::ParserStub> > ws_stubs;
    };

    /** \class GUIData
     *  \brief Container class for GUI data.
     */
    struct GUIData {
        /** \property std::unique_ptr<lvgl::core::Object> root
         *  \brief Root LVGL object.
         */
        std::unique_ptr<lvgl::core::Object> root;

        /** \property std::vector< std::shared_ptr<gui::widgets::ImageButton> > buttons
         *  \brief This is where buttons instances are stored. We need shared_ptr here
         *  because we need to reinterpret_cast between button types.
         */
        std::vector< std::shared_ptr<gui::widgets::ImageButtonPNG>> buttons;

        /** \property std::vector< std::unique_ptr<lvgl::widgets::Bar> > bars
         *  \brief This stores the bars instances.
         */
        std::vector< std::unique_ptr<lvgl::widgets::Bar> > bars;

        /** \property std::vector< std::unique_ptr<lvgl::core::Object> > lvgl_objects
         *  \brief This contains LVGL objects that have to be stored for the program to
         *  work properly, but are not accessed directly.
         */
        std::vector< std::unique_ptr<lvgl::core::Object> > lvgl_objects;

        /** \property std::vector< std::shared_ptr<lvgl::misc::Style> > lvgl_styles
         *  \brief A list of styles used in the interface.
         */
        std::vector< std::shared_ptr<lvgl::misc::Style> > lvgl_styles;

        /** \property std::vector< std::unique_ptr<lvgl::misc::StyleTransition> > lvgl_transitions
         *  \brief A list of style transitions used by styles.
         */
        std::vector< std::shared_ptr<lvgl::misc::StyleTransition> > lvgl_transitions;

        /** \property std::vector< std::shared_ptr<gui::image::LvImagePNG> > wifi_imgs
         *  \brief Images for WiFi level indicator.
         */
        std::vector<std::shared_ptr<gui::image::LvImagePNG>> wifi_imgs;

        /** \property std::unique_ptr<gui::widgets::Image> wifi_icon
         *  \brief WiFi level indicator icon.
         */
        std::unique_ptr<gui::widgets::ImagePNG> wifi_icon;

        /** \property std::vector< std::shared_ptr<gui::image::LvImagePNG> > battery_imgs
         *  \brief Images for battery level indicator.
         */
        std::vector<std::shared_ptr<gui::image::LvImagePNG>> battery_imgs;

        /** \property std::unique_ptr<gui::widgets::Image> battery_icon
         *  \brief Battery level indicator icon.
         */
        std::unique_ptr<gui::widgets::ImagePNG> battery_icon;

        /** \fn GUIData()
         *  \brief Constructor.
         */
        GUIData() {
            this->root = std::make_unique<lvgl::core::Object>(lv_scr_act());
        }
    };

    /** \enum ButtonType
     *  \brief Button types.
     */
    enum class ButtonType : uint8_t {
        PushButton = 0, /**< push button */
        ToggleButton = 1 /**< toggle button */
    };


    /** \class ButtonConfiguration
     *  \brief Container class for button configuration.
     */
    struct ButtonConfiguration {
        /** \property std::string image_off
         *  \brief File path for off-state (released) image.
         */
        std::string image_off;

        /** \property std::string image_on
         *  \brief File path for on-state (pressed) image.
         */
        std::string image_on;

        /** \property ButtonType type
         *  \brief Button type.
         */
        ButtonType type;

        /** \property std::string command_on
         *  \brief Command issued when button is pressed or toggled on.
         */
        std::string command_on;

        /** \property std::string command_off
         *  \brief Command issued when button is toggled off.
         */
        std::string command_off;

        /** \property lv_color_t event_color
         *  \brief Color used to highlight click events.
         */
        lv_color_t event_color;

        /** \property lv_opa_t event_opacity
         *  \brief Opacity of highlighting for click events.
         */
        lv_opa_t event_opacity;

        /** \fn ButtonConfiguration(std::shared_ptr<storage::NVStorage> nvs, uint8_t idx)
         *  \brief Constructor.
         *  \param nvs: pointer to a non-volatile storage accessor.
         *  \param idx: button index in storage.
         */
        ButtonConfiguration(std::shared_ptr<storage::NVStorage> nvs, uint8_t idx);
    };

}