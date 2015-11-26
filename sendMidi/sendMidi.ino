#define MIDICHANNEL 1;
//define globals
int SENSORVALUES[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte MIDINOTES[8] = {62, 64, 66, 67, 69, 71, 73, 74};
bool ACTIVENOTES[8] = {0, 0, 0, 0, 0, 0, 0, 0};
bool ALREADYACTIVE[8] = {0, 0, 0, 0, 0, 0, 0, 0};
bool NOTEPLAYED[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int notesPlayed = 0;
int noteLengths[6] = {0, 0, 0, 0, 0, 0};
int minNoteLength = 150;
int halfNoteLength = 0;
int pattern[24][3] = {{62, 52, 55},
                     {59, 62, 66},
                     {55, 59, 62},
                     {57, 61, 64},
                     {62, 52, 55},
                     {59, 62, 66},
                     {55, 59, 62},
                     {59, 0, 0},
                     {52, 0, 0},
                     {59, 62, 66},
                     {55, 59, 62},
                     {57, 61, 64},
                     {54, 0, 0},
                     {52, 0, 0},
                     {52, 0, 0},
                     {52, 0, 0},
                     {50, 0, 0},
                     {52, 0, 0},
                     {54, 0, 0},
                     {57, 0, 0},
                     {59, 0, 0},
                     {59, 0, 0},
                     {59, 0, 0},
                     {59, 0, 0}
                    };
bool patternStarted = false;
bool noteStarted = false;
int currPatternIndex = 0;
int currPatternLen = 0;

byte noteOn = 144;
byte noteOff = 128;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
}

void transmitPattern() {
  if(currPatternLen == 0 && 
    ((currPatternIndex < 21 && currPatternIndex > 15) || currPatternIndex < 13)) {
    for(int i = 0; i < 3; i++) {
      Serial.write(noteOn);
      int lastIndex;
      if(currPatternIndex == 0) lastIndex = 3;
      else lastIndex = currPatternIndex - 1;
      Serial.write(pattern[lastIndex][i]);
//      Serial.print(currPatternIndex);
//      if(i == 2) Serial.println(pattern[currPatternIndex][i]);
//      else { Serial.print(pattern[currPatternIndex][i]); Serial.print(" "); }
      Serial.write(0);
    }
    for(int i = 0; i < 3; i++) {
      //Don't play if it's 0
      if(pattern[currPatternIndex][i]) {
        Serial.write(noteOn);
        Serial.write(pattern[currPatternIndex][i]);
//      Serial.print(currPatternIndex);
//      if(i == 2) Serial.println(pattern[currPatternIndex][i]);
//      else { Serial.print(pattern[currPatternIndex][i]); Serial.print(" "); }
        Serial.write(70);
      }
    }
  }
  if(currPatternLen == (halfNoteLength - 1)) {
    currPatternIndex = (currPatternIndex + 1) % 24;
  }
  currPatternLen = (currPatternLen + 1) % halfNoteLength;
}

void transmitNote() {
  for(int i = 0; i < 8; i++) {
    if(!ALREADYACTIVE[i]) { // if note is already active
                            // we will not start another note
                            // because it will sound like shit
      Serial.write(noteOn);
      Serial.write(MIDINOTES[i]);
      if(ACTIVENOTES[i]) { 
        Serial.write(100);
      }
      else
        Serial.write(0);
    }
    // avoid sending over hella signals
    if(NOTEPLAYED[i] && !ALREADYACTIVE[i])
      ALREADYACTIVE[i] = true;
    // the note will no longer be active if activenotes is false
    if(!ACTIVENOTES[i] && ALREADYACTIVE[i]) {
      ALREADYACTIVE[i] = 0;
      NOTEPLAYED[i] = 0;
    }
  }
}

int getHalfNoteLength() {
  int length = 0;
  for(int m = 0; m < 6; m++) {
    noteLengths[m] = noteLengths[m] / 10;
  }
  int counts[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  for(int i = 0; i < 6; i++) {
    bool matched = false;
    for(int j = 0; j < i; j++) {
      // if this is already in array, match and increment
      if((noteLengths[i] - counts[j]) >= -2 && 
         (noteLengths[i] - counts[j]) <= 2) {
        counts[j + 6]++;
        matched = true;
      }
      if(!matched) {
        counts[i] = noteLengths[i];
        counts[i + 6] ++;
      }
    }
  }
  int modeIdx = 6;
  int mode;
  for(int k = 7; k < 12; k++) {
     if(counts[k] > modeIdx) modeIdx = counts[k];
  }
  if(counts[modeIdx] > 1) {
    return counts[modeIdx-6] * 20;
  }
  mode = counts[0];
  for(int l = 1; l < 6; l++) {
    if(counts[l] > mode) mode = counts[l];
  }
  return mode*20;
}

void loop() {
  // put your main code here, to run repeatedly:
  SENSORVALUES[0] = analogRead(A0);
  SENSORVALUES[1] = analogRead(A1);
  SENSORVALUES[2] = analogRead(A2);
  SENSORVALUES[3] = analogRead(A3);
  SENSORVALUES[4] = analogRead(A4);
  
  ACTIVENOTES[0] = SENSORVALUES[0] < 280;
  ACTIVENOTES[1] = SENSORVALUES[1] < 280;
  ACTIVENOTES[2] = SENSORVALUES[2] < 300;
  ACTIVENOTES[3] = SENSORVALUES[3] < 200;
  ACTIVENOTES[4] = SENSORVALUES[4] < 250;
  for(int i = 0; i < 5; i++) {
    if(ACTIVENOTES[i] && !NOTEPLAYED[i]) {
      NOTEPLAYED[i] = true;
      notesPlayed ++;
      // if note is not marked as started
      if(!noteStarted && notesPlayed <= 7) {
        noteStarted = true;
      }
    }
  }
  // keep track of how long note has been being played
  if(noteStarted && notesPlayed <= 6) noteLengths[notesPlayed-1] ++;
//  Serial.println(ACTIVENOTES[0]);
  // Serial.println(SENSORVALUES[0]);
  // Serial.println(notesPlayed);
//  if(notesPlayed > 0) 
  // Serial.println(noteLengths[notesPlayed - 1]);
  transmitNote();
  // if sufficient notes have been played
  if(notesPlayed > 6 && !patternStarted) {
    patternStarted = true;
    halfNoteLength = getHalfNoteLength();
  }
  // Serial.println(halfNoteLength);
  if(patternStarted) {
    transmitPattern();
  }
}


