#!python3
'''This script takes care of uploading configuration data to the device.
It can:
    - read a configuration file in JSON format and upload content together with required image files
    - upload icons for battery and WiFi indicators
Run the script without arguments to see detailed usage instructions.
There's an example file, conf_example.json, that you can use to write your own configuration file.
Run it from the root of the project using:
  scripts/write_config.py -i scripts/conf_example.json
You can add --with-icons=black,white option to transfer battery and WiFi indicator icons (black or white versions)
'''
from mcp2221 import find_devices
from device import Device, NVSType
import json
import sys, getopt

err = False # set to True if and error occurs

try:
    opts, args = getopt.getopt(sys.argv[1:],"hp:v:d:i:",["pid=","vid=","devidx=","ifile=","with-icons="])
except getopt.GetoptError:
    err = True

pid = None # device PID (if one is provided)
vid = None # device VID
devidx = None # device index
fname = None # file name
with_icons = False # set to True if --with-icons option is present
icons_color = ""

# namespaces for configuration sections
wifi_str = "wifi"
ws_str = "websocket"
screen_str = "screen"
batt_str = "battery"
bt_str = "buttons"
pot_str = "potentiometers"
# target folder for indicators icons
img_path = "images"

for opt, arg in opts:
    try:
        if opt in ("-p", "--pid"):
            pid = int(arg)
        elif opt in ("-v", "--vid"):
            vid = int(arg)
        elif opt in ("-d", "--devidx"):
            devidx = int(arg)
        elif opt in ("-i", "--ifile"):
            fname = arg
        elif opt == "-h":
            err = True
        elif opt == "--with-icons":
            with_icons = True
            icons_color = arg
    except ValueError:
        err = True

if err or fname is None and not with_icons:
    print("Usage: {scrname} -h -p pid -v vid -d devidx -i fname --with-icons".format(scrname=sys.argv[0]))
    print("At least one of these arguments is required:")
    print("  -i fname, --ifile=fname      read data from file fname")
    print("  --with-icons=black,white     if this option is present, icons for battery/WiFi indicators are uploaded")
    print("Optional arguments:")
    print("  -h                           display help")
    print("  -p pid, --pid=pid            use device with product ID pid")
    print("  -v vid, --vid=vid            use device with vendor ID vid")
    print("  -d devidx, --devidx=devidx   take device with index devidx")
    sys.exit(2)

if pid is not None and vid is not None and devidx is not None:
    dev = Device(find_devices(vid, pid)[devidx])
elif pid is not None and vid is not None and devidx is None:
    dev = Device(find_devices(vid, pid).pop())
elif pid is None and vid is None and devidx is not None:
    dev = Device(find_devices()[devidx])
else:
    dev = Device(find_devices().pop())

def has_keys(data:dict, keys:list) -> bool:
    """Tests if data dict has keys.
    
    Parameters:
        data(dict): data dict
        keys(list): list of string keys
    
    Returns:
        bool: True if data dict has all keys, False otherwise.
    """
    for key in keys:
        if not key in data:
            print("Error: key '{}' missing".format(key))
            return False
    return True

