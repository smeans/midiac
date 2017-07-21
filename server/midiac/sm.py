import serial
import time
import struct

class SoundModule(object):
    MSG_BASE = 0x3e
    MSG_RESET = MSG_BASE
    MSG_LOADNOTES = MSG_BASE+1
    MSG_PLAY = MSG_BASE+2

    def __init__(self, port, sm_type):
        self.serial = serial.Serial(port, 9600)
        print '%s: arduino status: %s' % (sm_type, self.read_status())

        self.sm_type = sm_type

    def reset(self):
        self.serial.write(str(chr(self.MSG_RESET)))

        return self.read_status()

    def bid_on_note(self, note):
        if self.sm_type == 'percussion':
            return 1.0 if note['duration'] == 0 else 0.0
        else:
            return 1.0

    def read_status(self):
        status = ''

        while status.find('\n') < 0:
            while self.serial.in_waiting <= 0:
                time.sleep(1)

            status += self.serial.read(self.serial.in_waiting).decode('utf8')

        return status

    def load_notes(self, unwrapped_notes):
        def note_to_bin(note):
            start_time = int(note['startTime'] * 1000)
            end_time = start_time + int(note['duration'] * 1000)
            return struct.pack('<LLBB', start_time, end_time, note['note'], note['velocity'])

        self.serial.write(str(chr(self.MSG_LOADNOTES)))
        buffer_len = len(unwrapped_notes) * len(note_to_bin(unwrapped_notes[0]))
        # !!!TODO!!! send length and all note structures to arduino
        self.serial.write(struct.pack('<L', buffer_len))

        print 'buffer_len %d note struct size: %d' % (buffer_len, len(note_to_bin(unwrapped_notes[0])))

        for note in unwrapped_notes:
            self.serial.write(note_to_bin(note))

        return self.read_status()

    def play(self):
        self.serial.write(str(chr(self.MSG_PLAY)))

        return self.read_status()

    def __del__(self):
        self.serial.close()
