
//========================Libraries========================//
#include <MIDI.h>
#include <ResponsiveAnalogRead.h>
MIDI_CREATE_DEFAULT_INSTANCE();

//===============================Pin Definitions===============================//
//========================Modulation========================//
ResponsiveAnalogRead modulationPot(A0, true); // Modulation Pot

//========================Pitch Bend========================//
ResponsiveAnalogRead pitchbendPot(A1, true); // PitchBend Pot

//======================Velocity Control======================//
const int velocityPot = A2; // Velocity Pot

//====================Transposition Buttons====================//
const int  raiseBtn = A3;  // Transpose +
const int  lowerBtn = A4;  // Transpose -
const int  modeBtn = A5;   // Interval = 1 semitone if pushed (else 12)

//=======================Keyboard Matrix=======================//
const int rowPin[] = { 2, 3, 4, 5, 6, 7, 8 }; // Rows are connected to Digital
const int clock = 9;  //SHCP p11 // 74HC595 (columns)
const int latch = 10; //STCP p12 // 74HC595 (columns)
const int data = 11;  //DS p14   // 74HC595 (columns)
//=============================Pin Definitions END=============================//


//===============================Other Integers===============================//
//=========================Transposition=========================//
int transpose = 0;         // counter for the transposition interval
int raiseBtnState = 0;     // current state of the button1
int lastraiseBtnState = 0; // previous state of the button1
int lowerBtnState = 0;     // current state of the button2
int lastlowerBtnState = 0; // previous state of the button2
int modeBtnState = 0;      // current state of the button3
int lastmodeBtnState = 0;  // previous state of the button3
int interval = 12;         // regular mode shift note by 12 Semitones

//==================How Many Keys In The Matrix==================//
uint8_t keyToMidiMap[56];
boolean keyPressed[56];

//=========================Channel Number=========================//
const int channel = 1;

//=====================74HC595 Shift Register=====================//
//==prepared bit vectors instead of shifting bit left everytime==//
int bits[] = { B00000001, B00000010, B00000100, B00001000, B00010000, B00100000, B01000000, B10000000 };

//=====================Shift To Next Column=====================//
void scanColumn(int value) {
  digitalWrite(latch, LOW); // Pulls the chips latch low
  shiftOut(data, clock, MSBFIRST, value); // Shifts out the 8 bits to the shift register
  digitalWrite(latch, HIGH); // Pulls the latch high displaying the data
}
//====================================================================================//


void setup() {
//==============================Keyboard Matrix==============================//
//==============Map Keys To Actual Midi Note Number==============//
//==========(num 41 corresponds to F1 MIDI note)==========//
//=========Group 1=========//
  keyToMidiMap[0] = 36;
  keyToMidiMap[1] = 37;
  keyToMidiMap[2] = 38;
  keyToMidiMap[3] = 39;
  keyToMidiMap[4] = 40;
  keyToMidiMap[5] = 41;
  keyToMidiMap[6] = 42;
//=========Group 2=========//
  keyToMidiMap[0 + 7] = 43;
  keyToMidiMap[1 + 7] = 44;
  keyToMidiMap[2 + 7] = 45;
  keyToMidiMap[3 + 7] = 46;
  keyToMidiMap[4 + 7] = 47;
  keyToMidiMap[5 + 7] = 48;
  keyToMidiMap[6 + 7] = 49;
//=========Group 3=========//
  keyToMidiMap[0 + 14] = 50;
  keyToMidiMap[1 + 14] = 51;
  keyToMidiMap[2 + 14] = 52;
  keyToMidiMap[3 + 14] = 53;
  keyToMidiMap[4 + 14] = 54;
  keyToMidiMap[5 + 14] = 55;
  keyToMidiMap[6 + 14] = 56;
//=========Group 4=========//
  keyToMidiMap[0 + 21] = 57;
  keyToMidiMap[1 + 21] = 58;
  keyToMidiMap[2 + 21] = 59;
  keyToMidiMap[3 + 21] = 60;
  keyToMidiMap[4 + 21] = 61;
  keyToMidiMap[5 + 21] = 62;
  keyToMidiMap[6 + 21] = 63;
//=========Group 5=========//
  keyToMidiMap[0 + 28] = 64;
  keyToMidiMap[1 + 28] = 65;
  keyToMidiMap[2 + 28] = 66;
  keyToMidiMap[3 + 28] = 67;
  keyToMidiMap[4 + 28] = 68;
  keyToMidiMap[5 + 28] = 69;
  keyToMidiMap[6 + 28] = 70;
//=========Group 6=========//
  keyToMidiMap[0 + 35] = 71;
  keyToMidiMap[1 + 35] = 72;
  keyToMidiMap[2 + 35] = 73;
  keyToMidiMap[3 + 35] = 74;
  keyToMidiMap[4 + 35] = 75;
  keyToMidiMap[5 + 35] = 76;
  keyToMidiMap[6 + 35] = 77;
//=========Group 7=========//
  keyToMidiMap[0 + 42] = 78;
  keyToMidiMap[1 + 42] = 79;
  keyToMidiMap[2 + 42] = 80;
  keyToMidiMap[3 + 42] = 81;
  keyToMidiMap[4 + 42] = 82;
  keyToMidiMap[5 + 42] = 83;
  keyToMidiMap[6 + 42] = 84;
//================END of Keyboard Matrix Map================//
 
//================Setup Pins Output/Input Mode================//
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(rowPin[0], INPUT);
  pinMode(rowPin[1], INPUT);
  pinMode(rowPin[2], INPUT);
  pinMode(rowPin[3], INPUT);
  pinMode(rowPin[4], INPUT);
  pinMode(rowPin[5], INPUT);
  pinMode(rowPin[6], INPUT);
  pinMode(raiseBtn, INPUT); // Transpose up
  pinMode(lowerBtn, INPUT); // Transpose down
  pinMode(modeBtn, INPUT);  // Transpose mode

//================MIDI================//
  MIDI.begin();
  delay(100);
}
//====================================================================================//

