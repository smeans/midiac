
int lowDrives[] = {3}; //the pins that all of the low-pitched hard drives are connected to
int highDrives[] = {2}; //the pins that all of the high-pitched hard drives are connected to

int lowDrums[] = {35, 36, 41, 43, 45, 46, 47, 60, 61, 66, 68, 77, 79, 86, 87}; //list of MIDI note numbers that correspond to low drums
int highDrums[] = {27, 28, 29, 30, 31, 32, 34, 37, 38, 39, 40, 42, 44, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 62, 63, 64, 65, 67, 69, 70, 71, 72, 73, 74, 75, 76, 78, 80, 81, 82, 83, 84, 85, 91, 92, 93}; //list of MIDI note numbers that correspond to high drums (83 and 84 are bells)


int note = 0; //the MIDI note that we are playing
int velocity = 0; //the MIDI velocity of our note
long int noteInfo; //the raw note and velocity information (note 83 velocity 127 would be stored as 083127)
long int clickDelay; //how long we should keep the hard drive pin HIGH before releasing it (determined by velocity)


long unsigned int parsed = 0;
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false; //variables for the serial receive function

void setup() {
  Serial.begin(115200); //duh
  Serial.setTimeout(5);
  for(int i = 0; i < sizeof(lowDrives) / sizeof(lowDrives[0]); i++){
    pinMode(lowDrives[i], OUTPUT); //iterate through each of the pins of each of the low-pitched drives and set them as outputs
    digitalWrite(lowDrives[i], LOW); //make sure that no drives are clicking
  }
  for(int i = 0; i < sizeof(highDrives) / sizeof(highDrives[0]); i++){
    pinMode(highDrives[i], OUTPUT); //iterate through each of the pins of each of the low-pitched drives and set them as outputs
    digitalWrite(highDrives[i], LOW); //make sure that no drives are clicking
  }
  resetDrives(); //reset all of the hard drives (make them click to confirm that they are working)
}


void loop() {
  recvWithEndMarker();
  newData = false; //receive data from the serial port and tell the receive function that ther is no new data
}

void resetDrives(){
  for(int i = 0; i < sizeof(lowDrives) / sizeof(lowDrives[0]); i++){
    digitalWrite(lowDrives[i], HIGH);
    delay(3);
    digitalWrite(lowDrives[i], LOW); //click all of the low-pitched drives
  }
  for(int i = 0; i < sizeof(highDrives) / sizeof(highDrives[0]); i++){
    digitalWrite(highDrives[i], HIGH);
    delay(3);
    digitalWrite(highDrives[i], LOW); //click all of the high-pitched drives
  }
}

void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;

    while(Serial.available() > 0 && newData == false) {
        rc = Serial.read();


        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0';
            ndx = 0;
            newData = true;
            parsed = atol(receivedChars);
            //Serial.println(parsed);
            if(parsed == 1024){
              resetDrives(); //if we get a command to reset the drives, do it
            }
            else{ //otherwise, we're getting a note command
              noteInfo = parsed;
              note = noteInfo / 1000;
              velocity = noteInfo - (note*1000);
              clickDelay = map(velocity, 0, 127, 1500, 2000); //map the 0-127 MIDI velocity to the 1653-3000 microseconds clickDelay
              if(velocity == 0){
                clickDelay = 0; //if the velocity is 0, we dan't want a click at all, so set clickDelay to 0
              }
            }
            for(int i = 0; i < sizeof(lowDrums) / sizeof(lowDrums[0]); i++){
              if(note == lowDrums[i]){
                clickDrives(0); //if the drum is low-pitched, click all of the low-pitched drives
              }
            }
            for(int i = 0; i < sizeof(highDrums) / sizeof(highDrums[0]); i++){
              if(note == highDrums[i]){
                clickDrives(1); //if the drum is high-pitched, click all of the high-pitched drives
              }
            }
        }
    }
}

void clickDrives(bool driveSet){
  if(driveSet == 0){
    for(int i = 0; i < sizeof(lowDrives) / sizeof(lowDrives[0]); i++){
      digitalWrite(lowDrives[i], HIGH);
      delayMicroseconds(clickDelay);
      digitalWrite(lowDrives[i], LOW); //click all of the low-pitched drives
    }
  }
  if(driveSet == 1){
    for(int i = 0; i < sizeof(highDrives) / sizeof(highDrives[0]); i++){
      digitalWrite(highDrives[i], HIGH);
      delayMicroseconds(clickDelay);
      digitalWrite(highDrives[i], LOW); //click all of the high-pitched drives
    }
  }
}
