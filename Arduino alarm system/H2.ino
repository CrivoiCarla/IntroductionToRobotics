
const int PIN_RED = 5;
const int PIN_GREEN = 6;
const int PIN_BLUE = 9;
const int PIN_BUZZER = 3;
const int PIN_TRIG = 10;
const int PIN_ECHO = 11;
const int PIN_BUTTON_1 = 2;
const int PIN_BUTTON_2 = 4;
const int PIN_BUTTON_3 = 7;

long duration;
int distance;

int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;

// Variabile pentru debouncing
int lastButtonState1 = LOW;
int lastButtonState2 = LOW;
int lastButtonState3 = LOW;
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime3 = 0;
unsigned long debounceDelay = 25;

// Vector pentru a stoca ordinea butoanelor
char desiredButtonOrder[] = { '1', '2', '3', '2', '3' };
int currentButtonIndex = 0;

int button1PressCount = 0;
int button2PressCount = 0;
int button3PressCount = 0;

unsigned long combinationStartTime = 0;
unsigned long maxTimeInterval = 15000;

bool alarmDetect = false;
bool wrongCode = false;

void setup() {
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);

  pinMode(PIN_BUZZER, OUTPUT);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  pinMode(PIN_BUTTON_1, INPUT);
  pinMode(PIN_BUTTON_2, INPUT);
  pinMode(PIN_BUTTON_3, INPUT);

  Serial.begin(9600);
}

void loop() {
  distance = calculateDistance();
  Serial.println(distance);
  if (distance < 50 && distance > 0 and alarmDetect == false and wrongCode == false) {
    alarmDetect = true;
  }

  if (alarmDetect == true && wrongCode == false) {
    alarmDetectActive();
  }

  if (wrongCode) {
    alarmDetect = false;
    alarmWrongCode();
  }

  button1Pressed();
  button2Pressed();
  button3Pressed();

  if (currentButtonIndex == 0) {
    combinationStartTime = millis();
  }

  if (millis() - combinationStartTime >= maxTimeInterval && alarmDetect == true) {
    Serial.println("Combinație greșită! A trecut prea mult timp.");
    wrongCode = true;
    currentButtonIndex = 0;
  }
}

void checkButtonCombination(char buttonPressed) {
  if (buttonPressed == desiredButtonOrder[currentButtonIndex]) {
    currentButtonIndex++;
    if (currentButtonIndex == sizeof(desiredButtonOrder) && (alarmDetect == true or wrongCode == true)) {
      Serial.println("Ai reușit combinatia!");
      stopAlarm();
      currentButtonIndex = 0;
    } else if (currentButtonIndex == sizeof(desiredButtonOrder)) {
      currentButtonIndex = 0;
    }
  } else {
    Serial.println("Combinație greșită!");
    // alarmWrongCode();
    if (alarmDetect) {
      wrongCode = true;
    }
    currentButtonIndex = 0;
  }
}

void button1Pressed() {

  int readingButton1 = digitalRead(PIN_BUTTON_1);
  if (readingButton1 != lastButtonState1) {
    lastDebounceTime1 = millis();
  }
  if ((millis() - lastDebounceTime1) > debounceDelay) {
    if (readingButton1 != buttonState1) {
      buttonState1 = readingButton1;
      if (buttonState1 == HIGH) {
        Serial.println("Button 1 pressed");
        button1PressCount++;
        checkButtonCombination('1');
      }
    }
  }
  lastButtonState1 = readingButton1;
}

void button2Pressed() {
  int readingButton2 = digitalRead(PIN_BUTTON_2);
  if (readingButton2 != lastButtonState2) {
    lastDebounceTime2 = millis();
  }
  if ((millis() - lastDebounceTime2) > debounceDelay) {
    if (readingButton2 != buttonState2) {
      buttonState2 = readingButton2;
      if (buttonState2 == HIGH) {
        Serial.println("Button 2 pressed");
        button2PressCount++;
        checkButtonCombination('2');
      }
    }
  }
  lastButtonState2 = readingButton2;
}

void button3Pressed() {
  int readingButton3 = digitalRead(PIN_BUTTON_3);
  if (readingButton3 != lastButtonState3) {
    lastDebounceTime3 = millis();
  }
  if ((millis() - lastDebounceTime3) > debounceDelay) {
    if (readingButton3 != buttonState3) {
      buttonState3 = readingButton3;
      if (buttonState3 == HIGH) {
        Serial.println("Button 3 pressed");
        button3PressCount++;
        checkButtonCombination('3');
      }
    }
  }
  lastButtonState3 = readingButton3;
}

int calculateDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  duration = pulseIn(PIN_ECHO, HIGH);
  distance = duration * 0.034 / 2;
  return distance;
}

void alarmDetectActive() {
  static unsigned long startTime = 0;
  static int step = 0;
  unsigned long currentTime = millis();
  static unsigned long redDuration = 500;
  static unsigned long buzzer1Frequency = 300;
  static unsigned long buzzer2Frequency = 500;

  if (step == 0) {
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 255);
    tone(PIN_BUZZER, buzzer1Frequency);
    startTime = currentTime;
    step = 1;
  } else if (step == 1 && currentTime - startTime >= redDuration) {
    tone(PIN_BUZZER, buzzer2Frequency);
    startTime = currentTime;
    step = 2;
  } else if (step == 2 && currentTime - startTime >= redDuration) {
    noTone(PIN_BUZZER);
    startTime = currentTime;
    step = 0;
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 0);
  }
}


void alarmWrongCode() {
  static unsigned long startTime = 0;
  static int step = 0;
  unsigned long currentTime = millis();
  static int buzzerFrequency = 600;
  static unsigned long whiteDuration = 150;
  static unsigned long redBlueDuration = 1000;

  if (step == 0) {
    analogWrite(PIN_RED, 255);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 0);
    for (int i = buzzerFrequency; i < (buzzerFrequency + 100); i++) {
      tone(PIN_BUZZER, i);
    }
    startTime = currentTime;
    step = 1;
  } else if (step == 1 && currentTime - startTime >= redBlueDuration) {
    analogWrite(PIN_RED, 255);
    analogWrite(PIN_GREEN, 255);
    analogWrite(PIN_BLUE, 255);
    step = 2;
  } else if (step == 2 && currentTime - startTime >= whiteDuration) {
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 255);
    for (int i = buzzerFrequency + 100; i > buzzerFrequency; i--) {
      tone(PIN_BUZZER, i);
    }
    startTime = currentTime;
    step = 3;
  } else if (step == 3 && currentTime - startTime >= redBlueDuration) {
    analogWrite(PIN_RED, 255);
    analogWrite(PIN_GREEN, 255);
    analogWrite(PIN_BLUE, 255);
    step = 0;
  }
}


void stopAlarm() {
  wrongCode = false;
  alarmDetect = false;

  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 255);
  analogWrite(PIN_BLUE, 0);
  noTone(PIN_BUZZER);
  delay(2000);
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 0);
}