if fname is not None:
    try:
        with open(fname, "r") as f:
            data = json.loads(f.read())
    except IOError:
        print("File {} couldn't be opened.".format(fname))
        sys.exit(1)
    except json.decoder.JSONDecodeError as e:
        print("File {} contains formatting errors:".format(fname))
        print("  {}".format(e))
        sys.exit(1)


    # parse WiFi options
    if wifi_str in data:
        print("Processing WiFi options...")
        if not has_keys(data[wifi_str], ["ssid", "password"]):
            print("WiFi network - mandatory properties:")
            print("  ssid: WiFi network SSID")
            print("  password: WiFi network password")
            sys.exit(1)
        dev.set_conf(wifi_str, "ssid", data[wifi_str]["ssid"], NVSType.Str)
        dev.set_conf(wifi_str, "password", data[wifi_str]["password"], NVSType.Str)


    # parse WebSocket options
    if ws_str in data:
        print("Processing WebSocket options...")
        if not has_keys(data[ws_str], ["host", "port"]):
            print("WebSocket server - mandatory properties:")
            print("  host: WebSocket host address")
            print("  port: WebSocket host port")
            print("Optional properties:")
            print("  path: path on WebSocket host")
            sys.exit(1)
        dev.set_conf(ws_str, "host", data[ws_str]["host"], NVSType.Str)
        dev.set_conf(ws_str, "port", data[ws_str]["port"], NVSType.U16)
        if "path" in data[ws_str]:
            dev.set_conf(ws_str, "path", data[ws_str]["path"], NVSType.Str)
    
    
    # parse screen options
    if screen_str in data:
        print("Processing screen options...")
        if not has_keys(data[screen_str], ["orientation", "backlight_level", "bg_color"]) or not(has_keys(data[screen_str]["bg_color"],["r","g","b"])):
            print("Screen - mandatory properties:")
            print("  orientation: 0 for potentiometers on the right, 1 for left.")
            print("  bl_lvl_act: backlight level when screen is active, with 0 = off and 1023 = maximum.")
            print("  bl_lvl_dimmed: backlight level when screen is dimmed, with 0 = off and 1023 = maximum.")
            print("  bl_dim_delay: screen is dimming delay, in ms (0 = no dimming).")
            print("  bg_color: {r: red, g: green, b: blue}, background color; each component is 8-bit.")
            sys.exit(1)
        dev.set_conf(screen_str, "bl_lvl_act", data[screen_str]["bl_lvl_act"], NVSType.U16)
        dev.set_conf(screen_str, "bl_lvl_dimmed", data[screen_str]["bl_lvl_dimmed"], NVSType.U16)
        dev.set_conf(screen_str, "bl_dim_delay", data[screen_str]["bl_dim_delay"], NVSType.U32)
        if dev.get_conf(screen_str, "orientation") != data[screen_str]["orientation"]:
            # orientation has changed => we need to swap touch screen calibration parameters
            touch_scaling_x = dev.get_conf(screen_str, "touch_scaling_x")
            touch_scaling_y = dev.get_conf(screen_str, "touch_scaling_y")
            touch_offset_x = dev.get_conf(screen_str, "touch_offset_x")
            touch_offset_y = dev.get_conf(screen_str, "touch_offset_y")
            dev.set_conf(screen_str, "touch_scaling_x", touch_scaling_y, NVSType.I16)
            dev.set_conf(screen_str, "touch_scaling_y", touch_scaling_x, NVSType.I16)
            dev.set_conf(screen_str, "touch_offset_x", touch_offset_y, NVSType.I16)
            dev.set_conf(screen_str, "touch_offset_y", touch_offset_x, NVSType.I16)
            dev.set_conf(screen_str, "orientation", data[screen_str]["orientation"], NVSType.U8)
        for cmp in ["r", "g", "b"]:
            dev.set_conf(screen_str, "bg_color_{cmp}".format(cmp=cmp), data[screen_str]["bg_color"][cmp], NVSType.U8)
    
    
    # parse battery options
    if batt_str in data:
        print("Processing battery options...")
        if not has_keys(data[batt_str], ["raw_min", "raw_max"]):
            print("Battery - mandatory properties:")
            print("  raw_min: minimum value read by DAC on battery monitoring pin.")
            print("  raw_max: maximum value.")
            sys.exit(1)
        dev.set_conf(batt_str, "raw_min", data[batt_str]["raw_min"], NVSType.U16)
        dev.set_conf(batt_str, "raw_max", data[batt_str]["raw_max"], NVSType.U16)
    
    
    # parse buttons options
    if bt_str in data:
        print("Processing buttons options...")
        for button in data[bt_str]:
            if not has_keys(button, ["index", "imageInactive", "type", "command_on"]) \
                or (button["type"]==1 and not has_keys(button, ["index", "imageInactive", "imageActive", "command_on", "command_off"])):
                print("Button options - mandatory properties:")
                print("  index: button index (0 to 5)")
                print("  imageInactive: definition of button image for toggled-off/released state")
                print("    filename: name of picture file on computer")
                print("    target: target path on device")
                print("  type: button type (0 = push button, 1 = toggle button)")
                print("  command_on: command issued when button is toggled on/pressed")
                print("Optional properties for push button type (!!!necessary for toggle button type!!!):")
                print("  imageActive: definition of button image for toggle-on state")
                print("    filename: name of picture file on computer")
                print("    target: target path on device")
                print("  command_off: command issued when button is toggled off")
                sys.exit(1)
            
            bt_idx = "{bt}_{idx}".format(bt=bt_str[:-1], idx=button["index"])
            btype = button["type"]
            file_list = (button["imageActive"], button["imageInactive"]) if btype == 1 else [button["imageInactive"]]
            for img in file_list:
                try:
                    print("  --> {}".format(img["filename"]))
                    with open(img["filename"], "rb") as f:
                        if not dev.put_file(f.read(), img["target"]):
                            print("File {fname} couldn't be transferred to {target}.".format(fname=img["filename"], target=img["target"]))
                            sys.exit(1)
                except IOError:
                    print("File {} couldn't be found.".format(img["filename"]))
                    sys.exit(1)
        
            dev.set_conf(bt_idx, "image_off", button["imageInactive"]["target"], NVSType.Str)
            dev.set_conf(bt_idx, "command_on", json.dumps(button["command_on"]), NVSType.Str)
            dev.set_conf(bt_idx, "type", button["type"], NVSType.U8)
            if btype == 1:
                dev.set_conf(bt_idx, "image_on", button["imageActive"]["target"], NVSType.Str)
                dev.set_conf(bt_idx, "command_off", json.dumps(button["command_off"]), NVSType.Str)
            if "event_color" in button:
                if not has_keys(button["event_color"], ["r", "g", "b", "a"]):
                    print("Button event color must be defined as:")
                    print("  {r: red, g: green, b: blue, a: alpha}, with 8-bit components.")
                    print("Correct values for button {} and try again.".format(button["index"]))
                    sys.exit(1)
                for cmp in ["r", "g", "b", "a"]:
                    dev.set_conf(bt_idx, "event_color_{}".format(cmp), button["event_color"][cmp], NVSType.U8)
            

    # parse potentiometers options
    if pot_str in data:
        print("Processing potentiometers options...")
        for pot in data[pot_str]:
            if not has_keys(pot, ["index", "obs_min", "obs_max", "command"]):
                print("Potentiometer settings - mandatory properties:")
                print("  index: potentiometer index (0 to 1)")
                print("  obs_min: minimum value accepted by obs-websocket command")
                print("  obs_max: maximum value accepted by obs-websocket command")
                print("  command: a format string used to generate the obs-websocket command; must contain a format placeholder for a float (e.g. '%0.2f')")
                print("Optional properties:")
                print("  divider: divider for obs_min and obs_max values")
                print("  bg_color: {r: red, g: green, b: blue, a: alpha}, background color; each component is 8-bit.")
                print("  fg_color: {r: red, g: green, b: blue, a: alpha}, foreground color; each component is 8-bit.")
                sys.exit(1)
        
            pot_idx = "{pot}_{idx}".format(pot=pot_str[:-1], idx=pot["index"])
            dev.set_conf(pot_idx, "obs_min", pot["obs_min"], NVSType.I16)
            dev.set_conf(pot_idx, "obs_max", pot["obs_max"], NVSType.I16)
            dev.set_conf(pot_idx, "command", pot["command"], NVSType.Str)
            if "divider" in pot:
                dev.set_conf(pot_idx, "divider", pot["divider"], NVSType.U16)
            for col_str in ["bg_color", "fg_color"]:
                if col_str in pot:
                    if not has_keys(pot[col_str], ["r", "g", "b", "a"]):
                        print("Potentiometer colors must be defined as:")
                        print("  {r: red, g: green, b: blue, a: alpha}, with 8-bit components.")
                        print("Correct values for potentiometer {} and try again.".format(pot["index"]))
                        sys.exit(1)
                    for cmp in ["r", "g", "b", "a"]:
                        dev.set_conf(pot_idx, "{col}_{cmp}".format(col=col_str, cmp=cmp), pot[col_str][cmp], NVSType.U8)
            


if with_icons:
    print("Uploading icons for battery and WiFi indicators...")
    for fname in ["wifi_{color}_{idx:d}".format(color=icons_color,idx=n) for n in range(4)] + ["battery_{color}_{idx:d}".format(color=icons_color,idx=n) for n in range(6)]:
        try:
            path = "{path}/{fname}.png".format(path=img_path, fname=fname)
            print("  --> {}".format(path))
            with open(path, "rb") as f:
                if not dev.put_file(f.read(), path.replace("{}_".format(icons_color),"")):
                    print("File {fname} couldn't be transferred.".format(fname=path))
                    sys.exit(1)
        except IOError:
            print("File {}.png couldn't be found.".format(fname))
            sys.exit(1)

print("Configuration data uploaded succesfully.")
print("Restarting device...")
dev.set_boot_mode()
