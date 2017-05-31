from . import mido_pkg as mido
from . import audio

def queue(midi_file):
    unwrapped_notes = unwrap_midi(midi_file)
    audio.playUnwrappedNotes(unwrapped_notes)


def unwrap_midi(midi_file):
    unwrapped_notes = []
    note_array = [None]*128

    midi_time = 0

    mid = mido.MidiFile(file=midi_file)

    for message in mid:
        if message.type == 'note_on':
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
                        'duration': midi_time-note_array[message.note]['startTime']})
                    note_array[message.note] = None

        midi_time += message.time
        print 'midi_time %f message.time %f' % (midi_time, message.time)

    return unwrapped_notes
