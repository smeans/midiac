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
    audio.playUnwrappedNotes(unwrapped_notes)
    #floppy_sm.reset()
    #while(1):
        #print(floppy_sm.readStatus())
        #sleep(.1)


def unwrap_midi(midi_file):
    def calc_velocity(velocities):
        return reduce(lambda v, fv: max(v, fv), velocities.itervalues(), 0)

    unwrapped_notes = []
    note_array = [None]*128

    midi_time = 0

    mid = mido.MidiFile(file=midi_file)

    for message in mid:
        midi_time += message.time

        if message.type == 'note_on':
            if note_array[message.note]:
                note_array[message.note]['velocities'][message.channel] = message.velocity
                new_velocity = calc_velocity(note_array[message.note]['velocities'])
                if new_velocity != note_array[message.note]['velocity']:
                    unwrapped_notes.append({'note': message.note,
                        'startTime': note_array[message.note]['startTime'],
                        'velocity': note_array[message.note]['velocity'],
                        'duration': midi_time-note_array[message.note]['startTime'] if message.channel != 9 else 0})
                    note_array[message.note]['startTime'] = midi_time
                    note_array[message.note]['velocity'] = new_velocity
            else:
                note_array[message.note] = {'startTime': midi_time, 'velocity': message.velocity, 'velocities': {message.channel: message.velocity}}
        elif message.type =='note_off':
            if note_array[message.note]:
                try:
                    del note_array[message.note]['velocities'][message.channel]
                except:
                    pass

                if not bool(note_array[message.note]['velocities']):
                    unwrapped_notes.append({'note': message.note,
                        'startTime': note_array[message.note]['startTime'],
                        'velocity': note_array[message.note]['velocity'],
                        'duration': midi_time-note_array[message.note]['startTime'] if message.channel != 9 else 0})
                    note_array[message.note] = None
                else:
                    new_velocity = calc_velocity(note_array[message.note]['velocities'])

                    if new_velocity != note_array[message.note]['velocity']:
                        unwrapped_notes.append({'note': message.note,
                            'startTime': note_array[message.note]['startTime'],
                            'velocity': note_array[message.note]['velocity'],
                            'duration': midi_time-note_array[message.note]['startTime'] if message.channel != 9 else 0})
                        note_array[message.note]['startTime'] = midi_time
                        note_array[message.note]['velocity'] = new_velocity

    return unwrapped_notes
