const int pinA = 12;
const int pinB = 10;
const int pinC = 9;
const int pinD = 8;
const int pinE = 7;
const int pinF = 6;
const int pinG = 5;
const int pinDP = 4;
const int ledPin = 13;  

const int segmentPins[] = {pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP}; 
bool segmentState[8] = {false, false, false, false, false, false, false, false}; 
String currentSegment = "dp";  

const int joyX = A0;  
const int joyY = A1;  
const int joyButton = 2;  

unsigned long lastBlinkTime = 0; 
const long blinkInterval = 500; 
unsigned long lastButtonPress = 0;  
const long debounceTime = 500;  

unsigned long lastJoystickMove = 0;  
const long joystickDebounceTime = 500;  


unsigned long buttonPressStartTime = 0;  
const long longPressDuration = 2000;  // 2 seconds for a long press
bool buttonIsBeingPressed = false;  // Track if the button is currently being pressed

const int noOfDigits = 10; 
const int segSize = 8;    

// global var for blinkLED
bool ledState = HIGH;
unsigned long previousLedMillis = 0;
const long ledOnTime = 200; 
const long ledOffTime = 200; 
int blinkCount = 0; 
const long ledEndOnTime = 2000; // time the LED stays on after blinking

bool shouldBlinkCurrentSegment = false; 

byte digitMatrix[noOfDigits+1][segSize - 1 ] = {
  // a b c d e f g

  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}, // 9
  {0, 0, 0, 0, 0, 0, 0}  // only dp

};

void setup() {
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], LOW);  
  }
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  pinMode(joyButton, INPUT_PULLUP);  

  Serial.begin(9600);
}

void loop() {
  String movement = getJoystickMovement();

  if (movement != "NONE" && millis() - lastJoystickMove > joystickDebounceTime) {
    lastJoystickMove = millis();
    currentSegment = getNextSegment(currentSegment, movement);
  }
  
  if (isButtonPressed) handleButtonPress();

  if (millis() - lastBlinkTime > blinkInterval) {
    shouldBlinkCurrentSegment = !shouldBlinkCurrentSegment;
    lastBlinkTime = millis();
  }

  updateDisplay();
  checkSegmentStateAndBlink();
 
}

bool isButtonPressed() {
    static unsigned long lastDebounceTime = 0;  
    const unsigned long debounceDelay = 15;     
    static int buttonState = HIGH;              
    int reading = digitalRead(joyButton);       

    
    if (reading != buttonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading == LOW) {
            buttonState = reading;
            return true;
        }
    }
    buttonState = reading;
    return false;
}


void handleButtonPress() {
  if (digitalRead(joyButton) == LOW) {
    if (!buttonIsBeingPressed) {  
        buttonIsBeingPressed = true;
        buttonPressStartTime = millis();
    } else if (millis() - buttonPressStartTime > longPressDuration) {  
        resetDisplay();
        buttonIsBeingPressed = false;  
    }
  } else if (buttonIsBeingPressed && millis() - buttonPressStartTime < longPressDuration) {
    if (millis() - lastButtonPress > debounceTime) {
        toggleSegment(currentSegment);
        lastButtonPress = millis();
    }
    buttonIsBeingPressed = false;  
  }
}

String getJoystickMovement() {
  int xVal = analogRead(joyX);
  int yVal = analogRead(joyY);
 
  // I changed the values ​​to have a natural movement depending on how I hold the joystick
  if (yVal > 950) return "DOWN";
  if (yVal < 250) return  "UP";
  if (xVal > 950) return "RIGHT";
  if (xVal < 250) return "LEFT";
  return "NONE";
}

String getNextSegment(String current, String movement) {

  if (current == "a") {
    if (movement == "DOWN") return "g";
    if (movement == "LEFT") return "f";
    if (movement == "RIGHT") return "b";
  } else if (current == "b") {
    if (movement == "UP") return "a";
    if (movement == "DOWN") return "g";
    if (movement == "LEFT") return "f";
  } else if (current == "c") {
    if (movement == "UP") return "g";
    if (movement == "DOWN") return "d";
    if (movement == "LEFT") return "e";
    if (movement == "RIGHT") return "dp";
  } else if (current == "d") {
    if (movement == "UP") return "g";
    if (movement == "LEFT") return "e";
    if (movement == "RIGHT") return "c";
  } else if (current == "e") {
    if (movement == "UP") return "g";
    if (movement == "DOWN") return "d";
    if (movement == "RIGHT") return "c";
  } else if (current == "f") {
    if (movement == "UP") return "a";
    if (movement == "DOWN") return "g";
    if (movement == "RIGHT") return "b";
  } else if (current == "g") {
    if (movement == "UP") return "a";
    if (movement == "DOWN") return "d";
  } else if (current == "dp") {
    if (movement == "LEFT") return "c";
  }
  
  return current; 
}

void toggleSegment(String segment) {
  int index = getSegmentIndex(segment);
  Serial.print(segmentState[index]);
  segmentState[index] = !segmentState[index];  
}

int getSegmentIndex(String segment) {
  if (segment == "a") return 0;
  if (segment == "b") return 1;
  if (segment == "c") return 2;
  if (segment == "d") return 3;
  if (segment == "e") return 4;
  if (segment == "f") return 5;
  if (segment == "g") return 6;
  if (segment == "dp") return 7;
}

void updateDisplay() {
 
  int index = getSegmentIndex(currentSegment);
  for (int i = 0; i < 8; i++) {
    if(i == index && shouldBlinkCurrentSegment) {
      digitalWrite(segmentPins[i], !segmentState[i]); 
    } else {
      digitalWrite(segmentPins[i], segmentState[i]);
    }
  }
}

void resetDisplay() {
  for (int i = 0; i < 8; i++) {
      segmentState[i] = false;  
  }
  currentSegment = "dp";  
}

void checkSegmentStateAndBlink() {
  for (int digit = 0; digit < noOfDigits; digit++) {  
    bool match = true;  
    for (int segment = 0; segment < segSize - 1; segment++) {  
      if (segmentState[segment] != bool(digitMatrix[digit][segment])) {
        match = false;  
        break;  
      }
    }
    if (match) {
      blinkLED(digit+1 );
      break;  
    }
  }
}



void blinkLED(int numberOfBlinks) {
  unsigned long currentMillis = millis();

  if (blinkCount < numberOfBlinks * 2) {
    if ((ledState == HIGH) && (currentMillis - previousLedMillis >= ledOnTime)) {
      ledState = LOW; 
      previousLedMillis = currentMillis; 
      digitalWrite(ledPin, ledState); 
      blinkCount++; 
    } 
    else if ((ledState == LOW) && (currentMillis - previousLedMillis >= ledOffTime)) {
      ledState = HIGH; 
      previousLedMillis = currentMillis; 
      digitalWrite(ledPin, ledState); 
      blinkCount++; 
    }
  } else if (blinkCount >= numberOfBlinks * 2 && currentMillis - previousLedMillis < ledEndOnTime) {
    ledState = HIGH; 
    digitalWrite(ledPin, ledState);
  } else if (currentMillis - previousLedMillis >= ledEndOnTime) {
    ledState = LOW; 
    digitalWrite(ledPin, ledState);
    blinkCount = 0; 
  }
}



