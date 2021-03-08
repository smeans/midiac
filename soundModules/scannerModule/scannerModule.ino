const int dirPins[] = {6, 7}; //the direction control pins for each of the scanners
const int stepPins[] = {3, 4}; //the step pins for each of the scanners
const int enablePin = 8; //the enable pin on the A4988 stepper driver
const int maxSteps = 8000; //the maximum number of times the carriage can step before hitting the end of its range (700 for ImageWriter, 500 for StyleWriter II, 8000 for BigBoi scanner, 6500 for LittleBoi scanner, 500 for dot matrix carriage)
float pulseStart[sizeof(stepPins) / sizeof(stepPins[0])]; //the times at which a period of a note starts for each scanner

const unsigned long note_periods[] = { //all MIDI note periods in microseconds from note 0 to note 127
  122312, 115447, 108968, 102852, 97079, 91631, 86488, 81634, 77052, 72727, 68645, 64793, 61156, 57724, 54484, 51426, 48540, 45815, 43244, 40817, 38526, 36364, 34323, 32396, 30578, 28862, 27242, 25713, 24270, 22908, 21622, 20408, 19263, 18182, 17161, 16198, 15289, 14431, 13621, 12856, 12135, 11454, 10811, 10204, 9631, 9091, 8581, 8099, 7645, 7215, 6810, 6428, 6067, 5727, 5405, 5102, 4816, 4545, 4290, 4050, 3822, 3608, 3405, 3214, 3034, 2863, 2703, 2551, 2408, 2273, 2145, 2025, 1911, 1804, 1703, 1607, 1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012, 956, 902, 851, 804, 758, 716, 676, 638, 602, 568, 536, 506, 478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253, 239, 225, 213, 201, 190, 179, 169, 159, 150, 142, 134, 127, 119, 113, 106, 100, 95, 89, 84, 80
};


long int notes[sizeof(stepPins) / sizeof(stepPins[0])]; //the MIDI notes that we are playing
long int velocities[sizeof(stepPins) / sizeof(stepPins[0])]; //the MIDI velocities of our notes
long int pitchWheelInfo; //a temp variable while we're processing the pitchwheel serial data
long int pitchWheelPin; //the scanner that the pitchwheel command is intended for
long int note = 0; //a temp variable to hold the note value while we're working with it
long int velocity = 0; //a temp variable to hold the velocity value while we're working with it
long int noteInfo; //the raw note, velocity, and scanner number information (note 83 velocity 127 scanner 1 would be stored as 0831271)
long int pitchWheel[sizeof(stepPins) / sizeof(stepPins[0])]; //the pitchwheel value for each of the scanners
long int notePeriods[sizeof(stepPins) / sizeof(stepPins[0])]; //the period of each note that is currently playing (adjusted to account for the pitchwheel)
int totalScanners = sizeof(stepPins) / sizeof(stepPins[0]); //the number of total scanners connected
int scannerNum = 0; //which of the pins on the head we're currently trying to control
int scannerPos[sizeof(stepPins) / sizeof(stepPins[0])]; //a variable to store the current position of each scanner
bool scannerDir[sizeof(stepPins) / sizeof(stepPins[0])]; //a variable to store the current direction of each scanner

long unsigned int parsed = 0;
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false; //variables for the receive serial data code

void setup() {
  Serial.begin(115200); //duh
  Serial.setTimeout(5);
  for(int i = 0; i < totalScanners; i++){
    pinMode(dirPins[i], OUTPUT);
    pinMode(stepPins[i], OUTPUT);
    digitalWrite(dirPins[i], scannerDir[i]); //iterate through each scanner, set dir and step as outputs, and set dir to zero for each scanner
  }

  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, LOW); //set the enable pin of the A4988 low

  for(int i = 0; i < totalScanners; i++){
    pulseStart[i] = micros(); //set the pulseStart for each scanner to the current time
  }
}


void loop() {
  recvWithEndMarker();
  newData = false; //receive data from the serial port and tell the receive function that there is no new data to receive

  for(int i = 0; i < totalScanners; i++){
    if(notePeriods[i] <= micros() - pulseStart[i]){
      if(velocities[i] != 0){
        pulseScanner(i);
        //Serial.println(i);

      }
      pulseStart[i] = micros(); //if 1 period has elapsed between now and last time we pulsed the scanner and the velocity is not zero, pulse it again and reset the period time
    }
  }
}


void resetScanners(){
  for(int i = 0; i < totalScanners; i++){
    digitalWrite(dirPins[i], HIGH);
    if(scannerDir[i] == 1){
      for(int j = scannerPos[i]; j <= 8000; j++){
      digitalWrite(stepPins[i], HIGH); //step the scanner
      digitalWrite(stepPins[i], LOW); //step the scanner
      delayMicroseconds(1000);
      }
    }
    else{
      for(int j = scannerPos[i]; j >= 0; j--){
        digitalWrite(stepPins[i], HIGH); //step the scanner
        digitalWrite(stepPins[i], LOW); //step the scanner
        delayMicroseconds(1000);
      }
    }
  }
}

void pulseScanner(int pin){
  if(scannerPos[pin] == maxSteps){
    scannerDir[pin] = !scannerDir[pin]; //if the scanner has reached the end of its range, reverse its direction in the direction variable
    digitalWrite(dirPins[pin], scannerDir[pin]); //write the direction to the scanner motor
    scannerPos[pin] = 0; //reset the position to 0
  }
  digitalWrite(stepPins[pin], HIGH); //step the scanner
  digitalWrite(stepPins[pin], LOW); //step the scanner
  scannerPos[pin] += 1; //increment the position of the scanner
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
            if(parsed / 1000000 == 123){
              pitchWheelInfo = parsed - 123000000;
              pitchWheelPin = pitchWheelInfo % 10;
              pitchWheel[pitchWheelPin] =  pitchWheelInfo / 10; //determine the pitchwheel value and the pin it's intended for
              notePeriods[pitchWheelPin] = map(pitchWheel[pitchWheelPin], 0, 16384, note_periods[note - 1], note_periods[note + 1]); //if we get a pitchwheel command (123 + 0-16384 pitchwheel value + pin number), account for it in the note period. 8192 is no pitchbend
            }
            else if(parsed == 1024){
              resetScanners(); //if we get a command to reset the scanners, do it
            }
            else{ //otherwise, we're getting a note command
              noteInfo = parsed;
              note = noteInfo / 10000;
              velocity = (noteInfo - (note*10000))/10;
              scannerNum = noteInfo - (note*10000) - (velocity*10); //determine the note, velocity, and the particular scanner it's for
              pitchWheel[scannerNum] = 8192; //set the pitchwheel to its normal value of 8192 for the scanner that we're controlling
              notes[scannerNum] = note;
              notePeriods[scannerNum] = map(pitchWheel[scannerNum], 0, 16384, note_periods[note - 1], note_periods[note + 1]); //extract the note and velocity info from the serial data and determine the period of the note
              velocities[scannerNum] = velocity; //write these note and velocity values to their appropritate places in the notes and velocities arrays
              pulseStart[scannerNum] = micros(); //set the start of the current note on the pin that we just changed the note on to the current time

            }

        }
    }
}
