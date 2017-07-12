void setup() {
  Serial.begin(9600);
  Serial.println("Ready!");
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
        Serial.print("unknown message: ");
        Serial.print(msg, HEX);
        Serial.print('\n');
      } break;
    }
  }
}

void loop() {
  getMessage();
  delay(10);
}
