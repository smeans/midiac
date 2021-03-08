import mido
import serial
import time
import sys
import datetime
import signal

mid = mido.MidiFile('sandstorm.mid') #the input MIDI file


print(mid.tracks) #print out the track names in the file

#DISABLE STEPPER DRIVER WHEN NOT PLAYING A NOTE TO REDUCE MOTOR STRESS AND HEATING

'''#track = mido.MidiTrack()
mid = mido.MidiFile()
for i, tracks in enumerate(input.tracks):
    track = mido.MidiTrack()
    mid.tracks.append(track)
    for msg in tracks:
        #if not msg.type == "control_change" and not msg.type == "program_change":
        track.append(msg)

mid.save('new_song.mid')'''


floppy1 = serial.Serial('/dev/cu.usbserial-144320', baudrate = 115200, timeout = 0.007)
floppy2 = serial.Serial('/dev/cu.usbserial-144340', baudrate = 115200, timeout = 0.007)
scanners = serial.Serial('/dev/cu.usbserial-144310', baudrate = 115200, timeout = 0.007)
dotMatrix = serial.Serial('/dev/cu.usbserial-144310', baudrate = 115200, timeout = 0.007)
percussion1 = serial.Serial('/dev/cu.usbserial-144330', baudrate = 115200, timeout = 0.007) #open the serial connections to the Arduinos
time.sleep(4) #wait for the sound modules to reset
currentlyPlaying = []
currentlyPlaying1 = []
currentlyPlaying2 = []
currentlyPlaying3 = []
currentlyPlaying4 = []
currentlyPlaying5 = []
currentlyPlaying6 = []


def valmap(value, istart, istop, ostart, ostop):
  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart))

#floppyModule controls a stack of floppy drives
#printerModule plays music on the carriage motor of a printer (adjust the maxSteps to account for different printers)
#percussionModule plays percussion on hard drives connected through a L293D and differentiates between high and low drum notes
#hardDriveModule plays music on the heads of hard drives and supports multiple independent drives, thus requiring commands to be sent with a drive number at the end (adjust the velocity to clickDelay mapping if the drives are clicking instead of playing notes)
#dotMatrixModule plays music on a dot matrix print head and supports multiple indepenedent pins, thus requiring commands to be sent with a drive number at the end

#purple to orange, blue to brown, grey to yellow, green to black

#make system for assigning instruments to channels more intuitive.
#fix currentlyPlaying so that they are all in one list and array size is determined by number of instruments.
#put some of this stuff in functions so that the code is simpler
#add comments to Python code

