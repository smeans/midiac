import mido

def queue(midi_file):
    unwrapped_notes = unwrap_midi(midi_file)

    print(unwrapped_notes)


def unwrap_midi(midi_file):
    unwrapped_notes = []
    note_array = [None]*128

    parser = mido.Parser()
    parser.feed(midi_file.read())

    midi_time = 0

    for message in parser:
        print(message)
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

        midi_time += message.time
        print('midi_time: %d message_time: %d' % (midi_time, message.time))

    return unwrapped_notes