void loop() {
//========================Pitch Bend========================//
  pitchbendPot.update();
  int mappedPBPot = map(pitchbendPot.getValue(), 0, 1023, 0, 16363);
  if(pitchbendPot.hasChanged()) {
    MIDImessage(0xE0,(0 & 0x7F),(mappedPBPot >> 7) & 0x7F) ;
  }
    delay(20);      
    
//========================Modulation========================//
  modulationPot.update();
  int mappedMPot = map(modulationPot.getValue(), 0, 1023, 0, 127);
  if(modulationPot.hasChanged()) {
  MIDI.sendControlChange(1, mappedMPot, channel);
  }
  delay(20); 
  
//======================Velocity Control======================//
  int velocity = (map(analogRead(velocityPot), 0, 1023, 0, 127));
  
//=======================Keyboard Matrix=======================//
//=====Read Rows & Columns=====//
//==(What notes are played)==//
  for (int i = 0; i < 8; i++) {
    scanColumn(bits[i]);
    for (int t = 0; t < 7; t++) {
      int groupValue = digitalRead(rowPin[t]);
      
//==Process if any combination of keys pressed==//
      if (groupValue != 0 && !keyPressed[8 * t + i]) {
        keyPressed[8 * t + i] = true;
        noteOn(channel, keyToMidiMap[8 * t + i], velocity);
    }
      if (groupValue == 0 && keyPressed[8 * t + i]) {
        keyPressed[8 * t + i] = false;
        noteOff(channel, keyToMidiMap[8 * t + i], velocity);
      }
    }
  }
//=========================Transposition=========================//
//=====================Read Buttons=====================//
raiseBtnState = digitalRead(raiseBtn); // Transpose up
lowerBtnState = digitalRead(lowerBtn); // Transpose down
modeBtnState = digitalRead(modeBtn); // Transpose mode

//=============Reset Transposition Function=============//
// Button 1 and 2 pressed at the same time reset to original tuning
if ((raiseBtnState == HIGH) && (lowerBtnState == HIGH)) {
      transpose = 0;
      delay(1000);
    }

//=====================Transpose Up=====================//
if (raiseBtnState != lastraiseBtnState && transpose < 36) {// Check if the state has changed
    if (raiseBtnState == HIGH) {// If the current state is HIGH then the button went from off to on:
       transpose = transpose + interval;
    delay(50);// Delay a little bit to avoid bouncing
    MIDI.sendControlChange(123,0,channel);
  }
}
  lastraiseBtnState = raiseBtnState;// save the current state as the last state, for next time through the loop
  
//=====================Transpose Down=====================//
if (lowerBtnState != lastlowerBtnState && transpose > - 24) {// Check if the state has changed
    if (lowerBtnState == HIGH) {// If the current state is HIGH then the button went from off to on:
      transpose = transpose - interval;
    } 
   delay(50);// Delay a little bit to avoid bouncing
   MIDI.sendControlChange(123,0,channel);
  }
  
  lastlowerBtnState = lowerBtnState;// Save the current state as the last state, for next time through the loop

//=====================Transpose Mode=====================//
if (modeBtnState != lastmodeBtnState) {// Check if the state has changed
    if (modeBtnState == HIGH) {// If the current state is HIGH then the button went from off to on:
      interval = 1;// Transpose 1 semitone if button is held down
    } else {
      interval = 12;// Transpose 1 octave if button is not pushed
    }
    delay(50);// Delay a little bit to avoid bouncing
  }
  lastmodeBtnState = modeBtnState;// Save the current state as the last state, for next time through the loop
}

//=================================MIDI=================================//
void MIDImessage(int type,int note,int velocity)  {
  Serial.write(type);// Send command byte
  Serial.write(note);// Send data byte #1
  Serial.write(velocity);// Send data byte #2
}

//=================Note ON=================//
void noteOn(int channel, int pitch, int velocity) {
    pitch = pitch  + transpose;
    MIDI.sendNoteOn(pitch, velocity, channel);
}

//=================Note OFF=================//
void noteOff(int channel, int pitch, int velocity) {
    velocity = 0;
    pitch = pitch  + transpose;
    MIDI.sendNoteOff(pitch, velocity, channel);
}
