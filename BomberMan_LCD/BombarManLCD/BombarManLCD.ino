#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

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


// lcd initialization
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int menuItem = 0;
int lastMenuItem = 0;
unsigned long lastDebounceTimeMenu = 0;
const unsigned long debounceDelayMenu = 300;


// Game variables
String playerName = "Player1";
int bombRange = 1;
int level = 1;


struct HighScoreEntry {
  char name[10];  // Assuming a maximum of 9 characters for the name
  unsigned int score;
};

HighScoreEntry highscores[3];  // Array to store top 3 highscores


const int lcdBrightnessPin = 3;
struct Settings {
  int lcdBrightness;
  int matrixBrightness;
};

Settings currentSettings;



void setup() {
  Serial.begin(9600);

  lc.shutdown(0, false);                                 // Turn off power saving, index 0
  lc.setIntensity(0, currentSettings.matrixBrightness);  // Set initial brightness
  lc.clearDisplay(0);                                    // Clear display on matrix 0


  pinMode(pinSW, INPUT_PULLUP);
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);

  initializeGameMap();

  for (int i = 0; i < maxBombs; i++) {
    bombs[i].active = false;
    bombs[i].isOn = false;
    bombs[i].lastBlink = 0;
  }
  // Initialize LCD and LED Matrix
  loadSettings();
  applySettings();

  lcd.begin(16, 2);
  lcd.print("Welcome to Game!");
  delay(3000);  // Show the message for 3 seconds
  updateMenu();
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

    displayHappyFace();

    unsigned long currentMillis = millis();

    if (currentMillis - lastDebounceTimeMenu > debounceDelayMenu) {
      int y = analogRead(pinY);

      if (y < 100) {  // Joystick mutat în sus
        menuItem--;
        lastDebounceTimeMenu = currentMillis;
      } else if (y > 900) {  // Joystick mutat în jos
        menuItem++;
        lastDebounceTimeMenu = currentMillis;
      }

      menuItem = constrain(menuItem, 0, 4);

      if (lastMenuItem != menuItem) {
        updateMenu();
        lastMenuItem = menuItem;
      }
    }

    if (isButtonPressed()) {
      switch (menuItem) {
        case 0:
          // Logica pentru "Start Game"
          menuState = GAME;
          break;
        case 1:
          // Logica pentru "Highscore"
          break;
        case 2:
          // Logica pentru "Settings"
          setSettings();
          break;
        case 3:
          // Logica pentru "About"
          showAbout();
          //delay(5000);  // Wait for a while before showing it again
          break;
        case 4:
          // Logica pentru "How to Play"
          showHowToPlay();
          //delay(5000);  // Wait for a while before showing it again
          break;
      }
    }
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


  // Horizontal movement
  if (joystickX < 200) {  // Left
    if (playerX > 0 && !walls[playerX - 1][playerY]) {
      playerX--;
    }
  } else if (joystickX > 900) {  // Right
    if (playerX < matrixSize - 1 && !walls[playerX + 1][playerY]) {
      playerX++;
    }
  }

  // Vertical movement
  else if (joystickY < 200) {  // Up
    if (playerY > 0 && !walls[playerX][playerY - 1]) {
      playerY--;
    }
  } else if (joystickY > 900) {  // Down
    if (playerY < matrixSize - 1 && !walls[playerX][playerY + 1]) {
      playerY++;
    }
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
      // displayGameInfo();
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
    displayVictoryMessage();
    displayHappyFace();
  } else {
    displayDefeatMessage();
    displaySadFace();
  }
  while (!isButtonPressed()) {
    // Keep checking if the button has been pressed
    delay(10);  // Small delay to prevent overwhelming the CPU
  }

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
  //displayGameInfo();

  while (!gameOver) {
    displayElements();
    updateBombs();
    displayGameInfo();

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

void updateMenu() {
  lcd.clear();
  switch (menuItem) {
    case 0:
      lcd.print(">Start Game");
      break;
    case 1:
      lcd.print(">Highscore");
      break;
    case 2:
      lcd.print(">Settings");
      break;
    case 3:
      lcd.print(">About");
      break;
    case 4:
      lcd.print(">How to Play");
      break;
  }
}

void showHowToPlay() {
  String message = "Move joystick to navigate, press button to select. ";
  int messageLen = message.length();
  int startPos = 0;

  while (startPos < messageLen) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("How to Play:");

    lcd.setCursor(0, 1);
    if (startPos + 16 < messageLen) {
      lcd.print(message.substring(startPos, startPos + 16));
    } else {
      // When near the end of the message, pad it with spaces
      String temp = message.substring(startPos, messageLen) + "                ";
      lcd.print(temp.substring(0, 16));
    }

    delay(500);  // Speed of the scrolling
    startPos++;
  }
}

// void showAbout() {
//   String aboutMessage = "Game: BomberMini, Author: Crivoi Carla, GitHub: @CrivoiCarla ";
//   int aboutLen = aboutMessage.length();
//   int startPos = 0;

//   while (startPos < aboutLen) {
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("About:");