try:
    for msg in mid.play(): #play the MIDI file
        print(msg)
        #e1m1 is 2 on both floppies and 0 and 1 on the scanners
        #sail is 0, 0, 0, 4, 11, 14, 9
        #believer is 8 on both floppies, 2 and 6 on the dot matrix, and 3 on the scanner
        #sandstorm is 4 and 11 on floppies, 6 and 7 on the dot matrix, and 5 on the scanner
        #candionEdited2 is 1 and 2 on the floppies and 0 and 3 on the scanners
        #bloxonius is 3 on both floppies and 1 and 2 on the scanners
        if msg.channel == 8:
            #print(msg)
            if msg.type == 'pitchwheel':
                floppy1.write(bytes(str('123') + str(int(valmap(msg.pitch, -8192, 8192, 0, 16384))).zfill(5) + "\n", 'utf8'))
            if msg.type == 'note_on':
                if msg.velocity != 0 and not msg.note in currentlyPlaying1:
                    currentlyPlaying1.append(msg.note)
                    currentlyPlaying1.append(msg.velocity)
                #print(msg)
                floppy1.write(bytes(str(msg.note) + str(msg.velocity).zfill(3) + "\n", 'utf8')) #if we get a note_on, print it and write it to the arduino
            if msg.type == 'note_off':
                for index, value in enumerate(currentlyPlaying1):
                    if value == msg.note:
                        currentlyPlaying1.remove(value)
                        currentlyPlaying1.pop(index)
                #print(msg)
                floppy1.write(bytes(str(msg.note) + "000" + "\n", 'utf8')) #if we get a note_off, write it to the Arduino and set the velocity to 0 to turn the note off
                if len(currentlyPlaying1) > 0:
                    floppy1.write(bytes(str(currentlyPlaying1[0]) + str(currentlyPlaying1[1]).zfill(3) + "\n", 'utf8')) #if we get a note_on, print it and write it to the arduino

        if msg.channel == 8:
            #print(msg)
            if msg.type == 'pitchwheel':
                floppy2.write(bytes(str('123') + str(int(valmap(msg.pitch, -8192, 8192, 0, 16384))).zfill(5) + "\n", 'utf8'))
            if msg.type == 'note_on':
                if msg.velocity != 0 and not msg.note in currentlyPlaying2:
                    currentlyPlaying2.append(msg.note)
                    currentlyPlaying2.append(msg.velocity)
                #print(msg)
                floppy2.write(bytes(str(msg.note) + str(msg.velocity).zfill(3) + "\n", 'utf8')) #if we get a note_on, print it and write it to the arduino
            if msg.type == 'note_off':
                for index, value in enumerate(currentlyPlaying2):
                    if value == msg.note:
                        currentlyPlaying2.remove(value)
                        currentlyPlaying2.pop(index)
                #print(msg)
                floppy2.write(bytes(str(msg.note) + "000" + "\n", 'utf8')) #if we get a note_off, write it to the Arduino and set the velocity to 0 to turn the note off
                if len(currentlyPlaying2) > 0:
                    floppy2.write(bytes(str(currentlyPlaying2[0]) + str(currentlyPlaying2[1]).zfill(3) + "\n", 'utf8')) #if we get a note_on, print it and write it to the arduino

        if msg.channel == 3:
              if msg.type == 'pitchwheel':
                  scanners.write(bytes(str('123') + str(int(valmap(msg.pitch, -8192, 8192, 0, 16384))).zfill(5) + "0" + "\n", 'utf8'))
              if msg.type == 'note_on':
                  if msg.velocity != 0 and not msg.note in currentlyPlaying3:
                      currentlyPlaying3.append(msg.note)
                      currentlyPlaying3.append(msg.velocity)
                  #print(msg)
                  scanners.write(bytes(str(msg.note) + str(msg.velocity).zfill(3) + "0" + "\n", 'utf8')) #if we get a note_on, print it and write it to the arduino
              if msg.type == 'note_off':
                  for index, value in enumerate(currentlyPlaying3):
                      if value == msg.note:
                          currentlyPlaying3.remove(value)
                          currentlyPlaying3.pop(index)
                  #print(msg)
                  scanners.write(bytes(str(msg.note) + "000" + "0" + "\n", 'utf8')) #if we get a note_off, write it to the Arduino and set the velocity to 0 to turn the note off
                  if len(currentlyPlaying3) > 0:
                      scanners.write(bytes(str(currentlyPlaying3[0]) + str(currentlyPlaying3[1]).zfill(3) + "0" + "\n", 'utf8'))

        if msg.channel == 3:
              #print(currentlyPlaying)
              if msg.type == 'pitchwheel':
                  scanners.write(bytes(str('123') + str(int(valmap(msg.pitch, -8192, 8192, 0, 16384))).zfill(5) + "1" + "\n", 'utf8'))
              if msg.type == 'note_on':
                  if msg.velocity != 0 and not msg.note in currentlyPlaying:
                      currentlyPlaying.append(msg.note)
                      currentlyPlaying.append(msg.velocity)
                  #print(msg)
                  scanners.write(bytes(str(msg.note) + str(msg.velocity).zfill(3) + "1" + "\n", 'utf8')) #if we get a note_on, print it and write it to the arduino
              if msg.type == 'note_off':
                  for index, value in enumerate(currentlyPlaying):
                      if value == msg.note:
                          currentlyPlaying.remove(value)
                          currentlyPlaying.pop(index)
                  #print(msg)
                  scanners.write(bytes(str(msg.note) + "000" + "1" + "\n", 'utf8')) #if we get a note_off, write it to the Arduino and set the velocity to 0 to turn the note off
                  if len(currentlyPlaying) > 0:
                      scanners.write(bytes(str(currentlyPlaying[0]) + str(currentlyPlaying[1]).zfill(3) + "1" + "\n", 'utf8'))
        if msg.channel == 2:
            if msg.type == 'pitchwheel':
                dotMatrix.write(bytes(str('123') + str(int(valmap(msg.pitch, -8192, 8192, 0, 16384))).zfill(5) + "0" + "\n", 'utf8'))
            if msg.type == 'note_on':
                if msg.velocity != 0:
                    currentlyPlaying3.append(msg.note)
                    currentlyPlaying3.append(msg.velocity)
                #print(msg)
                dotMatrix.write(bytes(str(msg.note) + str(msg.velocity).zfill(3) + "0" + "\n", 'utf8')) #if we get a note_on, print it and write it to the arduino
            if msg.type == 'note_off':
                for index, value in enumerate(currentlyPlaying3):
                    if value == msg.note:
                        currentlyPlaying3.remove(value)
                        currentlyPlaying3.pop(index)
                #print(msg)
                dotMatrix.write(bytes(str(msg.note) + "000" + "0" + "\n", 'utf8')) #if we get a note_off, write it to the Arduino and set the velocity to 0 to turn the note off
                if len(currentlyPlaying3) > 0:
                    dotMatrix.write(bytes(str(currentlyPlaying3[0]) + str(currentlyPlaying3[1]).zfill(3) + "0" + "\n", 'utf8'))
        if msg.channel == 6:
            if msg.type == 'pitchwheel':
                dotMatrix.write(bytes(str('123') + str(int(valmap(msg.pitch, -8192, 8192, 0, 16384))).zfill(5) + "1" + "\n", 'utf8'))
            if msg.type == 'note_on':
                if msg.velocity != 0:
                    currentlyPlaying4.append(msg.note)
                    currentlyPlaying4.append(msg.velocity)
                #print(msg)
                dotMatrix.write(bytes(str(msg.note) + str(msg.velocity).zfill(3) + "1" + "\n", 'utf8')) #if we get a note_on, print it and write it to the arduino
            if msg.type == 'note_off':
                for index, value in enumerate(currentlyPlaying4):
                    if value == msg.note:
                        currentlyPlaying4.remove(value)
                        currentlyPlaying4.pop(index)
                #print(msg)
                dotMatrix.write(bytes(str(msg.note) + "000" + "1" + "\n", 'utf8')) #if we get a note_off, write it to the Arduino and set the velocity to 0 to turn the note off
                if len(currentlyPlaying4) > 0:
                    dotMatrix.write(bytes(str(currentlyPlaying4[0]) + str(currentlyPlaying4[1]).zfill(3) + "1" + "\n", 'utf8'))
        if msg.channel == 999:
            if msg.type == 'pitchwheel':
                dotMatrix.write(bytes(str('123') + str(int(valmap(msg.pitch, -8192, 8192, 0, 16384))).zfill(5) + "2" + "\n", 'utf8'))
            if msg.type == 'note_on':
                if msg.velocity != 0:
                    currentlyPlaying5.append(msg.note)
                    currentlyPlaying5.append(msg.velocity)
                #print(msg)
                dotMatrix.write(bytes(str(msg.note) + str(msg.velocity).zfill(3) + "2" + "\n", 'utf8')) #if we get a note_on, print it and write it to the arduino
            if msg.type == 'note_off':
                for index, value in enumerate(currentlyPlaying5):
                    if value == msg.note:
                        currentlyPlaying5.remove(value)
                        currentlyPlaying5.pop(index)
                #print(msg)o
                dotMatrix.write(bytes(str(msg.note) + "000" + "2" + "\n", 'utf8')) #if we get a note_off, write it to the Arduino and set the velocity to 0 to turn the note off
                if len(currentlyPlaying5) > 0:
                    dotMatrix.write(bytes(str(currentlyPlaying5[0]) + str(currentlyPlaying5[1]).zfill(3) + "2" + "\n", 'utf8'))
        if msg.channel == 999:
            if msg.type == 'pitchwheel':
                dotMatrix.write(bytes(str('123') + str(int(valmap(msg.pitch, -8192, 8192, 0, 16384))).zfill(5) + "3" + "\n", 'utf8'))
            if msg.type == 'note_on':
                if msg.velocity != 0:
                    currentlyPlaying6.append(msg.note)
                    currentlyPlaying6.append(msg.velocity)
                #print(msg)
                dotMatrix.write(bytes(str(msg.note) + str(msg.velocity).zfill(3) + "3" + "\n", 'utf8')) #if we get a note_on, print it and write it to the arduino
            if msg.type == 'note_off':
                for index, value in enumerate(currentlyPlaying6):
                    if value == msg.note:
                        currentlyPlaying6.remove(value)
                        currentlyPlaying6.pop(index)
                #print(msg)
                dotMatrix.write(bytes(str(msg.note) + "000" + "3" + "\n", 'utf8')) #if we get a note_off, write it to the Arduino and set the velocity to 0 to turn the note off
                if len(currentlyPlaying6) > 0:
                    dotMatrix.write(bytes(str(currentlyPlaying6[0]) + str(currentlyPlaying6[1]).zfill(3) + "3" + "\n", 'utf8'))
        if msg.channel == 9:
            if msg.type == 'note_on':
                percussion1.write(bytes(str(msg.note) + str(msg.velocity).zfill(3) + "\n", 'utf8'))
                #print(msg)
