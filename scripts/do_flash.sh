#!/bin/zsh
# This script sets the ESP32 in flash mode, flashes the firmware and reboots.
# Change IDF_PY_PATH to something else if your ESP-IDF install is elsewhere.
# If idf.py is in the command path, set it empty.
# The script must be run from the root of the project directory, for instance this way:
#
#   sh scripts/do_flash.sh
#
IDF_PY_PATH=~/esp/esp-idf/tools/
python scripts/set_flash.py
if [ ! -z "$1" ]; then
  DEV_PORT=$1
else
  DEV_PORT=/dev/cu.usbmodem14201
fi

${IDF_PY_PATH}idf.py -p $DEV_PORT flash -b 2000000
python scripts/set_boot.py