//     lcd.setCursor(0, 1);
//     if (startPos + 16 < aboutLen) {
//       lcd.print(aboutMessage.substring(startPos, startPos + 16));
//     } else {
//       // When near the end of the message, pad it with spaces
//       String temp = aboutMessage.substring(startPos, aboutLen) + "                ";
//       lcd.print(temp.substring(0, 16));
//     }

//     delay(300);  // Speed of the scrolling
//     startPos++;
//   }
// }
void showAbout() {
  String aboutMessage = "Game: BomberMini, Author: Crivoi Carla, GitHub: @CrivoiCarla ";
  int aboutLen = aboutMessage.length();
  int startPos = 0;

  while (startPos < aboutLen) {
    lcd.clear();
    delay(100);  // Allow time for the LCD to clear the display

    lcd.setCursor(0, 0);
    lcd.print("About:");
    lcd.setCursor(0, 1);

    for (int i = 0; i < 16; ++i) {
      if (startPos + i < aboutLen) {
        lcd.write(aboutMessage.charAt(startPos + i));
      } else {
        lcd.write(' ');  // Fill the rest of the line with spaces
      }
    }

    delay(300);  // Speed of the scrolling
    startPos++;
  }
}


// void showAbout() {
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("About:");

//     // Display Game name
//     lcd.setCursor(0, 1);
//     lcd.print("Game: BomberMini");
//     delay(1000); // Wait for 1 second

//     // Display Author name
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("Author:");
//     lcd.setCursor(0, 1);
//     lcd.print("Crivoi Carla");
//     delay(1000); // Wait for 1 second

//     // Display GitHub link
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("GitHub:");
//     lcd.setCursor(0, 1);
//     lcd.print("@CrivoiCarla");
//     delay(1000); // Wait for 1 second
// }


void displayGameInfo() {
  lcd.clear();

  // First row: Player name and lives
  lcd.setCursor(0, 0);
  lcd.print(playerName.substring(0, 10));  // Truncate name if too long


  // Second row: Bomb icon, number of bombs, bomb range, and level
  lcd.setCursor(0, 1);
  lcd.print("B:");
  lcd.print(maxBombs - usedBombs);
  lcd.print(" R:");
  lcd.print(bombRange);
  lcd.print(" Lvl:");
  lcd.print(level);
}

void loadHighScores() {
  for (int i = 0; i < 3; i++) {
    EEPROM.get(i * sizeof(HighScoreEntry), highscores[i]);
    if (highscores[i].score == 0xFFFF) {  // If uninitialized
      // Set to default values
      strncpy(highscores[i].name, "Player", sizeof(highscores[i].name));
      highscores[i].score = 0;
    }
  }
}

void updateHighScores(const char* playerName, unsigned int score) {
  for (int i = 0; i < 3; i++) {
    if (score > highscores[i].score) {
      // Shift down lower scores
      for (int j = 2; j > i; j--) {
        highscores[j] = highscores[j - 1];
      }
      // Insert new score
      strncpy(highscores[i].name, playerName, sizeof(highscores[i].name));
      highscores[i].score = score;
      // Save to EEPROM
      EEPROM.put(i * sizeof(HighScoreEntry), highscores[i]);
      break;
    }
  }
}

void saveSettings() {
  EEPROM.put(100, currentSettings);
}

void loadSettings() {
  EEPROM.get(100, currentSettings);
}

void applySettings() {
  analogWrite(lcdBrightnessPin, currentSettings.lcdBrightness);
  lc.setIntensity(0, currentSettings.matrixBrightness);
}



void setSettings() {
  int newLcdBrightness = getNewLcdBrightness();
  int newMatrixBrightness = getNewMatrixBrightness();

  bool settingsChanged = false;

  if (newLcdBrightness >= 0 && newLcdBrightness <= 255) {
    if (newLcdBrightness != currentSettings.lcdBrightness) {
      currentSettings.lcdBrightness = newLcdBrightness;
      analogWrite(lcdBrightnessPin, newLcdBrightness);
      settingsChanged = true;
    }
  }

  if (newMatrixBrightness >= 0 && newMatrixBrightness <= 15) {
    if (newMatrixBrightness != currentSettings.matrixBrightness) {
      currentSettings.matrixBrightness = newMatrixBrightness;
      lc.setIntensity(0, newMatrixBrightness);  // Assuming a single matrix
      settingsChanged = true;
    }
  }

  if (settingsChanged) {
    saveSettings();
  }
}


int getNewLcdBrightness() {
  Serial.println("Enter new LCD brightness (0-255):");
  while (!Serial.available()) {
    // Wait for input
  }
  int newValue = Serial.parseInt();
  return newValue;
}

int getNewMatrixBrightness() {
  Serial.println("Enter new Matrix brightness (0-15):");
  while (!Serial.available()) {
    // Wait for input
  }
  int newValue = Serial.parseInt();
  return newValue;
}

void displayVictoryMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Congratulations!");
  lcd.setCursor(0, 1);
  lcd.print("You Won!");
}

void displayDefeatMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game Over!");
  lcd.setCursor(0, 1);
  lcd.print("Try Again?");
}
