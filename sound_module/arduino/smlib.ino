void sm_setup() {
  Serial.begin(9600);
  Serial.println("Ready!");
}

const uint8_t MSG_BASE = 0x3e;
const uint8_t MSG_RESET = MSG_BASE;
const uint8_t MSG_LOADNOTES = MSG_BASE+1;
const uint8_t MSG_PLAY = MSG_BASE+2;
const uint8_t MSG_PAUSE = MSG_BASE+3;
const uint8_t MSG_STOP = MSG_BASE+4;
const uint8_t MSG_VOLUME = MSG_BASE+5;
const uint8_t MSG_INFO = MSG_BASE+6;

struct NOTE {
  unsigned short delay;
  unsigned short duration;
  byte note;
  byte velocity;
};

NOTE *note_buffer = NULL;
int note_count;
const int MAX_AMPLITUDE = 127;
byte volume = MAX_AMPLITUDE;

void readSerialBytes(int cb, byte *bytes) {
  while (cb > 0) {
    int cba;

    while ((cba = Serial.available()) <= 0) {
      delay(10);
    }

    cba = min(cb, cba);

    while (cba-- > 0) {
      *bytes++ = Serial.read();
      cb--;
    }
  }
}

void resetSoundModule() {
  if (note_buffer) {
    free(note_buffer);

    note_buffer = NULL;
  }
}

int getMessage() {
  if (Serial.available()) {
    uint8_t msg = Serial.read();
    switch (msg) {
      case MSG_RESET: {
        resetSoundModule();

        Serial.println("MSG_RESET: OK");
      } break;

      case MSG_PLAY: {
        Serial.println("MSG_PLAY: OK");
      } break;

      case MSG_LOADNOTES: {
        resetSoundModule();

        unsigned long buffer_len;

        readSerialBytes(sizeof(buffer_len), (byte *)&buffer_len);
        if (note_buffer = (NOTE *)malloc(buffer_len)) {
          note_count = buffer_len/sizeof(*note_buffer);
          readSerialBytes(buffer_len, (byte *)note_buffer);
          Serial.print("MSG_LOADNOTES: OK: ");
          Serial.print(note_count);
          Serial.println(" notes loaded");
        } else {
          Serial.println("MSG_LOADNOTES: ERROR: could not allocate buffer");
        }
      } break;

      case MSG_VOLUME: {
        readSerialBytes(sizeof(volume), &volume);
        Serial.println("MSG_VOLUME: OK");
      } break;

      case MSG_INFO: {} break;

      default: {
        Serial.print("unknown message: ");
        Serial.print(msg, HEX);
        Serial.print('\n');
      } break;
    }

    return msg;
  }

  return 0;
}
