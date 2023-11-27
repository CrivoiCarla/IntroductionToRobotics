#include "LedControl.h"  
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);
byte matrixBrightness = 2;

// Declare all the joystick pins
const int pinSW = 2;
const int pinX = A0;
const int pinY = A1;

// Game-specific variables
const byte playerBlinkRate = 500;  // Milliseconds, slow blink for player
const byte bombBlinkRate = 100;    // Milliseconds, fast blink for bombs

// Game map representation
bool walls[matrixSize][matrixSize];
bool ledStates[matrixSize][matrixSize] = { false };
bool playerMoved = false;

// Maximum number of bombs that can be placed at a level
const byte maxBombs = 6;

struct Bomb {
  byte x, y;
  bool active;
  bool isOn;
  unsigned long lastBlink;
  unsigned long placedTime;
};

unsigned usedBombs = 0;
Bomb bombs[maxBombs];

unsigned long lastMoveTime = 0;
const unsigned long moveDelay = 150;
byte number3[8] = { B00111100, B01000010, B00000010, B00011100, B00000010, B01000010, B00111100, B00000000 };
byte number2[8] = { B00111100, B01000010, B00000010, B00000100, B00001000, B00010000, B01111110, B00000000 };
byte number1[8] = { B00011000, B00101000, B01001000, B00001000, B00001000, B00001000, B01111110, B00000000 };

byte sadFace[8] = { 0x3C, 0x42, 0xA5, 0x81, 0x99, 0xA5, 0x42, 0x3C };
byte happyFace[8] = { 0x3C, 0x42, 0xA5, 0x81, 0xA5, 0x99, 0x42, 0x3C };

bool playerWins = false;
bool gameOver = false;

byte playerX = 0, playerY = 0;
const byte prizeX = 7, prizeY = 7;
const byte prizeBlinkRate = 50;
unsigned long lastPrizeBlink = 0;
bool prizeVisible = true;
unsigned long lastBombUseTime = 0;

enum GameState {
  MENU,
  GAME,
  RESTART,
};

GameState menuState = MENU;

void setup() {
  Serial.begin(9600);

  lc.shutdown(0, false);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);

  pinMode(pinSW, INPUT_PULLUP);
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);

  initializeGameMap();

  for (int i = 0; i < maxBombs; i++) {
    bombs[i].active = false;
    bombs[i].isOn = false;
    bombs[i].lastBlink = 0;
  }
}

void initializeGameMap() {
  for (int x = 0; x < matrixSize; x++) {
    for (int y = 0; y < matrixSize; y++) {
      walls[x][y] = (random(100) < 50) && !(x == playerX && y == playerY) && !(x == prizeX && y == prizeY);
    }
  }
}

void loop() {
  if (menuState == MENU) {
    displayMenu();
    waitForMenuSelection();
  } else if (menuState == GAME) {
    startGame();
  } else if (menuState == RESTART) {
    restartGame();
  }
}

void updatePlayerPosition() {
  unsigned long currentTime = millis();
  if (currentTime - lastMoveTime < moveDelay) {
    return;
  }

  int joystickX = analogRead(pinX);
  int joystickY = analogRead(pinY);

  int oldX = playerX;
  int oldY = playerY;

  int threshold = 512 / 4;

  if (joystickX < 512 - threshold) {
    if (playerX > 0 && !walls[playerX - 1][playerY]) {
      playerX--;
    }
  } else if (joystickX > 512 + threshold) {
    if (playerX < matrixSize - 1 && !walls[playerX + 1][playerY]) {
      playerX++;
    }
  }

  if (joystickY < 512 - threshold) {
    if (playerY < matrixSize - 1 && !walls[playerX][playerY + 1]) {
      playerY++;
    }
  } else if (joystickY > 512 + threshold) {
    if (playerY > 0 && !walls[playerX][playerY - 1]) {
      playerY--;
    }
  }

  if (playerMoved) {
    lastMoveTime = currentTime;
  }

  playerMoved = (oldX != playerX) || (oldY != playerY);

  if (playerMoved) {
    lastMoveTime = currentTime;

    if (!walls[oldX][oldY]) {
      setLedState(oldX, oldY, false);
    }
  }

  if (isButtonPressed()) {
    placeBomb();
  }
}

void displayElements() {
  displayWalls();
  updatePlayerPosition();
  updatePlayerBlink();
  updateBombBlink();

  if (millis() - lastPrizeBlink > prizeBlinkRate) {
    prizeVisible = !prizeVisible;
    lc.setLed(0, prizeX, prizeY, prizeVisible);
    lastPrizeBlink = millis();
  }

  if (playerX == prizeX && playerY == prizeY) {
    playerWins = true;
    gameOver = true;
  }
}

void updatePlayerBlink() {
  static unsigned long lastPlayerBlink = 0;
  if (millis() - lastPlayerBlink > playerBlinkRate) {
    bool currentPlayerState = getLedStatus(playerX, playerY);
    if (!walls[playerX][playerY]) {
      setLedState(playerX, playerY, !currentPlayerState);
    }
    lastPlayerBlink = millis();
  }
}

void setLedState(int row, int col, bool state) {
  lc.setLed(0, row, col, state);
  ledStates[row][col] = state;
}

bool getLedStatus(int row, int col) {
  return ledStates[row][col];
}

void placeBomb() {
  if (walls[playerX][playerY]) return;

  for (int i = 0; i < maxBombs; i++) {
    if (!bombs[i].active) {
      bombs[i].x = playerX;
      bombs[i].y = playerY;
      bombs[i].active = true;
      bombs[i].placedTime = millis();
      usedBombs += 1;
      break;
    }
  }
}

