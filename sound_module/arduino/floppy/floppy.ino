#include "C://Users/alexe/SourceControl/midiac/sound_module/arduino/smlib.ino"
#include "C://Users/alexe/SourceControl/midiac/sound_module/arduino/notemicros.ino"

const int pins_per_floppy = 2;
#define step_pin  0
#define dir_pin   1
#define dir_forward   LOW
#define dir_reverse   HIGH
const int max_floppy_steps = 80;

int floppy_pins[] = {2,4,6,8,10,12,14,16};
#define floppy_count ((int)(sizeof(floppy_pins) / sizeof(floppy_pins[0])))
int floppy_position[floppy_count];
int floppy_dir[floppy_count];
int group_size = MAX_AMPLITUDE / floppy_count;

void setup() {
  sm_setup();

  for (int i = 0; i < floppy_count; i++) {
    for (int j = 0; j < pins_per_floppy; j++) {
      pinMode(floppy_pins[i]+j, OUTPUT);
    }
  }

  reset_drives();
}

bool playing = false;
bool in_note = false;
int current_note = 0;
unsigned long note_start_micros;
unsigned long note_interval;
unsigned int floppies_playing;
unsigned long last_millis;
unsigned long elapsed_millis;
unsigned long song_millis;

void reset_drives() {
  for (int f = 0; f < floppy_count; f++) {
    digitalWrite(floppy_pins[f]+dir_pin, dir_reverse);
  }

  for (int i = 0; i < max_floppy_steps; i++) {
    for (int s = 0; s < 2; s++) {
      for (int f = 0; f < floppy_count; f++) {
        digitalWrite(floppy_pins[f]+step_pin, s ? LOW : HIGH);
      }

      delay(1);
    }
  }

  for (int f = 0; f < floppy_count; f++) {
    floppy_position[f] = 0;
    floppy_dir[f] = dir_forward;
  }
}

void pulse_drives(int floppies_playing) {
  for (int f = 0; f < floppies_playing; f++) {
    digitalWrite(floppy_pins[f]+step_pin, floppy_position[f]++ % 2 ? LOW : HIGH);
    if (floppy_position[f] > max_floppy_steps * 2) {
      floppy_dir[f] = floppy_dir[f] == dir_forward ? dir_reverse : dir_forward;
      digitalWrite(floppy_pins[f]+dir_pin, floppy_dir[f]);
      floppy_position[f] = 0;
    }
  }
}

void reset_song() {
  playing = false;
  in_note = false;
  note_start_micros = 0;
  current_note = 0;
  elapsed_millis = 0;
  song_millis = 0;
}

void loop() {
  switch (getMessage()) {
    case MSG_LOADNOTES: {
      current_note = 0;
      reset_drives();
    } break;

    case MSG_PLAY: {
      playing = true;
      last_millis = millis();
      Serial.println("MSG_PLAY: OK");
    } break;

    case MSG_PAUSE: {
      playing = false;
      Serial.println("MSG_PAUSE: OK");
    } break;

    case MSG_STOP: {
      reset_song();
      Serial.println("MSG_STOP: OK");
    } break;

    case MSG_INFO: {
      Serial.println("MSG_INFO: OK: FLOPPY MODULE");
    }
  }

  if (playing) {
    unsigned long new_millis = millis();

    elapsed_millis += new_millis - last_millis;
    last_millis = new_millis;

    if (in_note && elapsed_millis > note_buffer[current_note].delay
        + note_buffer[current_note].duration + song_millis) {
      in_note = false;
      song_millis += note_buffer[current_note].delay + note_buffer[current_note].duration;
      if (++current_note >= note_count) {
        reset_song();
      }
    }

    if (!in_note && elapsed_millis > note_buffer[current_note].delay + song_millis) {
      in_note = true;
      note_start_micros = micros();
      note_interval = note_micros[(int)note_buffer[current_note].note];
      floppies_playing = (int)(((float)note_buffer[current_note].velocity/(float)MAX_AMPLITUDE) * (float)floppy_count);
    }

    if (in_note) {
      if ((micros() - note_start_micros) >= note_interval) {
        note_start_micros = micros();
        pulse_drives(floppies_playing);
        delayMicroseconds(20);
        pulse_drives(floppies_playing);
      }
    }
  }
}
