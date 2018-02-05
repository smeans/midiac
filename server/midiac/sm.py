import os
import serial
import time
import struct
import subprocess

from string import Template

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

    def __init__(self, id, port, sm_type):
        self.id = id
        self.port = port
        self.sm_type = sm_type
        self.serial = None

        self.open_port()

    def open_port(self):
        if self.serial:
            self.close_port()

        self.serial = serial.Serial(self.port, 9600)
        print '%s: %s: port opened: arduino status: %s' % (self.port, self.sm_type, self.read_status())
        print '%s: %s' % (self.sm_type, self.info())

    def close_port(self):
        if not self.serial:
            return

        print '%s: %s: port closed' % (self.port, self.sm_type)

        self.serial.close()
        self.serial = None

    def reset(self):
        self.serial.write(str(chr(self.MSG_RESET)))

        return self.read_status()

    def bid_on_note(self, notes, note):
        if self.sm_type == 'percussion':
            return 1.0 if note['duration'] == 0 else 0.0
        else:
            if len(notes) > 0:
                last_note = notes[-1]
                if last_note['startTime'] + last_note['duration'] > note['startTime']:
                    return 0.0

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

            return struct.pack('<HHBB', delay, duration, note['note'], note['velocity'])

        self.serial.write(str(chr(self.MSG_LOADNOTES)))
        buffer_len = len(unwrapped_notes) * len(note_to_bin(unwrapped_notes[0]))
        print "%s: %s bytes required" % (self.sm_type, buffer_len)

        # !!!TODO!!! send length and all note structures to arduino
        self.serial.write(struct.pack('<L', buffer_len))

        for note in unwrapped_notes:
            self.serial.write(note_to_bin(note))

        return self.read_status()

    def play(self, start_delay):
        self.serial.write(str(chr(self.MSG_PLAY)) + struct.pack('<L', start_delay))

        #return self.read_status()

    def info(self):
        self.serial.write(str(chr(self.MSG_INFO)))

        return self.read_status()

    def control(self, command):
        message_map = {'play': self.MSG_PLAY, 'pause': self.MSG_PAUSE, 'stop': self.MSG_STOP, 'volume': self.MSG_VOLUME}

        action = command['action']

        if not action in message_map:
            print 'unrecognized command "%s"' % action
            return None

        if action == 'play':
            self.serial.write(str(chr(self.MSG_PLAY)) + struct.pack('<L', 0))
        elif action == 'volume':
            if not 'value' in command:
                print 'missing volume value: %s' % command
                return None

            self.serial.write(str(chr(message_map[action])))
            value = float(command['value'])
            new_volume = int(min(self.MAX_AMPLITUDE, max(0, value * self.MAX_AMPLITUDE)))
            print 'value %d new_volume %d' % (value, new_volume)
            self.serial.write(str(chr(new_volume)))
        else:
            self.serial.write(str(chr(message_map[action])))

        return self.read_status()

    def make_sketch(self, unwrapped_notes):
        template_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'arduino/%s-template.ino' % self.sm_type)
        print template_path
        template_file = open(template_path, 'r')
        template = Template(''.join(template_file.readlines()))
        template_file.close()

        note_elements = ['{%s, %s, %s, %s}' % (int(note['delay']*1000), int(note['duration']*1000), note['note'], note['velocity']) for note in unwrapped_notes]
        sketch = template.substitute({'notes': ','.join(note_elements)})

        return sketch

    def upload_sketch(self, unwrapped_notes):
        sketch = self.make_sketch(unwrapped_notes)
        build_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'arduino/build/%s' % self.id)

        if not os.path.exists(build_path):
            os.makedirs(build_path)

        sketch_path = os.path.join(build_path, '%s.ino' % self.id)
        with open(sketch_path, 'w') as f:
            f.write(sketch)
            f.close()

        self.close_port()
        args = ['arduino', '--upload', sketch_path, '--port', self.port]
        print('launching %s' % ' '.join(args))
        return subprocess.Popen(args)
        #self.open_port()

    def __del__(self):
        self.serial.close()
