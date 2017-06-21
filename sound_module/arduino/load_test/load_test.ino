void setup() {
  Serial.begin(9600);
}

const uint8_t MSG_BASE = 0x3e;
const uint8_t MSG_RESET = MSG_BASE;
const uint8_t MSG_PLAY = MSG_BASE+1;

int getMessage() {
  if (Serial.available()) {
    uint8_t msg = Serial.read();

    switch (msg) {
      case MSG_RESET: {
        Serial.println("resetting");
      } break;

      case MSG_PLAY: {
        Serial.println("playing");
      } break;
      
      default: {
        Serial.println("unknown message: " + msg);
      } break;
    }
  }
}

void loop() {
  getMessage();
}
