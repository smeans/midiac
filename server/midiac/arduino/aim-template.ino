#include "/home/pi/midiac/sound_module/arduino/smlib.ino"
#include "/home/pi/midiac/sound_module/arduino/notemicros.ino"

const NOTE song_buffer[] PROGMEM = {
    $notes
};

#include "C:\Users\alexe\SourceControl\midiac\sound_module\arduino\songbuffer.ino"

#define fire_pin    2

int head_pins[] = {2, 3, 4, 5, 6};
#define head_pin_count ((int)(sizeof(head_pins) / sizeof(head_pins[0])))
int current_head_pin = 0;
unsigned int pinStatus;

bool playing = false;
bool in_note = false;
int current_note = 0;
unsigned long note_start_micros;
unsigned long note_interval;
unsigned long last_millis;
unsigned long elapsed_millis;
unsigned long song_millis;

void setup() {
  sm_setup();

  for (int i = 0; i < head_pin_count; i++) {
    pinMode(head_pins[i], OUTPUT);
  }

  playing=true;
}

void reset_head_pins() {
  for (int i = 0; i < head_pin_count; i++) {
    digitalWrite(head_pins[i], LOW);
  }
}

void pulse_head_pin() {
  unsigned int status = pinStatus & 1 << current_head_pin;
  digitalWrite(head_pins[current_head_pin], status ? LOW : HIGH);
  if (status) {
    pinStatus &= ~(1 << current_head_pin);
  } else {
    pinStatus |= 1 << current_head_pin;
  }

  current_head_pin++;
  if (current_head_pin >= head_pin_count) {
    current_head_pin = 0;
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
  unsigned long new_millis = millis();

  switch (getMessage()) {
    case MSG_LOADNOTES: {
      current_note = 0;
    } break;

    case MSG_PLAY: {
      playing = true;
      last_millis = new_millis;
      Serial.println("MSG_PLAY: OK");
    } break;

    case MSG_PAUSE: {
      playing = false;
      reset_head_pins();
      Serial.println("MSG_PAUSE: OK");
    } break;

    case MSG_STOP: {
      reset_song();
      reset_head_pins();
      Serial.println("MSG_STOP: OK");
    } break;

    case MSG_INFO: {
      Serial.println("MSG_INFO: OK: AIM MODULE");
    }
  }

  if (playing) {
    elapsed_millis += new_millis - last_millis;
    last_millis = new_millis;

    if (in_note && elapsed_millis > PSONG(current_note)->delay
        + PSONG(current_note)->duration + song_millis) {
      in_note = false;
      song_millis += PSONG(current_note)->delay + PSONG(current_note)->duration;
      if (++current_note >= PSONG_COUNT) {
        reset_song();
      }
    }

    if (!in_note && elapsed_millis > PSONG(current_note)->delay + song_millis) {
      in_note = true;
      note_start_micros = micros();
      note_interval = note_micros[(int)PSONG(current_note)->note];
    }

    if (in_note) {
      if ((micros() - note_start_micros) >= note_interval) {
        note_start_micros = micros();
        pulse_head_pin();
      }
    } else {
      reset_head_pins();
    }
  }
}
