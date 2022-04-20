'''This script sets the ESP32 in boot mode. By default, it assumes that there's only one device
with MCP2221A connected, and it has the default VID=1240, PID=221. In case your configuration
differs, you can use the script this way:

    - if you have more than one device with default VID=1240, PID=221, select device with:
        set_boot.py device_index
      with device_index = the index of the device in the list of devices (0-based)
    - if you have one device with non-default VID / PID:
        set_boot.py vid_value pid_value
    - if you have multiple devices with non-default VID / PID:
        set_boot.py vid_value pid_value device_index
'''
from time import sleep
import sys
from device import Device
from mcp2221 import find_devices

if len(sys.argv)==4:
    dev = Device(find_devices(int(sys.argv[1]), int(sys.argv[2]))[int(sys.argv[3])])
elif len(sys.argv)==3:
    dev = Device(find_devices(int(sys.argv[1]), int(sys.argv[2])).pop())
elif len(sys.argv)==2:
    dev = Device(find_devices()[int(sys.argv[1])])
else:
    dev = Device(find_devices().pop())

dev.set_boot_mode()
