const int ledPinFloor1 = 10;
const int ledPinFloor2 = 11;
const int ledPinFloor3 = 12;
const int ledPinOperational = 13;

const int buttonPinFloor1 = 2;
const int buttonPinFloor2 = 3;
const int buttonPinFloor3 = 4;

int currentFloor = 1;
int targetFloor = 1;
unsigned long lastButtonPressTime = 0;
unsigned long lastOperationTime = 0;


const char* ELEVATOR_STATES[] = {"STATIONARY", "DOOR_CLOSING", "MOVING", "DOOR_OPENING"};
int elevatorState = 0;  // 0 corresponds to STATIONARY
bool operationLEDState = true;
unsigned long lastLEDOperationTime = 0;
const int buzzer = 8;

void setup() {
  pinMode(ledPinFloor1, OUTPUT);
  pinMode(ledPinFloor2, OUTPUT);
  pinMode(ledPinFloor3, OUTPUT);
  pinMode(ledPinOperational, OUTPUT);
  
  pinMode(buttonPinFloor1, INPUT_PULLUP);
  pinMode(buttonPinFloor2, INPUT_PULLUP);
  pinMode(buttonPinFloor3, INPUT_PULLUP);


  // Initialize elevator at floor 1
  digitalWrite(ledPinFloor1, HIGH);
  digitalWrite(ledPinOperational, HIGH);

  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);


}
void setFloorLED(int floor) {
  digitalWrite(ledPinFloor1, LOW);
  digitalWrite(ledPinFloor2, LOW);
  digitalWrite(ledPinFloor3, LOW);
  digitalWrite(ledPinOperational, HIGH);
  
  switch (floor) {
    case 1:
      digitalWrite(ledPinFloor1, HIGH);
      break;
    case 2:
      digitalWrite(ledPinFloor2, HIGH);
      break;
    case 3:
      digitalWrite(ledPinFloor3, HIGH);
      break;
  }
}


void loop() {
  handleButtonPress();
  
  if (strcmp(ELEVATOR_STATES[elevatorState], "STATIONARY") == 0) {
    // Nothing to do here
  } 
  else if (strcmp(ELEVATOR_STATES[elevatorState], "DOOR_CLOSING") == 0) {
    if (millis() - lastOperationTime > 2000) {  
      elevatorState = 2;  // MOVING
      lastOperationTime = millis();
    }
  } 
  else if (strcmp(ELEVATOR_STATES[elevatorState], "MOVING") == 0) {
    if (currentFloor != targetFloor) {
      if (millis() - lastOperationTime > 3000) {  
        if (currentFloor < targetFloor) {
          currentFloor++;
        } else {
          currentFloor--;
        }
        setFloorLED(currentFloor);
        lastOperationTime = millis();
      }
      // Blink the operation LED + sound from buzzer
      if (millis() - lastLEDOperationTime > 100) {
        operationLEDState = !operationLEDState;
        digitalWrite(ledPinOperational, operationLEDState ? HIGH : LOW);
        lastLEDOperationTime = millis(); // update the LED operation time
        tone(buzzer, 1000, 100); // Elevator movement sound
      }
    } else {
      tone(buzzer, 1500, 100); // Arrival sound
      elevatorState = 3;  // DOOR_OPENING
      lastOperationTime = millis();
    }
  } 
  
}

void handleButtonPress() {
  // If the elevator is MOVING, ignore button presses 
  if (elevatorState == 2) { 
    return;
  }

  unsigned long currentTime = millis();
  if (currentTime - lastButtonPressTime < 200) {  // debounce interval of 200ms
    return;                                       // previous press are ignored
  }

  if (digitalRead(buttonPinFloor1) == LOW && currentFloor != 1) {
    tone(buzzer, 500, 100); // Door closing sound
    targetFloor = 1;
    elevatorState = 1;  // DOOR_CLOSING
    lastOperationTime = currentTime;
    lastButtonPressTime = currentTime;
    
  } 
  else if (digitalRead(buttonPinFloor2) == LOW && currentFloor != 2) {
    tone(buzzer, 500, 100); // Door closing sound
    targetFloor = 2;
    elevatorState = 1;  // DOOR_CLOSING
    lastOperationTime = currentTime;
    lastButtonPressTime = currentTime;
  }
  else if (digitalRead(buttonPinFloor3) == LOW && currentFloor != 3) {
    tone(buzzer, 500, 100); // Door closing sound
    targetFloor = 3;
    elevatorState = 1;  // DOOR_CLOSING
    lastOperationTime = currentTime;
    lastButtonPressTime = currentTime;
  }
}





//  DELAY 
// void handleButtonPress() {
//   unsigned long currentTime = millis();
//   if (currentTime - lastButtonPressTime < 200) { // debounce interval of 200ms
//     return;
//   }

//   if (digitalRead(buttonPinFloor1) == LOW && currentFloor != 1) {
//     targetFloor = 1;
//     lastButtonPressTime = currentTime;
//   } 
//   else if (digitalRead(buttonPinFloor2) == LOW && currentFloor != 2) {
//     targetFloor = 2;
//     lastButtonPressTime = currentTime;
//   }
//   else if (digitalRead(buttonPinFloor3) == LOW && currentFloor != 3) {
//     targetFloor = 3;
//     lastButtonPressTime = currentTime;
//   }
// }

// void moveElevatorToTargetFloor() {
  
//   tone(buzzer, 500, 100); // Door closing sound
//   delay(2000); // simulate door closing

//   while (currentFloor != targetFloor) {
//     if (currentFloor < targetFloor) {
//       currentFloor++;
//     } else {
//       currentFloor--;
//     }

//     setFloorLED(currentFloor);
//     tone(buzzer, 1000, 100); // Elevator movement sound
//     delay(3000); // simulate elevator movement
//   }
  
//   tone(buzzer, 1500, 100); // Arrival sound
//   delay(2000); // simulate door opening
  
// }


// void loop() {
//   handleButtonPress();
  
//   if (currentFloor != targetFloor) {
//     digitalWrite(ledPinOperational, HIGH);
//     delay(500);
//     digitalWrite(ledPinOperational, LOW);
//     moveElevatorToTargetFloor();
//   }
// }



