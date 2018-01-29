#include "/home/pi/midiac/sound_module/arduino/smlib.ino"

const NOTE song_buffer[] PROGMEM = {
    $notes
};

#include "/home/pi/midiac/sound_module/arduino/songbuffer.ino"

const int hd_pins[] = {2,3,4,5,6,7};
const int hd_count = sizeof(hd_pins) / sizeof(hd_pins[0]);
const int group_size = MAX_AMPLITUDE / hd_count;

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

void play_note(const NOTE *note) {
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
  unsigned long new_millis = millis();

  switch (getMessage()) {
    case MSG_LOADNOTES: {
      current_note = 0;
    } break;

    case MSG_PLAY: {
      playing = true;
      last_millis = new_millis;
      Serial.println(F("MSG_PLAY: OK"));
    } break;

    case MSG_PAUSE: {
      playing = false;
      Serial.println(F("MSG_PAUSE: OK"));
    } break;

    case MSG_STOP: {
      reset_song();
      Serial.println(F("MSG_STOP: OK"));
    } break;

    case MSG_INFO: {
      Serial.println(F("MSG_INFO: OK: PERCUSSION MODULE"));
    }
  }

  if (playing) {
    elapsed_millis += new_millis - last_millis;
    last_millis = new_millis;

    if (elapsed_millis > PSONG(current_note)->delay + song_millis) {
      play_note(PSONG(current_note));

      song_millis += PSONG(current_note)->delay + PSONG(current_note)->duration;

      if (++current_note >= PSONG_COUNT) {
        reset_song();
      }
    }
  }
}
