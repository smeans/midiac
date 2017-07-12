import serial
import time

class SoundModule(object):
    MSG_BASE = 0x3e
    MSG_RESET = MSG_BASE
    MSG_PLAY = MSG_BASE+1

    def __init__(self, port):
        self.serial = serial.Serial(port, 9600)

    def reset(self):
        self.serial.write(str(chr(self.MSG_RESET)))

    def readStatus(self):
        while self.serial.in_waiting <= 0:
            time.sleep(1)

        return self.serial.read(self.serial.in_waiting).decode('utf8')

    def __del__(self):
        self.serial.close()
