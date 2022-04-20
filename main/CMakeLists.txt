idf_component_register(SRCS
    "storage/spi_flash.cpp"
    "storage/file.cpp"
    "storage/dir.cpp"
    "storage/nvs.cpp"

    "hardware/screen/st7789vi.cpp"
    "hardware/screen/screen_lvgl.cpp"
    "hardware/input/res_touch.cpp"
    "hardware/input/touch_lvgl.cpp"
    "hardware/analog_pin.cpp"
    "hardware/digital_pin.cpp"

    "comm/pipe/uart_pipe.cpp"
    "comm/pipe/wifi_pipe.cpp"
    "comm/pipe/websocket_pipe.cpp"
    "comm/parser/serial_parser.cpp"
    "comm/parser/serial_parser_stub.cpp"
    "comm/parser/obs_parser.cpp"
    "comm/parser/obs_parser_stub.cpp"
    "comm/parser/obs_reply_parser.cpp"
    
    "gui/image/image_png.cpp"

    "impl/setup.cpp"
    "impl/gui.cpp"
    "impl/calibrate.cpp"
    "impl/types.cpp"

    "esp_obs_cmd.cpp"

     INCLUDE_DIRS "."
     )

