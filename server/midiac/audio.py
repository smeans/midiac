import sys
import math
import pyaudio

#See http://en.wikipedia.org/wiki/Bit_rate#Audio
BITRATE = 22000

def playUnwrappedNotes(unwrapped_notes):
    wave_data = makeWaveData(unwrapped_notes)

    p = pyaudio.PyAudio()
    stream = p.open(
        format=pyaudio.paFloat32,
        channels=1,
        rate=BITRATE,
        output=True,
        )
    print(str(wave_data[:BITRATE]))

    #stream.write(wave_data)
    stream.stop_stream()
    stream.close()
    p.terminate()

def getNoteFreq(midi_note):
    return 8.1757989156 * pow(2.0, float(midi_note)/12.0)

def addNote(wave_data, note):
    freq = getNoteFreq(note['note'])

    print('note %d freq %f' % (note['note'], freq))

    start_sample = int(note['startTime'] * BITRATE)
    samples_per_cycle = int(BITRATE/freq)

    for i in range(0, int(note['duration'] * BITRATE)):
        wave_data[start_sample + i] += math.sin(float(i % samples_per_cycle)/float(samples_per_cycle)*2*math.pi)

def makeWaveData(unwrapped_notes):
    total_time = reduce(lambda t, note: note['startTime'] + note['duration'] if note['startTime'] + note['duration'] > t else t, unwrapped_notes, 0)

    wave_data = [0.0] * int(total_time * BITRATE)

    for note in unwrapped_notes:
        addNote(wave_data, note)

    return wave_data