void updateBombs() {
  unsigned long currentTime = millis();
  for (int i = 0; i < maxBombs; i++) {
    if (bombs[i].active && currentTime - bombs[i].placedTime > 2000) {
      explodeBomb(i);
      bombs[i].active = false;
    }
  }
}

void explodeBomb(int bombIndex) {
  int x = bombs[bombIndex].x;
  int y = bombs[bombIndex].y;

  if (x > 0) walls[x - 1][y] = false;
  if (x < matrixSize - 1) walls[x + 1][y] = false;
  if (y > 0) walls[x][y - 1] = false;
  if (y < matrixSize - 1) walls[x][y + 1] = false;

  lc.setLed(0, x - 1, y, false);
  lc.setLed(0, x + 1, y, false);
  lc.setLed(0, x, y - 1, false);
  lc.setLed(0, x, y + 1, false);

  setLedState(x, y, false);

  if (!walls[playerX][playerY]) {
    setLedState(playerX, playerY, getLedStatus(playerX, playerY));
  }
}

void updateBombBlink() {
  for (int i = 0; i < maxBombs; i++) {
    if (bombs[i].active) {
      if (millis() - bombs[i].lastBlink > bombBlinkRate) {
        bombs[i].isOn = !bombs[i].isOn;
        setLedState(bombs[i].x, bombs[i].y, bombs[i].isOn);
        bombs[i].lastBlink = millis();
      }
    }
  }
}

void displayWalls() {
  for (int x = 0; x < matrixSize; x++) {
    for (int y = 0; y < matrixSize; y++) {
      if (walls[x][y]) {
        lc.setLed(0, x, y, true);
      } else {
        if (!(x == playerX && y == playerY)) {
          lc.setLed(0, x, y, false);
        }
      }
    }
  }
}

bool isButtonPressed() {
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 15;
  static bool buttonPressed = false;
  static int buttonState = HIGH;
  int reading = digitalRead(pinSW);

  if (reading != buttonState) {
    lastDebounceTime = millis();
    buttonPressed = false;
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && !buttonPressed) {
      buttonPressed = true;
      return true;
    }
  }
  buttonState = reading;
  return false;
}

void nextLevelGame() {
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      lc.setLed(0, i, j, true);
      delay(50);
    }
  }

  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      lc.setLed(0, i, j, false);
      delay(50);
    }
  }
}

void displayNumber(byte numberPattern[]) {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      bool isLedOn = bitRead(numberPattern[row], 7 - col);
      lc.setLed(0, row, col, isLedOn);
    }
  }
}

void startGameAnimation() {
  displayNumber(number3);
  delay(1000);
  lc.clearDisplay(0);

  displayNumber(number2);
  delay(1000);
  lc.clearDisplay(0);

  displayNumber(number1);
  delay(1000);
  lc.clearDisplay(0);
}

void displaySadFace() {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      lc.setLed(0, row, col, bitRead(sadFace[row], 7 - col));
    }
  }
}

void displayHappyFace() {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      lc.setLed(0, row, col, bitRead(happyFace[row], 7 - col));
    }
  }
}

void endGameAnimation(bool isWin) {
  if (isWin) {
    displayHappyFace();
  } else {
    displaySadFace();
  }
  delay(2000);
  lc.clearDisplay(0);
}

void displayBombs() {
  updateBombBlink();
}

void displayPrize() {
  if (millis() - lastPrizeBlink > prizeBlinkRate) {
    prizeVisible = !prizeVisible;
    lc.setLed(0, prizeX, prizeY, prizeVisible);
    lastPrizeBlink = millis();
  }
}

void startGame() {
  usedBombs = 0;
  gameOver = false;
  playerWins = false;
  playerX = 0;
  playerY = 0;

  lc.clearDisplay(0);

  startGameAnimation();

  initializeGameMap();

  while (!gameOver) {
    displayElements();
    updateBombs();

    if (usedBombs == maxBombs && millis() - lastBombUseTime > 10000) {
      gameOver = true;
      playerWins = false;
    }
    if (playerX == prizeX && playerY == prizeY) {
      playerWins = true;
      gameOver = true;
    }
  }

  endGameAnimation(playerWins);
}

void displayMenu() {
  Serial.println("Menu:");
  Serial.println("1. Start Game");
  Serial.println("2. Restart");
}

void waitForMenuSelection() {
  int choice = 0;
  while (choice != 1 && choice != 2) {
    displayHappyFace();
    Serial.print("Enter your choice (1 for Start Game, 2 for Restart): ");
    while (!Serial.available()) {
    }
    choice = Serial.parseInt();
    Serial.println(choice);
  }

  if (choice == 1) {
    menuState = GAME;
  } else if (choice == 2) {
    restartGame();
  }
}

void restartGame() {
  usedBombs = 0;
  gameOver = false;
  playerWins = false;
  playerX = 0;
  playerY = 0;

  lc.clearDisplay(0);
  displaySadFace();
  delay(1000);
  startGameAnimation();

  initializeGameMap();

  while (!gameOver) {
    displayElements();
    updateBombs();

    // Check for game over conditions and winning conditions
    if (usedBombs == maxBombs && millis() - lastBombUseTime > 10000) {
      gameOver = true;
      playerWins = false;
    }
    if (playerX == prizeX && playerY == prizeY) {
      playerWins = true;
      gameOver = true;
    }
  }


  endGameAnimation(playerWins);

  // After the game is over, return to the menu state
  menuState = MENU;
}
