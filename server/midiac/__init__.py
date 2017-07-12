import mido
from . import audio
from . import sm
from time import sleep
import pprint
pp = pprint.PrettyPrinter(indent=4)
# !!!LATER!!! refactor to be managed by config.

floppy_sm = sm.SoundModule('COM5')

def queue(midi_file):
    unwrapped_notes = unwrap_midi(midi_file)
    pp.pprint(unwrapped_notes)
    #audio.playUnwrappedNotes(unwrapped_notes)
    #floppy_sm.reset()
    #while(1):
        #print(floppy_sm.readStatus())
        #sleep(.1)


def unwrap_midi(midi_file):
    unwrapped_notes = []
    note_array = [None]*128

    midi_time = 0

    mid = mido.MidiFile(file=midi_file)

    for message in mid:
        if message.type == 'note_on':
            print 'velocity %d channel %d' % (message.velocity, message.channel)
            if note_array[message.note]:
                note_array[message.note]['count'] += 1
            else:
                note_array[message.note] = {'count': 1, 'startTime': midi_time}
        elif message.type =='note_off':
            if note_array[message.note]:
                if note_array[message.note]['count'] > 1:
                    note_array[message.note]['count'] -= 1
                else:
                    unwrapped_notes.append({'note': message.note,
                        'startTime': note_array[message.note]['startTime'],
                        'duration': midi_time-note_array[message.note]['startTime'] if message.channel != 9 else 0})
                    note_array[message.note] = None

        midi_time += message.time

    return unwrapped_notes
