# ESP-OBS-Commander: A stream controller with obs-websocket and ESP32

This is a project meant to run on an ESP32 wired with a touch screen and a number of other components. It communicates with the [obs-websocket](https://github.com/obsproject/obs-websocket) plugin for OBS Studio, with protocol v5.0.0.

The project is based on the [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32) and a number of external components (see *dependencies* below). I use [Microsoft Visual Studio Code](https://code.visualstudio.com), for which an [ESP-IDF plugin](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/vscode-setup.html) is available.

The graphical interface is built using [LVGL](https://github.com/lvgl/lvgl) through my own C++ bindings, [lvglpp](https://github.com/vpaeder/lvglpp).

You can find the PCB model that I used for testing [here](https://github.com/vpaeder/kicad_stream_controller)(in French). When I started this, this project costed me, for a single unit (components, PCB and case), about 75€ including shipping. Be aware that you need a little bit of practice with electronics to mount the components correctly. It is of course possible to use another hardware, as long as the screen is big enough. You may need to adapt drivers though.

# Functionalities

- Configurable touch screen interface
- Two analog potentiometers
- Battery or USB-powered
- Wireless control of OBS through WiFi and [obs-websocket](https://github.com/obsproject/obs-websocket)
- Interface configuration through USB

# Pics

I'll add some pics of a real device when I'll have taken meaningful pics of one. In the meantime, here's a 3D model of what it looks like and a snapshot of the interface with default settings. Buttons, icons and colors are customizable (see details below).

<div align="center">
<img src="https://user-images.githubusercontent.com/6388158/117356617-9f9ec800-aebc-11eb-991d-d3a8a837ffc3.png" alt="3D model - front view" width="400"/>
<strong>3D model - front view</strong>
</div>
<div align="center">
<img src="https://user-images.githubusercontent.com/6388158/164441908-b3c408cb-1ae1-4c87-b7b1-7d5f2c05ab33.png" alt="default touch interface" width="400"/>
<strong>default touch interface</strong>
</div>

# Dependencies

- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32)
- [lvglpp](https://github.com/vpaeder/lvglpp) for the graphical interface
- [Pngle](https://github.com/kikuchan/pngle), a lightweight PNG loader
- [OBS Studio](https://obsproject.com) with [obs-websocket](https://github.com/obsproject/obs-websocket)
- for Python scripts, [pymcp2221](https://github.com/vpaeder/pymcp2221), [pySerial](https://github.com/pyserial/pyserial) and [python-websocket-server](https://github.com/Pithikos/python-websocket-server)

# How to compile

Open a command line and move to the project folder. If you didn't do it yet, you must first configure the project. For this, type:

`idf.py menuconfig`

This implies that you have set up the ESP-IDF in such a way that `idf.py`is in the execution path. In case you didn't, just add the appropriate path in front of it (for me on MacOS, this means `~/esp/esp-idf/tools/idf.py`).

This configuration step is for a number of low-level things such as pin numbers, cache size, etc. You can also set WiFi and WebSocket default values here, although you can also set them later through USB (see details below). The graphical interface is not configured there (see also below for this).

Once you finished configuration and saved, you can run:

`idf.py build`

# How to install

Again, open a command line and move to the project folder. Set the ESP32 in flash mode (see below or *scripts* folder), and type:

`idf.py -p <path_to_serial_port> flash -b 2000000`

Check on your system how the serial port is defined. For me, it shows up as `/dev/cu.usbmodem14201`. On Windows, it should be something like `COM1`. The PCB I made for the purpose uses a USB to serial converter from Microchip, the [MCP2221A](https://www.microchip.com/en-us/product/MCP2221A). I wrote a [Python driver](https://github.com/vpaeder/pymcp2221) for it, which I use to set the ESP32 in flash or boot mode. There are scripts in the *scripts* folder to do that.

On first lauch, you'll be guided through the screen and potentiometers calibration processes. If, for any reason, you want to re-calibrate your device at a latter time, you can use the script `clear_calibration.py` available in the `scripts` folder.

# How to configure the interface

You can find some help in the *scripts* folder to automate the configuration step (see script *write_config.py* and configuration file *conf_example.json*). However, it is also possible to configure the device manually. I defined a serial protocol for this purpose. Here is a list of the commands.
| Command                             | Function                         | Response                     |
|-------------------------------------|----------------------------------|------------------------------|
| AT+PUTFILE=fpath,fsize | Initiates the transfer of a file that should be saved as *fpath* and is of size *fsize*. Next command must be *AT+PUDATA* to transfer file content, or *AT+ABORT* to abort transfer. | *OK* if the file could be opened, *ERROR* otherwise. |
| AT+PUTDATA=data | Transfers a data packet. It must be encoded in base64. | *OK* if the packet could be saved, *ERROR* otherwise. Returns *UNKN* if *AT+PUTFILE* wasn't called first. |
| AT+GETFILE=fpath | Requests to open the file present at *fpath* on the device. Next commands must be *AT+GETDATA* until all data was transferred, or *AT+ABORT* to abort transfer. | *SIZE=fsize* if the file could be opened, with *fsize* being the file size in bytes. Replies *ERROR* if the file couldn't be opened. |
| AT+GETDATA=dsize | Requests *dsize* bytes from device. | *DATA=data* where *data* is the data in base64 format, or *ERROR* if the transfer has failed. Replies *UNKN* if *AT+GETFILE* wasn't called first. |
| AT+DELETE=path | Asks the device to delete file or directory at *path*. | *OK* if the path could be deleted, *ERROR* otherwise. |
| AT+LISTDIR=dpath | Requests a list of the files in directory at *dpath*. Next commands must be *AT+NEXTFILE* until all files were listed, or *AT+ABORT* to stop. | *NUMFILES=count* if the directory exists, with *count* the number of files in it. If the path couldn't be found, replies *ERROR*. |
| AT+NEXTFILE | Asks for the next file in the directory opened with the *AT+LISTDIR* command. | *FILE=name,type* if the command succeeds, with *name* the name of the file/directory and *type*=1 for files or 2 for directories. Returns *ERROR* if command failed, and *UNKN* if *AT+LISTDIR* hasn't been called first. |
| AT+MAKEDIR=dpath | Creates a directory at *dpath*. | *OK* if the directory could be created, *ERROR* otherwise. |
| AT+ABORT | Aborts an ongoing command. | *OK* if the device could return to default state, *ERROR* otherwise. |
| AT+SETCONF=namespace,key,type,value | Sets a value in the non-volatile storage, in the namespace *namespace*, at key *key*, of type *type*. The value of *type* can be: <ul><li>01: 8-bit unsigned integer</li><li>17: 8-bit signed integer</li><li>02: 16-bit unsigned integer</li><li>18: 16-bit signed integer</li><li>04: 32-bit unsigned integer</li><li>20: 32-bit signed integer</li><li>08: 64-bit unsigned integer</li><li>24: 64-bit signed integer</li><li>33: string</li><li>62: binary blob</li></ul> | *OK* if the key could be set, *ERROR* otherwise |
| AT+GETCONF=namespace,key | Gets the value of the key *key* in namespace *namespace* from non-volatile storage. | *VALUE=type,value* if the key could be read, with *type* the code for the key type (see above) and *value* its value. Returns *ERROR* if the key couldn't be read. |
| AT+DELCONF=namespace,key | Deletes key *key* in namespace *namespace* from non-volatile storage. | *OK* if key could be deleted, *ERROR* otherwise. |
| AT+GETBUFS | Requests the size of the serial buffer. | *BUFS=value*, where *value* is the size of the serial buffer in bytes. |
| AT+GETFWVER | Requests firmware version. | *FWVER=value*, where *value* is the firmware version |

It is possible to configure the interface manually with a serial tool, such as screen (command line tool for MacOS/Linux) or Putty (for Windows). To transfer files, you must be able to encode data in base64. Otherwise, configuration keys are not encoded in anyway way and are easy to set. The relevant keys are:
| Namespace | Key              | Value type                  | Description                              |
|-----------|------------------|-----------------------------|------------------------------------------|
| wifi | ssid | 33 (string) | WiFi network SSID |
| wifi | password | 33 (string) | WiFi network password |
| websocket | host | 33 (string) | obs-websocket host address |
| websocket | port | 02 (uint16_t) | obs-websocket host port |
| websocket | path | 33 (string) | path on WebSocket server |
| screen | orientation | 01 (uint8_t) | screen orientation (0=potentiometers on the right, 1=on the left) |
| screen | bl_lvl_act | 02 (uint16_t) | backlight intensity when screen is active (0=off, 1023=maximum) |
| screen | bl_lvl_dimmed | 02 (uint16_t) | backlight intensity when screen is dimmed (0=off, 1023=maximum) |
| screen | bl_dim_delay | 04 (uint32_t) | backlight dimming delay, in ms (0=no dimming) |
| screen | bg_color_r | 01 (uint8_t) | red component of background color |
| screen | bg_color_g | 01 (uint8_t) | green component of background color |
| screen | bg_color_b | 01 (uint8_t) | blue component of background color |
| battery | raw_min | 02 (uint16_t) | minimum value read on battery monitor pin  |
| battery | raw_max | 02 (uint16_t) | maximum value read on battery monitor pin  |
| button_n, where n is the button index (0 to 5) | image_off | 33 (string) | path to image file for toggled-off/released state |
| button_n | image_on | 33 (string) | path to image file for toggled-on/pressed state |
| button_n | event_color_r | 01 (uint8_t) | red component of event highlighting color |
| button_n | event_color_g | 01 (uint8_t) | green component of event highlighting color |
| button_n | event_color_b | 01 (uint8_t) | blue component of event highlighting color |
| button_n | event_color_a | 01 (uint8_t) | alpha component of event highlighting color |
| button_n | type | 01 (uint8_t) | 0 for push button, 1 for toggle button |
| button_n | command_on |  | 33 (string) | command issued when toggled on/pressed |
| button_n | command_off | 33 (string) | command issued when toggled off |
| potentiometer_n, where n is the potentiometer index (0 to 1) | bg_color_r | 01 (uint8_t) | red component of background color |
| potentiometer_n | bg_color_g | 01 (uint8_t) | green component of bar background color |
| potentiometer_n | bg_color_b | 01 (uint8_t) | blue component of bar background color |
| potentiometer_n | bg_color_a | 01 (uint8_t) | alpha component of bar background color |
| potentiometer_n | fg_color_r | 01 (uint8_t) | red component of bar foreground color |
| potentiometer_n | fg_color_g | 01 (uint8_t) | green component of bar foreground color |
| potentiometer_n | fg_color_b | 01 (uint8_t) | blue component of bar foreground color |
| potentiometer_n | fg_color_a | 01 (uint8_t) | alpha component of bar foreground color |
| potentiometer_n | raw_min | 02 (uint16_t) | minimum raw value |
| potentiometer_n | raw_max | 02 (uint16_t) | maximum raw value |
| potentiometer_n | obs_min | 18 (int16_t) | minimum value accepted by obs-websocket command |
| potentiometer_n | obs_max | 18 (int16_t) | maximum value accepted by obs-websocket command |
| potentiometer_n | divider | 02 (uint16_t) | divider for obs_min and obs_max values |
| potentiometer_n | command | 33 (string) | a format string used to generate the obs-websocket command; must contain a format placeholder for a float (e.g. `%0.2f`) |

Remember to upload the files that you reference in the *image_off_n* and *image_on_n* keys. These should be 100x100 pixels and in PNG format. If you use an image of another size, it'll be rescaled. Also, the battery level and WiFi network indicators require the following files in the *images* folder:
- *battery_n.png* with n from 0 to 5 (0 to 4 = 0%, 25%, 50%, 75%, 100% and 5 = charging)
- *wifi_n.png* with n from 0 to 4 (0 = no connection, 1 = poor, 2 = ok, 3 = excellent)
These icons are available in the *images* folder, together with a number of basic button icons composed using [FontAwesome](https://fontawesome.com) glyphs in [Inkscape](https://inkscape.org/). You can of course make you own icons if you prefer.

## Configuring command strings for buttons

Commands accepted by obs-websocket are defined in [protocol.md](https://github.com/obsproject/obs-websocket/blob/master/docs/generated/protocol.md). All setters can be used, but at the moment I didn't program anything to handle a potential response from the server. Therefore, getters cannot be used for now. The command strings must be parsable to JSON. The parsed JSON must be compatible with obs-websocket. The structure must be:
```json
    {
        "op": either 6 (Request) or 8 (RequestBatch),
        "d": {
            "requestType": "see protocol.md for available commands",
            "requestData": {
                ... if request type requires data, fill with appropriate fields
            }
        }
    }
```

For example, let us take the command to change scene defined [here](https://github.com/obsproject/obs-websocket/blob/master/docs/generated/protocol.md#setcurrentprogramscene). We would write:
```json
    {
        "op": 6,
        "d": {
            "requestType": "SetCurrentProgramScene",
            "requestData": {
                "sceneName": "The scene name"
            }
        }
    }
```

It is possible to fill the JSON configuration file with JSON commands, which will be stringified before upload, or stringify them yourself.

### Configuring command strings for potentiometers

For potentiometers, we must transmit the value in some way. I used a rather dirty but simple method for that. The command string must contain a placeholder for the *printf* command, for a float number. It is typically something like `%f`, or with formatting directives `%0.2f`. This causes a little issue. If you want to define your command in a JSON configuration file, you must stringify the command. This is because `%f` is obviously not a number, and this makes the JSON parser fail. For example, if we want to define a command to [change input volume](https://github.com/obsproject/obs-websocket/blob/master/docs/generated/protocol.md#setinputvolume), we would write:
```json
    """{
        \"op\": 6,
        \"d\": {
            \"requestType\": \"SetInputVolume\",
            \"requestData\": {
                \"inputName\":\"Main input\",
                \"inputVolumeMul\":%0.2f
            }
        }
    }"""
```
Note the literal double quotes \\\". This is how stringified JSON looks like.

As for the actual value, it is likely that what is read from the potentiometer is not within the bounds of what obs-websocket accepts. If you followed the calibration process on first start, the only thing you need to know is what range obs-websocket expects. For example, the [SetInputVolume](https://github.com/obsproject/obs-websocket/blob/master/docs/generated/protocol.md#setinputvolume) command accepts either values from 0 to 20 (if *inputVolumeMul* is set) or from -100 to 26 (if *inputVolumeDb* is set). You can therefore set *obs_min=0* and *obs_max=20*, or *obs_min=-100* and *obs_max=26* in the configuration file. If you need to work with floats, you can set *divider > 1*. This will be used to divide the computed value.

## A note on flash partitions

The flash image is configured for a 16MB ESP32 module. You can edit `partitions.csv` to adjust the size of the data partition if you use a module with less flash. The current configuration defines a 128kB non-volatile storage called *config* (for configuration keys), a 2MB app partition, and leaves 13.8MB for data (called *data*).

## Additional design notes

I collected here some elements of explanation regarding miscellaneous design choices.

- As I chose to use LVGL only at a late stage of the project, I had already written file and image management routines, which could mostly be handled by LVGL as well.
- I've written a few binding classes that connect LVGL to my own classes; namely, `gui/image/image_lvgl.h` binds the `Image` classes, `hardware/input/touch_lvgl.h,cpp` the `TouchPanel` classes, `hardware/screen/screen_lvgl.h,cpp` the `Screen` classes and `storage/partition_lvgl.h` the `Partition` classes (the latter being unused for now).
- I implemented a simple pub/sub system to handle network and serial events. There's a `DataBroker` class that takes subscriptions from `DataNode`s (as callbacks) and dispatches messages. I originally used `weak_ptr` to store callbacks, but the `expired()` method doesn't seem to work on ESP-IDF. I've split data nodes into two categories: pipes (that's the ones that handle data transfers) and parsers (the ones parsing data).
- To keep the pub/sub system simple and general enough, the message type I use is text strings (which makes that JSON structures need to be stringified to exchange them in between data nodes). Data nodes are built in a modular manner, with plug-in data parsers that treat one command each.
- Backlight dimming relies on the LED control ESP-IDF module. You will likely need to connect the pin to the gate of a transistor, as the current delivered by ESP32 pins won't suffice to power a screen backlight (except for a stamp-sized one maybe).

# Scripts

The Python and bash scripts located in the *scripts* folder are written to facilitate flashing and configuration. Here is a short description of the scripts (you'll find more details inside each file):
- set_flash.py: this sets the ESP32 into flashing mode
- set_boot.py: this sets the ESP32 into booting mode
- do_flash.sh: this sets flashing mode and flashes the ESP32, then reboots
- device.py: this file defines a class to communicate with the device with serial commands; it is meant to be used by other scripts for this purpose
- write_config.py: this script reads a configuration file in JSON format (see *conf_example.json* for an example), transfers the listed files and writes the configuration keys
- conf_example.json: this is an example of configuration file; use it with *write_config.py*.
- test_ws_server.py: a script that sets up a test WebSocket server
- clear_calibration.py: this clears calibration flags

# Tests

I have to find the time to install the unit testing host on a Linux VM, as testing on a real device is rather unpractical. For now I can tell it has been monkey-tested by pseudo-random personal actions, and it monkey-works alright.