except:
    floppy1.write(bytes(str(msg.note) + "000" + "\n", 'utf8'))
    floppy2.write(bytes(str(msg.note) + "000" + "\n", 'utf8'))
    scanners.write(bytes(str(msg.note) + "000" + "0" + "\n", 'utf8'))
    scanners.write(bytes(str(msg.note) + "000" + "1" + "\n", 'utf8'))
    time.sleep(2)
    floppy1.write(bytes(str("1024" + "\n"), 'utf8'))
    floppy2.write(bytes(str("1024" + "\n"), 'utf8'))
    scanners.write(bytes(str("1024" + "\n"), 'utf8'))
    percussion1.write(bytes(str("1024" + "\n"), 'utf8'))
    time.sleep(1)
    floppy1.close()
    floppy2.close()
    scanners.close()
    percussion1.close()
    sys.exit(0)

#dotMatrix.write(bytes(1024))
floppy1.write(bytes(str(msg.note) + "000" + "\n", 'utf8'))
floppy2.write(bytes(str(msg.note) + "000" + "\n", 'utf8'))
scanners.write(bytes(str(msg.note) + "000" + "0" + "\n", 'utf8'))
scanners.write(bytes(str(msg.note) + "000" + "1" + "\n", 'utf8'))
time.sleep(5)
floppy1.write(bytes(str("1024" + "\n"), 'utf8'))
floppy2.write(bytes(str("1024" + "\n"), 'utf8'))
scanners.write(bytes(str("1024" + "\n"), 'utf8'))
percussion1.write(bytes(str("1024" + "\n"), 'utf8'))
floppy1.close()
floppy2.close()
scanners.close()
percussion1.close()
#dotMatrix.close() #close connections to all Arduinos
