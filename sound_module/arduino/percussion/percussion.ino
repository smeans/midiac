#include "C://Users/alexe/SourceControl/midiac/sound_module/arduino/smlib.ino"

int hd_pins[] = {2,3,4,5,6,7};
int hd_count = sizeof(hd_pins) / sizeof(hd_pins[0]);
int group_size = MAX_AMPLITUDE / hd_count;

void setup() {
  sm_setup();

  for (int i = 0; i < hd_count; i++) {
    pinMode(hd_pins[i], OUTPUT);
  }
}

bool playing = false;
int current_note = 0;
unsigned long last_millis;
unsigned long elapsed_millis;
unsigned long song_millis;

void play_note(NOTE *note) {
  unsigned short velocity = (unsigned int)((float)note->velocity * ((float)volume/(float)MAX_AMPLITUDE));

  for (int i = 0; i < hd_count; i++) {
    if (velocity / group_size > i) {
      analogWrite(hd_pins[i], MAX_AMPLITUDE);
    } else if(velocity / group_size == i) {
      analogWrite(hd_pins[i], int(float(velocity % group_size) / float(group_size) * float(MAX_AMPLITUDE)));
    } else {
      analogWrite(hd_pins[i], 0);
    }
  }

  delay(10);

  for (int i = 0; i < hd_count; i++) {
    analogWrite(hd_pins[i], 0);
  }
}

void reset_song() {
  playing = false;
  current_note = 0;
  elapsed_millis = 0;
  song_millis = 0;
}

void loop() {
  switch (getMessage()) {
    case MSG_LOADNOTES: {
      current_note = 0;
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
      Serial.println("MSG_INFO: OK: PERCUSSION MODULE");
    }
  }

  if (playing) {
    elapsed_millis += millis() - last_millis;
    last_millis = millis();

    if (elapsed_millis > note_buffer[current_note].delay + song_millis) {
      play_note(&note_buffer[current_note]);

      song_millis += note_buffer[current_note].delay + note_buffer[current_note].duration;

      if (++current_note >= note_count) {
        reset_song();
      }
    }
  }
}
