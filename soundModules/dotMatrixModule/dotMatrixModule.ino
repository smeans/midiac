const int headPins[] = {2, 3, 4, 5}; //the pins connected to the print head
float pulseStart[sizeof(headPins) / sizeof(headPins[0])]; //the times at which a period of a note starts for each pin

const unsigned long note_periods[] = { //all MIDI note periods in microseconds from note 0 to note 127
  122312, 115447, 108968, 102852, 97079, 91631, 86488, 81634, 77052, 72727, 68645, 64793, 61156, 57724, 54484, 51426, 48540, 45815, 43244, 40817, 38526, 36364, 34323, 32396, 30578, 28862, 27242, 25713, 24270, 22908, 21622, 20408, 19263, 18182, 17161, 16198, 15289, 14431, 13621, 12856, 12135, 11454, 10811, 10204, 9631, 9091, 8581, 8099, 7645, 7215, 6810, 6428, 6067, 5727, 5405, 5102, 4816, 4545, 4290, 4050, 3822, 3608, 3405, 3214, 3034, 2863, 2703, 2551, 2408, 2273, 2145, 2025, 1911, 1804, 1703, 1607, 1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012, 956, 902, 851, 804, 758, 716, 676, 638, 602, 568, 536, 506, 478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253, 239, 225, 213, 201, 190, 179, 169, 159, 150, 142, 134, 127, 119, 113, 106, 100, 95, 89, 84, 80
};



long int notes[sizeof(headPins) / sizeof(headPins[0])]; //the MIDI notes that we are playing
long int velocities[sizeof(headPins) / sizeof(headPins[0])]; //the MIDI velocities of our notes
long int pitchWheelInfo; //a temp variable while we're processing the pitchwheel serial data
long int pitchWheelPin; //the pin that the pitchwheel command is intended for
long int note = 0; //a temp variable to hold the note value while we're working with it
long int velocity = 0; //a temp variable to hold the velocity value while we're working with it
long int noteInfo; //the raw note, velocity, and pin information (note 83 velocity 127 pin 1 would be stored as 0831271)
long int pitchWheel[sizeof(headPins) / sizeof(headPins[0])]; //the pitchwheel value for each of the head pins
long int notePeriods[sizeof(headPins) / sizeof(headPins[0])]; //the period of each note that is currently playing (adjusted to account for the pitchwheel)
int pinNum = sizeof(headPins) / sizeof(headPins[0]); //the number of total pins connected
int headNum = 0; //which of the pins on the head we're currently trying to control
int clickDelay[sizeof(headPins) / sizeof(headPins[0])]; //how long to hold the pins high before pullling them low again (adjusts velocity)

long unsigned int parsed = 0;
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false; //variables for the serial receive function

void setup() {
  Serial.begin(115200); //duh
  Serial.setTimeout(5);
  for(int i = 0; i < pinNum; i++){
    pinMode(headPins[i], OUTPUT);
    digitalWrite(headPins[i], LOW); //set all head pins os outputs and set them all low
  }

  for(int i = 0; i < pinNum; i++){
    pulseStart[i] = micros(); //set the pulseStert for each pin to the current time
  }
  resetHead(); //click the head pins just for fun
}


void loop() {
  recvWithEndMarker();
  newData = false; //receive data from the serial port and tell the receive function that ther is no new data

  for(int i = 0; i < pinNum; i++){
    if(notePeriods[i] <= micros() - pulseStart[i] + clickDelay[i]){
      if(velocities[i] != 0){
        pulseHead(i);
        //Serial.println(i);

      }
      pulseStart[i] = micros(); //if 1 period has elapsed between now and last time we pulsed the head and the velocity is not zero, pulse it again and reset the period time
    }
  }
}




void pulseHead(int pin){
  digitalWrite(headPins[pin], HIGH);
  delayMicroseconds(clickDelay[pin]);
  digitalWrite(headPins[pin], LOW); //click the pin once
  pulseStart[pin] = micros(); //reset pulseStart for the pin we're clicking to the current time
}



void resetHead(){
  for(int i = 0; i < pinNum; i++){
    digitalWrite(headPins[i], HIGH);
    delay(10);
    digitalWrite(headPins[i], LOW); //click each of the head pins for 10ms
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
              resetHead(); //if we get a command to reset the head, do it
            }
            else if(parsed / 1000000 == 123){
              pitchWheelInfo = parsed - 123000000;
              pitchWheelPin = pitchWheelInfo % 10;
              pitchWheel[pitchWheelPin] =  pitchWheelInfo / 10; //determine the pitchwheel value and the pin it's intended for
              notePeriods[pitchWheelPin] = map(pitchWheel[pitchWheelPin], 0, 16384, note_periods[note - 1], note_periods[note + 1]); //if we get a pitchwheel command (123 + 0-16384 pitchwheel value + pin number), account for it in the note period. 8192 is no pitchbend
            }
            else{ //otherwise, we're getting a note command
              noteInfo = parsed;
              note = noteInfo / 10000;
              velocity = (noteInfo - (note*10000))/10;
              headNum = noteInfo - (note*10000) - (velocity*10); //determine the note, velocity, and the particular pin it's for
              pitchWheel[headNum] = 8192; //set the pitchwheel to its normal value of 8192 for the head that we're controlling
              notes[headNum] = note;
              notePeriods[headNum] = map(pitchWheel[headNum], 0, 16384, note_periods[note - 1], note_periods[note + 1]); //extract the note and velocity info from the serial data and determine the period of the note
              velocities[headNum] = velocity; //write these note and velocity values to their appropritate places in the notes and velocities arrays
              clickDelay[headNum] = map(velocity, 0, 127, 250, 1000); //map the 0-127 MIDI velocity to the 500-1000us click delay
              pulseStart[headNum] = micros(); //set the start of the current note on the pin that we just changed the note on to the current time

            }
        }
    }
}
