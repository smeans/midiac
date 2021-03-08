const int dir = 1;
const int headStep = 2; //offsets from drives array for the direction and step pins
const int maxSteps = 80; //the maximum number of times the heads can step before hitting the end of their range
float pulseStart = 0; //the time at which a period of a note starts

const unsigned long note_periods[] = { //all MIDI note periods in microseconds from note 0 to note 127
  122312, 115447, 108968, 102852, 97079, 91631, 86488, 81634, 77052, 72727, 68645, 64793, 61156, 57724, 54484, 51426, 48540, 45815, 43244, 40817, 38526, 36364, 34323, 32396, 30578, 28862, 27242, 25713, 24270, 22908, 21622, 20408, 19263, 18182, 17161, 16198, 15289, 14431, 13621, 12856, 12135, 11454, 10811, 10204, 9631, 9091, 8581, 8099, 7645, 7215, 6810, 6428, 6067, 5727, 5405, 5102, 4816, 4545, 4290, 4050, 3822, 3608, 3405, 3214, 3034, 2863, 2703, 2551, 2408, 2273, 2145, 2025, 1911, 1804, 1703, 1607, 1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012, 956, 902, 851, 804, 758, 716, 676, 638, 602, 568, 536, 506, 478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253, 239, 225, 213, 201, 190, 179, 169, 159, 150, 142, 134, 127, 119, 113, 106, 100, 95, 89, 84, 80
};

int drives[] = {22, 25, 28, 31, 34, 37}; //the first pin where each of the drives is connected (add another drive by putting a 40 in the array)

#define floppyCount sizeof(drives) / sizeof(drives[0]) //the number of floppies connected minus 1

int drivesPlaying = 0; //the number of drives currently playing
long int pitchWheel; //a variable to hold the setting of the pitchwheel
long int notePeriod; //the period of the note that is currently playing (adjusted to account for the pitchwheel)
int floppyPos[floppyCount]; //an array to store the position of each floppy drive
bool floppyDir[floppyCount]; //an array to hold the current direction of each drive
int note = 0; //the MIDI note that we are playing
int velocity = 0; //the MIDI velocity of our note
long int noteInfo; //the raw note and velocity information (note 83 velocity 127 would be stored as 083127)



long unsigned int parsed = 0;
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false; //variables for the receive serial data code
//22 -> drive enable
//23 -> direction
//24 -> step
//etc for all of the other drives

void setup() {
  Serial.begin(115200); //duh
  Serial.setTimeout(5);
  for(int i = 0; i < sizeof(drives) / sizeof(drives[0]); i++){
    for(int j = 0; j < 3; j++){
      pinMode(drives[i] + j, OUTPUT); //iterate through each of the pins of each of the drives and set them as outputs
    }
    digitalWrite(drives[i], HIGH); //set all drive enable pins high to disable the LEDs on all drives
  }
  pulseStart = micros(); //set the start of the first note to the current time
  resetDrives(); //reset all of the drives to their starting positions
}


void loop() {
  recvWithEndMarker();
  newData = false; //receive data from the serial port and tell the receive function that there is no new data to receive



  if(notePeriod <= micros() - pulseStart){
    pulseDrives();
    pulseStart = micros(); //if 1 period has elapsed between now and last time we pulsed the drives, pulse them again and reset the period time
  }
}




void pulseDrives(){
  for(int i = 0; i < drivesPlaying; i++){
    if(floppyPos[i] == maxSteps){
      floppyDir[i] = !floppyDir[i]; //if a drive that is currently playing has reached the end of its range, reverse its direction in the direction array
      digitalWrite(drives[i] + dir, floppyDir[i]); //write the direction to the drive
      floppyPos[i] = 0; //reset the position to 0
    }
    digitalWrite(drives[i] + headStep, HIGH); //step the heads
    digitalWrite(drives[i] + headStep, LOW); //step the heads
    floppyPos[i] += 1; //increment the position of the heads

  }
}


void resetDrives(){
  for(int i = 0; i < floppyCount; i++){ //iterate through each drive
    digitalWrite(drives[i], LOW); //turn on activity light
    digitalWrite(drives[i] + dir, HIGH); //set the direction to backwards (towards the back of the drive)
    for(int j = 0; j < maxSteps; j++){ //step the head backwards maxSteps times so that it is reset to its starting position
      delay(5);
      digitalWrite(drives[i] + headStep, HIGH); //step the heads
      digitalWrite(drives[i] + headStep, LOW); //step the heads
    }
    floppyPos[i] = 0;
    floppyDir[i] = 0; //reset the position and direction of each drive to 0
    digitalWrite(drives[i] + dir, LOW); //set the direction of all drives to forward
    digitalWrite(drives[i], HIGH); //turn off activity light
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
           if(parsed / 100000 == 123){
              pitchWheel = parsed - 12300000;
              notePeriod = map(pitchWheel, 0, 16384, note_periods[note - 1], note_periods[note + 1]); //if the serial data starts with 123, it's a pitchwheel command, so receive it and map it to a period value
            }
            else if(parsed == 1024){
              resetDrives(); //if we get a command to reset the drives, do it
            }
            else{ //otherwise, we're getting a note command
              pitchWheel = 8192; //set the pitchwheel to its default value of 8192
              noteInfo = parsed;
              note = noteInfo / 1000;
              velocity = noteInfo - (note*1000);
              drivesPlaying = map(velocity, 0, 127, 0, floppyCount); //map the 0-127 MIDI velocity to the 0-floppyCount drivesPlaying
              notePeriod = map(pitchWheel, 0, 16384, note_periods[note - 1], note_periods[note + 1]); //take the pitchwheel value into account when determining the period
              pulseStart = micros(); //set the start of the current note to right now
            }
            for(int i = floppyCount - 1; i >= drivesPlaying; i--){
              digitalWrite(drives[i], HIGH); //turn off the LEDs on any drives that aren't playing
            }
            for(int i = 0; i < drivesPlaying; i++){
              digitalWrite(drives[i], LOW); //make sure that the LEDs on all playing drives are on
            }
        }
    }
}
