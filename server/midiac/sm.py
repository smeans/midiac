import serial
import time
import struct

class SoundModule(object):
    MAX_AMPLITUDE = 127
    MSG_BASE = 0x3e
    MSG_RESET = MSG_BASE
    MSG_LOADNOTES = MSG_BASE+1
    MSG_PLAY = MSG_BASE+2
    MSG_PAUSE = MSG_BASE+3
    MSG_STOP = MSG_BASE+4
    MSG_VOLUME = MSG_BASE+5
    MSG_INFO = MSG_BASE+6

    def __init__(self, port, sm_type):
        self.serial = serial.Serial(port, 9600)
        print '%s: arduino status: %s' % (sm_type, self.read_status())
        print '%s: %s' % (sm_type, self.info())

        self.sm_type = sm_type

    def reset(self):
        self.serial.write(str(chr(self.MSG_RESET)))

        return self.read_status()

    def bid_on_note(self, note):
        if self.sm_type == 'percussion':
            return 1.0 if note['duration'] == 0 else 0.0
        else:
            return 1.0 if note['duration'] > 0 else 0.0

    def read_status(self):
        status = ''

        while status.find('\n') < 0:
            while self.serial.in_waiting <= 0:
                time.sleep(1)

            status += self.serial.read(self.serial.in_waiting).decode('utf8')

        return status

    def load_notes(self, unwrapped_notes):
        def note_to_bin(note):
            delay = int(note['delay'] * 1000)
            duration = int(note['duration'] * 1000)

            print 'note %s: delay: %s duration: %s' % (note['note'], note['delay'], note['duration'])

            return struct.pack('<HHBB', delay, duration, note['note'], note['velocity'])

        self.serial.write(str(chr(self.MSG_LOADNOTES)))
        buffer_len = len(unwrapped_notes) * len(note_to_bin(unwrapped_notes[0]))
        # !!!TODO!!! send length and all note structures to arduino
        self.serial.write(struct.pack('<L', buffer_len))

        for note in unwrapped_notes:
            self.serial.write(note_to_bin(note))

        return self.read_status()

    def play(self):
        self.serial.write(str(chr(self.MSG_PLAY)))

        return self.read_status()

    def info(self):
        self.serial.write(str(chr(self.MSG_INFO)))

        return self.read_status()

    def control(self, command):
        message_map = {'play': self.MSG_PLAY, 'pause': self.MSG_PAUSE, 'stop': self.MSG_STOP, 'volume': self.MSG_VOLUME}

        action = command['action']

        if not action in message_map:
            print 'unrecognized command "%s"' % action
            return None

        if action != 'volume':
            self.serial.write(str(chr(message_map[action])))
        else:
            if not 'value' in command:
                print 'missing volume value: %s' % command
                return None

            self.serial.write(str(chr(message_map[action])))
            value = float(command['value'])
            new_volume = int(min(self.MAX_AMPLITUDE, max(0, value * self.MAX_AMPLITUDE)))
            print 'value %d new_volume %d' % (value, new_volume)
            self.serial.write(str(chr(new_volume)))

        return self.read_status()

    def __del__(self):
        self.serial.close()
