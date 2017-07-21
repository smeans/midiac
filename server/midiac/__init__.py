import mido
from . import audio
from . import sm
from time import sleep
import pprint
pp = pprint.PrettyPrinter(indent=4)
# !!!LATER!!! refactor to be managed by config.

sound_modules = [sm.SoundModule('COM5', 'percussion')]

def queue(midi_file):
    unwrapped_notes = unwrap_midi(midi_file)
    audio.playUnwrappedNotes(unwrapped_notes)
    #floppy_sm.reset()
    #while(1):
        #print(floppy_sm.readStatus())
        #sleep(.1)

def play(midi_file):
    unwrapped_notes = unwrap_midi(midi_file)
    distribute_notes(unwrapped_notes)

    for sound_module in sound_modules:
        status = sound_module.play()

        print '%s: arduino status: %s' % (sound_module.sm_type, status)

def distribute_notes(unwrapped_notes):
    notes = [[]] * len(sound_modules)

    for note in unwrapped_notes:
        winning_bid = 0.0
        winning_bid_sound_module = None

        for sound_module in sound_modules:
            bid = sound_module.bid_on_note(note)
            if bid > winning_bid:
                winning_bid = bid
                winning_bid_sound_module = sound_module

        if winning_bid_sound_module != None:
            notes[sound_modules.index(winning_bid_sound_module)].append(note)
        else:
            print 'unplayable note %s' % note

    for i in range(len(sound_modules)):
        status = sound_modules[i].load_notes(notes[i])
        print '%s: arduino status: %s' % (sound_modules[i].sm_type, status)

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
