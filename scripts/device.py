"""Python class to handle serial communication with the ESP32 controller firmware.
"""
import warnings
from time import sleep, time
from mcp2221 import MCP2221, find_devices
from mcp2221.enums import GPIODirection, GPIO1Function, GPIO3Function, MemoryType
from serial.tools.list_ports import comports
from serial import Serial
import base64
import enum

__all__ = ["NVSType", "Device"]

class NVSType(enum.IntEnum):
    """Non-volatile storage types."""
    U8 = 0x01 # 8-bit unsigned integer
    U16 = 0x02 # 16-bit unsigned integer
    U32 = 0x04 # 32-bit unsigned integer
    U64 = 0x08 # 64-bit unsigned integer
    I8 = 0x11 # 8-bit signed integer
    I16 = 0x12 # 16-bit signed integer
    I32 = 0x14 # 32-bit signed integer
    I64 = 0x18 # 64-bit signed integer
    Str = 0x21 # String
    Blob = 0x42 # Binary blob


class Device():
    """Class to handle serial communication with the ESP32 controller firmware.
    
    Attributes:
        _termchar(byte): termination character for serial communication
        _buf_size(int): serial buffer size, in bytes
    """

    _termchar = b'\r'
    _buf_size = 1024

    def __init__(self, desc:dict=None):
        """Constructor.
        
        Parameters:
            desc(dict): device descriptor (default:None). If None, first device found
                        will be opened.
        """
        if desc == None:
            try:
                desc = find_devices().pop()
            except IndexError:
                warnings.warn("No device found", Warning)
                return
        self.dev = MCP2221(desc)
        ser_port = self._find_serial_port(desc)
        if ser_port is not None:
            self._ser = Serial(ser_port, 115200)
            self._ser.timeout = 0.5
        else:
            self._ser = None

    def set_gpio_powerup_values(self) -> None:
        """Sets GPIO power-up values. This is the state of the GPIO pins
        of the USB to serial converter chip. They define in which state the
        ESP32 starts (flash or boot mode). By default, we want to start
        in boot mode.
        """
        # in order to work properly, we need pins 1 and 3 as GPIO;
        # we set them so as boot mode is default
        self.dev.set_default_memory_target(MemoryType.Flash)
        self.dev.gpio1_function = GPIO1Function.GPIO
        self.dev.gpio1_powerup_direction = GPIODirection.Output
        self.dev.gpio1_powerup_value = True
        self.dev.gpio3_function = GPIO3Function.GPIO
        self.dev.gpio3_powerup_direction = GPIODirection.Input
    
    def disable_esp32(self) -> None:
        """Turns ESP32 off.
        """
        self.dev.gpio3_function = GPIO3Function.GPIO
        self.dev.gpio3_direction = GPIODirection.Output
        self.dev.gpio3_value = False
    
    def enable_esp32(self) -> None:
        """Turns ESP32 on.
        """
        self.dev.gpio3_function = GPIO3Function.GPIO
        self.dev.gpio3_value = True
        self.dev.gpio3_direction = GPIODirection.Input

    def set_flash_mode(self) -> None:
        """Sets ESP32 in flash mode.
        """
        self.dev.set_default_memory_target(MemoryType.SRAM)
        # turn off ESP32
        self.disable_esp32()
        sleep(0.2)
        # set flash mode
        self.dev.gpio1_function = GPIO1Function.GPIO
        self.dev.gpio1_direction = GPIODirection.Output
        self.dev.gpio1_value = False
        sleep(0.2)
        # turn on ESP32
        self.enable_esp32()
    
    def set_boot_mode(self) -> None:
        """Sets ESP32 in boot mode.
        """
        self.dev.set_default_memory_target(MemoryType.SRAM)
        # turn off ESP32
        self.disable_esp32()
        sleep(0.2)
        # set boot mode
        self.dev.gpio1_function = GPIO1Function.GPIO
        self.dev.gpio1_direction = GPIODirection.Output
        self.dev.gpio1_value = True
        sleep(0.2)
        # turn on ESP32
        self.enable_esp32()

    def _find_serial_port(self, desc:dict) -> str:
        """Finds the serial port corresponding to given device descriptor.
        
        Parameters:
            desc(dict): device descriptor.
        
        Returns:
            str: path to serial port.
        """
        ports = [port for port in comports() if port.vid==desc["vendor_id"]
                 and port.pid==desc["product_id"]
                 and int(port.location.split("-")[-1])==desc["interface_number"]]
        if len(ports)>1:
            warnings.warn("More than one serial port found", Warning)
            return
        if len(ports)==0:
            warnings.warn("No serial port found", Warning)
            return
        return ports[0].device
    
    def _serial_write(self, data:bytes) -> int:
        """Writes data to serial port.
        
        Parameters:
            data(bytes): data to write.
        
        Returns:
            int: number of bytes written.
        """
        if self._ser is not None:
            written = self._ser.write(data + self._termchar)
            self._ser.flushOutput()
            return written
        return 0
    
    def _serial_read(self, nbytes:int=1024) -> bytes:
        """Reads data from serial port until termination character is encountered.
        
        Parameters:
            nbytes(int): maximum number of bytes to read (default: 1024)
        
        Returns:
            bytes: read bytes.
        """
        ret = b""
        if self._ser is not None:
            while True:
                ret = self._ser.read_until(self._termchar, nbytes)
                if len(ret)>0 and self._ser.in_waiting==0: break
        return ret

    def _serial_ask(self, cmd:bytes, nbytes:int=1024) -> bytes:
        """Sends a serial command and reads response.
        
        Parameters:
            cmd(bytes): command to write.
            nbytes(int): maximum number of bytes to read (default: 1024)
        
        Returns:
            bytes: response.
        """
        if self._ser is not None:
            self._serial_write(cmd)
            return self._serial_read(nbytes)
        return b""

    def _test_reply(self, resp:bytes) -> bool:
        """Tests if response is valid.
        
        Parameters:
            resp(bytes): response to test.
        
        Returns:
            bool: False if the response string means "error", True otherwise.
        """
        if resp is None: return False
        if len(resp) == 0: return False
        if resp.find(b"ERROR") == 0: return False
        if resp.find(b"UNKN") == 0: return False
        return True

    def _get_buffer_size(self) -> int:
        """Requests serial buffer size.
        
        Returns:
            int: buffer size, in bytes.
        """
        buf = self._serial_ask(b"AT+GETBUFS")
        if not self._test_reply(buf): return 0
        if buf.find(b"BUFS=") != 0:
            return 0
        return int(buf.split(b"=",1)[-1].strip())

    def list_dir(self, dir_path: str) -> 'list[list]':
        """Lists the content of a directory.
        
        Parameters:
            dir_path(str): path to list content.
        
        Returns:
            list[list]: a list of file and directory names.
        """
        cmd = "AT+LISTDIR={}".format(dir_path).encode("utf-8")
        nf_str = self._serial_ask(cmd)
        if nf_str.find(b"NUMFILES=")!=0:
            return []
        numfiles = int(nf_str.split(b"=",1)[-1].strip())
        files = []
        while numfiles:
            file = self._serial_ask(b"AT+NEXTFILE")
            if file.find(b"FILE=") == 0:
                spl = file.split(b"=",1)[-1].split(b",",1)
                files.append([spl[0], int(spl[1].strip())])
            numfiles-=1
        return files
    
    def get_file(self, file_path:str) -> bytes:
        """Gets a file from device.
        
        Parameters:
            file_path(str): path to file.
        
        Returns:
            bytes: file content.
        """
        # tries to open file; if ok, returns SIZE=n
        cmd = "AT+GETFILE={}".format(file_path).encode("utf-8")
        nb_str = self._serial_ask(cmd)
        if nb_str.find(b"SIZE=")!=0:
            return b""
        # converts file size
        nbytes = int(nb_str.split(b"=",1)[-1].strip())
        # transfers data; max. chunk size = uart buffer size - 5
        data = b""
        while len(data)<nbytes:
            rsize = min(nbytes-len(data), self._buf_size-5)
            rsize = rsize - rsize%4 # correction for base64
            cmd = "AT+GETDATA={}".format(rsize).encode("utf-8")
            chunk = self._serial_ask(cmd)
            if chunk.find(b"DATA=") == 0:
                data += chunk[5:-1] # removes DATA= prefix and \n suffix
        return base64.decodebytes(data)
    
    def put_file(self, data:bytes, file_path:str) -> bool:
        """Puts a file on device.
        
        Parameters:
            data(bytes): file content.
            file_path(str): target path.
        
        Returns:
            bool: True if transfer succeeded, False otherwise.
        """
        # tries to open file; if ok, returns OK, otherwise ERROR
        b64data = base64.standard_b64encode(data)
        nbytes = len(b64data)
        cmd = "AT+PUTFILE={path},{len}".format(path=file_path, len=nbytes).encode("utf-8")
        if not self._test_reply(self._serial_ask(cmd)): return False
        # transfers data
        n0 = 0
        while nbytes:
            # chunk size (must be a multiple of 4 for b64 decoding)
            csize = min(nbytes, self._buf_size-12) # max size - command size
            csize = csize - csize % 4
            cmd = b"AT+PUTDATA=" + b64data[n0:n0+csize]
            ret = self._serial_ask(cmd)
            if not self._test_reply(ret):
                # couldn't send data => abort
                self._serial_ask(b"AT+ABORT")
                self._ser.flush()
                return False
            n0 += csize
            nbytes -= csize
        return True

    def delete(self, path:str) -> bool:
        """Deletes a file or directory on device.
        
        Parameters:
            path(str): file/directory path.
        
        Returns:
            bool: True if deletion succeeded, False otherwise.
        """
        cmd = "AT+DELETE={}".format(path).encode("utf-8")
        self._serial_ask(cmd).find(b"OK")==0
    
    def make_dir(self, dir_path:str) -> bool:
        """Creates a directory on device.
        
        Parameters:
            dir_path(str): directory path.
        
        Returns:
            bool: True if creation succeeded, False otherwise.
        """
        cmd = "AT+MAKEDIR={}".format(dir_path).encode("utf-8")
        self._serial_ask(cmd).find(b"OK")==0

    def get_conf(self, ns:str, key:str):
        """Gets configuration key from device.
        
        Parameters:
            ns(str): namespace.
            key(str): key.
        
        Returns:
            Configuration key content if successful, None otherwise.
        """
        cmd = "AT+GETCONF={ns},{key}".format(ns=ns, key=key).encode("utf-8")
        value = self._serial_ask(cmd)
        if not self._test_reply(value): return
        if value.find(b"VALUE=")!=0: return
        val_type_str, val_data = value.split(b"=",1)[-1].split(b",",1)
        val_type = int(val_type_str.strip())
        if val_type in [NVSType.Str, NVSType.Blob]:
            return val_data
        if val_type in [NVSType.U16, NVSType.U32, NVSType.U64, NVSType.I8,
                        NVSType.I16, NVSType.I32, NVSType.I64]:
            return int(val_data.strip())
        if val_type == NVSType.U8:
            return ord(val_data.strip())
            

    def set_conf(self, ns:str, key:str, value, val_type:NVSType) -> bool:
        """Sets configuration key on device.
        
        Parameters:
            ns(str): namespace.
            key(str): key.
            value: value.
            val_type(NVSType): key type.
        
        Returns:
            bool: True if operation succeeded, False otherwise.
        """
        cmd = "AT+SETCONF={ns},{key},{val_type},{value}"\
               .format(ns=ns, key=key, val_type=val_type, value=value).encode("utf-8")
        return self._serial_ask(cmd).find(b"OK")==0

    def del_conf(self, ns:str, key:str) -> bool:
        """Deletes configuration key from device.
        
        Parameters:
            ns(str): namespace.
            key(str): key.
        
        Returns:
            bool: True if deletion succeeded, False otherwise.
        """
        cmd = "AT+DELCONF={ns},{key}".format(ns=ns, key=key).encode("utf-8")
        return self._serial_ask(cmd).find(b"OK")==0
    
    def get_firmware_version(self) -> str:
        """Gets device firmware version.
        
        Returns:
            str: firmware version string.
        """
        return self._serial_ask(b"AT+GETFWVER")
