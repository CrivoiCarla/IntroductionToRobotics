const int latchPin = 11;
const int clockPin = 10;
const int dataPin = 12;
const int segD1 = 4;
const int segD2 = 5;
const int segD3 = 6;
const int segD4 = 7;
const int StartPauseButtonPin = 2;
const int LapButtonPin = 9;
const int ResetButtonPin = 8;

int displayDigits[] = {segD1, segD2, segD3, segD4};

const int encodingsNumber = 16;

byte byteEncodings[encodingsNumber] = {
  //A B C D E F G DP
    B11111100, // 0
    B01100000, // 1
    B11011010, // 2
    B11110010, // 3
    B01100110, // 4
    B10110110, // 5
    B10111110, // 6
    B11100000, // 7
    B11111110, // 8
    B11110110, // 9
};
unsigned long lastIncrement = 0;
unsigned long delayCount = 100; 
unsigned long number = 0;
unsigned long displayCount = 4;

bool stopwatchRunning = true;
bool lastStartPauseButtonState = HIGH;
bool lastLapButtonState = HIGH;
bool lastResetButtonState = HIGH;

unsigned long lapTimes[4] = {0};
int lapIndex = 0;
bool isViewingLaps = false;
int viewLapIndex = -1; 
bool resetButtonPreviouslyPressed = false;
bool viewLapMode = false; 


unsigned long lastDebounceTime = 0;  
unsigned long debounceDelay = 50;   


void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }

  pinMode(StartPauseButtonPin, INPUT_PULLUP);
  pinMode(LapButtonPin, INPUT_PULLUP);
  pinMode(ResetButtonPin, INPUT_PULLUP);

  Serial.begin(9600);
  writeNumber(0); // Initialize display with 0
}


void loop() {
  bool currentStartPauseButtonState = digitalRead(StartPauseButtonPin);
  bool currentLapButtonState = digitalRead(LapButtonPin);
  bool currentResetButtonState = digitalRead(ResetButtonPin);

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Start/Pause button logic
    if (currentStartPauseButtonState != lastStartPauseButtonState) {
      if (currentStartPauseButtonState == LOW) { 
        stopwatchRunning = !stopwatchRunning;
        Serial.println(stopwatchRunning ? "Stopwatch started" : "Stopwatch paused");
        if (!stopwatchRunning) {
          viewLapMode = false; // Exit lap view mode on pause
        }
        lastDebounceTime = millis(); 
      }
    }

    // Lap button logic
    if (currentLapButtonState != lastLapButtonState) {
      if (currentLapButtonState == LOW) { 
        if (stopwatchRunning) {
          lapTimes[lapIndex % 4] = number; 
          lapIndex = (lapIndex + 1) % 4; 
          Serial.print("Lap "); Serial.print(lapIndex); Serial.print(": "); Serial.println(number);
        } else {
          // View lap logic
          viewLapIndex = (viewLapIndex + 1) % 4;
          Serial.print("Viewing lap "); Serial.print(viewLapIndex + 1); Serial.print(": "); Serial.println(lapTimes[viewLapIndex]);
          viewLapMode = true; // Enter view lap mode
        }
        lastDebounceTime = millis(); 
      }
    }

    // Reset button logic
    if (currentResetButtonState != lastResetButtonState) {
      if (currentResetButtonState == LOW) { 
        if(!stopwatchRunning && viewLapMode){
          memset(lapTimes, 0, sizeof(lapTimes)); 
          number = 0; 
          lapIndex = 0; 
          viewLapIndex = -1; 
          viewLapMode = false; 
          Serial.println("Stopwatch and lap times reset");
        } else if (!stopwatchRunning) {
          number = 0; 
          Serial.println("Reset to 0");
        }
        lastDebounceTime = millis(); 
      }
    }
  }

 
  if (stopwatchRunning && (millis() - lastIncrement) >= delayCount) {
    lastIncrement = millis();
    number++; 
    if (number > 9999) number = 0; 
  }

  if (!stopwatchRunning && viewLapMode) {
    writeNumber(lapTimes[viewLapIndex]); 
  } else if (!viewLapMode) {
    writeNumber(number); 
  }

  lastStartPauseButtonState = currentStartPauseButtonState;
  lastLapButtonState = currentLapButtonState;
  lastResetButtonState = currentResetButtonState;

  delay(10); 
}


void writeReg(int digit) {
  
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, digit);
  digitalWrite(latchPin, HIGH);
}

void activateDisplay(int displayNumber) {
 
  for (int i = 0; i < displayCount; i++) {
  	digitalWrite(displayDigits[i], HIGH);
  }
  digitalWrite(displayDigits[displayNumber], LOW);
}


void writeNumber(unsigned long number) {
  bool numberOverNine = number >= 0; 

  for (int digitPosition = 0; digitPosition < displayCount; digitPosition++) {

    int digitValue = (number / (int)pow(10, digitPosition)) % 10;
    byte segmentsToDisplay = byteEncodings[digitValue];

    //Turn on DP
    if (numberOverNine && digitPosition == (displayCount - 3)) {
      segmentsToDisplay |= B00000001; 
    }

    activateDisplay(displayCount - 1 - digitPosition);
    writeReg(segmentsToDisplay); 

    delay(5); 

    for (int i = 0; i < displayCount; i++) {
      digitalWrite(displayDigits[i], HIGH);
    }
  }
}

