
const int PIN_RED = 5;
const int PIN_GREEN = 6;
const int PIN_BLUE = 9;
const int PIN_BUZZER = 3;
const int PIN_TRIG = 10;
const int PIN_ECHO = 11;
const int PIN_BUTTON_1 = 2;
const int PIN_BUTTON_2 = 4;
const int PIN_BUTTON_3 = 7;


const int PIN_LED_1 = A3;
const int PIN_LED_2 = A2;
const int PIN_LED_3 = A1;
const int PIN_LED_4 = A0;

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

const int numPatterns = 100;  // Numărul de modele aleatoare din șir
char patterns[numPatterns];   // Vectorul pentru stocarea modelelor
int currentPatternIndex = 0;  // Indexul curent în șirul de modele
bool playerTurn = false;      // Indicator dacă este rândul jucătorului
int currentLevel = 1;         // Nivelul curent


const int initialMaxTime = 15000;  // 10 secunde inițial
int maxTime = initialMaxTime;
unsigned long startTime = 0;  // Variabilă pentru a înregistra momentul de start al fiecărui nivel


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

  pinMode(PIN_LED_1, OUTPUT);
  pinMode(PIN_LED_2, OUTPUT);
  pinMode(PIN_LED_3, OUTPUT);
  pinMode(PIN_LED_4, OUTPUT);


  Serial.begin(9600);

  randomSeed(analogRead(0));  // Inițializează generatorul de numere aleatoare
}


void loop() {
  if (!playerTurn) {
    // Generează un nou model dacă nu este rândul jucătorului
    generateRandomPattern();
    displayPattern();
    playerTurn = true;
  } else {
    // Așteaptă ca jucătorul să reproducă modelul
    waitForPlayerInput();
  }
}

void generateRandomPattern() {

  maxTime -= 500;  // Scad timpul
  if (maxTime < 5000) {
    maxTime = 5000;  // Asigură-te că timpul minim este de 1 secundă
  }

  for (int i = 0; i < currentLevel; i++) {
    int randomValue = random(1, 4);
    Serial.print("random ");
    Serial.println(randomValue);
    switch (randomValue) {
      case 1:
        patterns[i] = '1';
        break;
      case 2:
        patterns[i] = '2';
        break;
      case 3:
        patterns[i] = '3';
        break;
      case 4:
        patterns[i] = '4';
        break;
    }
  }

  Serial.print(currentLevel);
  Serial.println("Model: ");
  for (int i = 0; i < currentLevel; i++) {
    Serial.print(patterns[i]);
    Serial.print(" ");
  }
}

void displayPattern() {
  startTime = millis();  // Înregistrează momentul de start al nivelului

  for (int i = 0; i < currentLevel; i++) {
    // Verifică timpul disponibil la începutul fiecărui nivel
    if (millis() - startTime > maxTime) {
      Serial.println("Timpul a expirat!");
      playFailureSound();  // Redă sunetul de eșec și afișează roșu
      // Aici poți adăuga logica pentru încheierea jocului
      currentLevel = 1;
      currentPatternIndex = 0;
      playerTurn = false;
      maxTime = initialMaxTime;  // Resetarea timpului maxim la valoarea inițială
      return;
    }

    char patternChar = patterns[i];
    switch (patternChar) {
      case '1':
        digitalWrite(PIN_LED_1, HIGH);
        delay(500);
        digitalWrite(PIN_LED_1, LOW);
        delay(500);
        break;
      case '2':
        digitalWrite(PIN_LED_2, HIGH);
        delay(500);
        digitalWrite(PIN_LED_2, LOW);
        delay(500);
        break;
      case '3':
        digitalWrite(PIN_LED_3, HIGH);
        delay(500);
        digitalWrite(PIN_LED_3, LOW);
        delay(500);
        break;
      case '4':
        digitalWrite(PIN_LED_4, HIGH);
        delay(500);
        digitalWrite(PIN_LED_4, LOW);
        delay(500);
        break;
    }
  }
}


void waitForPlayerInput() {
  // Așteaptă ca jucătorul să apese butoanele corecte în funcție de nivelul curent
  for (int i = 0; i < currentLevel; i++) {
    char expectedButton = patterns[i];
    bool correctButtonPressed = false;

    while (!correctButtonPressed) {
      int buttonPressed = checkButtonPressed();
      if (buttonPressed > 0) {
        if (buttonPressed == (expectedButton - '0')) {
          // Jucătorul a apăsat butonul corect
          correctButtonPressed = true;
        } else {
          // Jucătorul a apăsat un buton greșit
          Serial.println("Ai pierdut!");
          playFailureSound();  // Redă sunetul de eșec și afișează roșu
          // Aici poți adăuga logica pentru încheierea jocului
          currentLevel = 1;
          currentPatternIndex = 0;
          playerTurn = false;
          return;  // Ieși din funcție
        }
      }
    }
  }
  playSuccessSound();
  currentLevel++;
  // Jucătorul a apăsat butoanele corecte pentru nivelul curent
  currentPatternIndex = 0;
  playerTurn = false;  // Treci la următorul model
}


int checkButtonPressed() {
  if (button1Pressed()) {
    digitalWrite(PIN_LED_1, HIGH);
    delay(500);
    digitalWrite(PIN_LED_1, LOW);
    delay(500);
    return 1;
  } else if (button2Pressed()) {
    digitalWrite(PIN_LED_2, HIGH);
    delay(500);
    digitalWrite(PIN_LED_2, LOW);
    delay(500);
    return 2;
  } else if (button3Pressed()) {
    digitalWrite(PIN_LED_3, HIGH);
    delay(500);
    digitalWrite(PIN_LED_3, LOW);
    delay(500);
    return 3;
  }
  return 0;
}




bool button1Pressed() {

  int readingButton1 = digitalRead(PIN_BUTTON_1);
  if (readingButton1 != lastButtonState1) {
    lastDebounceTime1 = millis();
  }
  if ((millis() - lastDebounceTime1) > debounceDelay) {
    if (readingButton1 != buttonState1) {
      buttonState1 = readingButton1;
      if (buttonState1 == HIGH) {
        Serial.println("Button 1 pressed");
        return true;
      }
    }
  }
  lastButtonState1 = readingButton1;
  return false;
}

bool button2Pressed() {
  int readingButton2 = digitalRead(PIN_BUTTON_2);
  if (readingButton2 != lastButtonState2) {
    lastDebounceTime2 = millis();
  }
  if ((millis() - lastDebounceTime2) > debounceDelay) {
    if (readingButton2 != buttonState2) {
      buttonState2 = readingButton2;
      if (buttonState2 == HIGH) {
        Serial.println("Button 2 pressed");
        return true;
      }
    }
  }
  lastButtonState2 = readingButton2;
  return false;
}

bool button3Pressed() {
  int readingButton3 = digitalRead(PIN_BUTTON_3);
  if (readingButton3 != lastButtonState3) {
    lastDebounceTime3 = millis();
  }
  if ((millis() - lastDebounceTime3) > debounceDelay) {
    if (readingButton3 != buttonState3) {
      buttonState3 = readingButton3;
      if (buttonState3 == HIGH) {
        Serial.println("Button 3 pressed");
        return true;
      }
    }
  }
  lastButtonState3 = readingButton3;
  return false;
}


void playSuccessSound() {
  tone(PIN_BUZZER, 1000, 500);
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 255);
  analogWrite(PIN_BLUE, 0);
  delay(1000);
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 0);
}

void playFailureSound() {
  tone(PIN_BUZZER, 500, 500);
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 255);
  delay(1000);
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 0);
}
