import mido
from . import sm
import time
from time import sleep
import pprint
pp = pprint.PrettyPrinter(indent=4)
# !!!LATER!!! refactor to be managed by config.

# sm.SoundModule('floppy1', '/dev/ttyUSB1', 'floppy'), sm.SoundModule('floppy2', '/dev/ttyUSB2', 'floppy'), sm.SoundModule('percussion0', '/dev/ttyUSB3', 'percussion')
sound_modules = [sm.SoundModule('floppy0', '/dev/ttyUSB1', 'floppy'), sm.SoundModule('floppy1', '/dev/ttyUSB2', 'floppy'), sm.SoundModule('percussion0', '/dev/ttyUSB0', 'percussion') ]

def queue(midi_file):
    unwrapped_notes = unwrap_midi(midi_file)
    audio.playUnwrappedNotes(unwrapped_notes)

def play(midi_file):
    unwrapped_notes = unwrap_midi(midi_file)
    distribute_notes(unwrapped_notes)

    start_time = time.clock() + 5

    for sound_module in sound_modules:
        status = sound_module.play((start_time-time.clock()) * 1000000)

        print '%s: arduino status: %s' % (sound_module.sm_type, status)

def control(command):
    if not 'action' in command:
        print 'bad command message %s' % command
        return None

    for sound_module in sound_modules:
        sound_module.control(command)

def distribute_notes(unwrapped_notes):
    notes = [[] for _ in range(len(sound_modules))]
    unplayable_notes = 0

    for note in unwrapped_notes:
        winning_bid = 0.0
        winning_bid_sound_module = None

        for sound_module in sound_modules:
            bid = sound_module.bid_on_note(notes[sound_modules.index(sound_module)], note)
            if bid > winning_bid:
                winning_bid = bid
                winning_bid_sound_module = sound_module

        if winning_bid_sound_module != None:
            notes[sound_modules.index(winning_bid_sound_module)].append(note)
        else:
            print 'unplayable note %s' % note
            unplayable_notes += 1

    if unplayable_notes > 0:
        print 'Unplayable Notes %d/%d (%f%%)' % (unplayable_notes, len(unwrapped_notes), float(unplayable_notes) / float(len(unwrapped_notes))*100.0)

    for i in range(len(sound_modules)):
        calc_delays(notes[i])
        # !!!HACK!!! simultaneous notes yield negative delays; we need multiple
        # floppy modules to play
        notes[i] = [note for note in notes[i] if note['delay'] >= 0]
        sound_modules[i].upload_sketch(notes[i])

def calc_delays(notes):
    if len(notes) <= 0:
        return

    notes[0]['delay'] = notes[0]['startTime']
    for i in range(1, len(notes)):
        notes[i]['delay'] = notes[i]['startTime'] - (notes[i-1]['startTime'] + notes[i-1]['duration'])

def unwrap_midi(midi_file):
    def calc_velocity(velocities):
        return reduce(lambda v, fv: max(v, fv), velocities.itervalues(), 0)

    unwrapped_notes = []
    note_array = [None]*128

    midi_time = 0

    mid = mido.MidiFile(file=midi_file)

    print 'ticks_per_beat %d' % mid.ticks_per_beat

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
        elif message.type == 'set_tempo':
            pp.pprint(message)
        elif message.type == 'time_signature':
            pp.pprint(message)


    return sorted(unwrapped_notes, key=lambda note: note['startTime'])
