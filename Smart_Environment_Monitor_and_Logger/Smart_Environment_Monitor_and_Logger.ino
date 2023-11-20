#include <EEPROM.h>

int photocellPin = 0;
const int trigPin = 9;
const int echoPin = 10;
int ultrasonicThreshold = 20;
int ldrThreshold = 500;
int samplingInterval = 5;


int ledRedPin = 5;
int ledGreenPin = 6;
int ledBluePin = 7;
bool automaticMode = false;


int manualRedValue = 0;
int manualGreenValue = 0;
int manualBlueValue = 0;


const long interval = 100;


const int ultrasonicStartAddress = 0;
const int ldrStartAddress = 20;
const int thermoStartAddress = 40;

unsigned long ledOnTime = 0;
const long ledDuration = 3000;  // 3 secunde
bool isLedOn = false;

int currentUltrasonicDistance = 0;
int currentLdrValue = 0;
unsigned long lastSensorReadTime = 0;

bool displayMainMenuView = true;


int thermistorPin = 1;
int Vo;
float R1 = 10000;
float logR2, R2, T, Tc, Tf;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
float currentThermoValue;


void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(photocellPin, INPUT);

  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledBluePin, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

}

void loop() {
  readInputSenzor();
  displayMainMenu();
  int choice = getUserChoice();

  switch (choice) {
    case 1:
      displayMainMenuView = true;
      sensorSettingsMenu();
      break;
    case 2:
      displayMainMenuView = true;
      resetLoggerData();
      break;
    case 3:
      displayMainMenuView = true;
      systemStatusMenu();
      break;
    case 4:
      displayMainMenuView = true;
      rgbLedControlMenu();
      break;
  }
}
void displayMainMenu() {
  Serial.println("Main Menu:");
  Serial.println("1. Sensor Settings");
  Serial.println("2. Reset Logger Data");
  Serial.println("3. System Status");
  Serial.println("4. RGB LED Control");
  Serial.print("Enter choice: ");
}

void readInputSenzor() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorReadTime >= samplingInterval * 1000) {
    lastSensorReadTime = currentMillis;
    currentUltrasonicDistance = readUltrasonicSensor();
    currentLdrValue = readLdrSensor();
    currentThermoValue = readTemperature(thermistorPin, 10000, c1, c2, c3);
    checkUltrasonicSensor(currentUltrasonicDistance);
    checkLdrSensor(currentLdrValue);
    logUltrasonicData(currentUltrasonicDistance);
    logLdrData(currentLdrValue);
    logThermoData(currentThermoValue);
  }
}



int getUserChoice() {
  unsigned long lastCheck = 0;
  readInputSenzor();

  while (!Serial.available()) {
    readInputSenzor();
    unsigned long currentMillis = millis();
    if (currentMillis - lastCheck >= interval) {
      lastCheck = currentMillis;
    }
  }
  int choice = Serial.parseInt();
  while (Serial.available()) {
    readInputSenzor();
    Serial.read();
  }
  return choice;
}


void sensorSettingsMenu() {
  bool inSensorMenu = true;
  while (inSensorMenu) {
    Serial.println("Sensor Settings:");
    Serial.println("1. Set Sensors Sampling Interval");
    Serial.println("2. Set Ultrasonic Alert Threshold");
    Serial.println("3. Set LDR Alert Threshold");
    Serial.println("4. Back to Main Menu");
    Serial.print("Enter choice: ");

    int choice = getUserChoice();
    switch (choice) {
      case 1:
        Serial.println("1");
        setSamplingInterval();
        break;
      case 2:
        Serial.println("2");
        setUltrasonicThreshold();
        break;
      case 3:
        Serial.println("3");
        setLdrThreshold();
        break;
      case 4:
        Serial.println("4");
        inSensorMenu = false;
        break;
      default:
        Serial.println(choice);
        Serial.println("Invalid choice. Try again.");
        break;
    }
  }
}


void setSamplingInterval() {
  Serial.print("Enter sampling interval (1-10 seconds): ");
  int interval = getUserChoice();
  if (interval >= 1 && interval <= 10) {
    samplingInterval = interval;
    Serial.println("Sampling interval set.");
  } else {
    Serial.println("Invalid interval. Please enter a value between 1 and 10.");
  }
}

void setUltrasonicThreshold() {
  Serial.print("Enter ultrasonic threshold value: ");
  int threshold = getUserChoice();
  ultrasonicThreshold = threshold;
  Serial.println("Ultrasonic threshold set.");
}

void setLdrThreshold() {
  Serial.print("Enter LDR threshold value: ");
  int threshold = getUserChoice();
  ldrThreshold = threshold;
  Serial.println("LDR threshold set.");
}



void resetLoggerData() {
  Serial.println("Are you sure you want to delete all data?");
  Serial.println("1. Yes");
  Serial.println("2. No");
  Serial.print("Enter choice: ");

  int choice = getUserChoice();
  switch (choice) {
    case 1:
      Serial.println("1");
      clearEEPROM();
      Serial.println("All data has been deleted.");
      break;
    case 2:
      Serial.println("2");
      Serial.println("Data reset canceled.");
      break;
    default:
      Serial.println("Invalid choice.");
      break;
  }
}

void clearEEPROM() {
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
}


void systemStatusMenu() {
  bool inSystemStatusMenu = true;
  while (inSystemStatusMenu) {
    Serial.println("System Status:");
    Serial.println("1. Current Sensor Readings");
    Serial.println("2. Current Sensor Settings");
    Serial.println("3. Display Logged Data");
    Serial.println("4. Back to Main Menu");
    Serial.print("Enter choice: ");

    int choice = getUserChoice();
    switch (choice) {
      case 1:
        displayCurrentSensorReadings();
        break;
      case 2:
        displayCurrentSensorSettings();
        break;
      case 3:
        displayLoggedData();
        break;
      case 4:
        inSystemStatusMenu = false;
        break;
      default:
        Serial.println("Invalid choice. Try again.");
        break;
    }
  }
}

void displayCurrentSensorReadings() {
  Serial.println("Current Sensor Readings (Press any key to stop):");
  unsigned long previousMillis = millis();
  while (!Serial.available()) {
    unsigned long currentMillis = millis();
    
    if (currentMillis - previousMillis >= 500) { 
      previousMillis = currentMillis; 

      readInputSenzor();
      Serial.print("Ultrasonic Distance: ");
      Serial.print(currentUltrasonicDistance);
      Serial.print(" cm | LDR Value: ");
      Serial.print(currentLdrValue);
      Serial.print(" | Thermo Value: ");
      Serial.println(currentThermoValue);
    }
  }

  while (Serial.available()) {
    readInputSenzor();
    Serial.read();
  }
}


void checkUltrasonicSensor(int distance) {
  if (distance > ultrasonicThreshold) {
    Serial.println("Ultrasonic sensor alert: Object too far!");

    if (automaticMode && !isLedOn) {
      turnOnLed();
    }
  } else if (automaticMode && isLedOn && millis() - ledOnTime > ledDuration) {
    restoreLedColor();
  }
}

void checkLdrSensor(int ldrValue) {
  if (ldrValue > ldrThreshold) {
    Serial.println("LDR sensor alert: Light level is high!");

    if (automaticMode && !isLedOn) {
      turnOnLed();
    }
  } else if (automaticMode && isLedOn && millis() - ledOnTime > ledDuration) {
    restoreLedColor();
  }
}

void turnOnLed() {
  analogWrite(ledRedPin, 255);
  analogWrite(ledGreenPin, 0);
  analogWrite(ledBluePin, 0);
  ledOnTime = millis();
  isLedOn = true;
}

void restoreLedColor() {
  updateLedColor(manualRedValue, manualGreenValue, manualBlueValue);
  isLedOn = false;
}


int readUltrasonicSensor() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;

  return distance;
}

int readLdrSensor() {
  int ldrValue = analogRead(photocellPin);
  return ldrValue;
}


void displayCurrentSensorSettings() {
  Serial.print("Sampling Interval: ");
  Serial.print(samplingInterval);
  Serial.println(" seconds");

  Serial.print("Ultrasonic Threshold: ");
  Serial.println(ultrasonicThreshold);

  Serial.print("LDR Threshold: ");
  Serial.println(ldrThreshold);
}


void displayLoggedData() {
  Serial.println("Which sensor data to display? Type 'U' for Ultrasonic, 'L' for LDR, or 'T' for Temperature:");

  unsigned long lastCheck = 0;
  const long interval = 100;  

  while (!Serial.available()) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastCheck >= interval) {
      lastCheck = currentMillis;
    }
  }

  char sensorType = Serial.read();
  int startAddress;

  if (sensorType == 'U') {
    startAddress = ultrasonicStartAddress;
    Serial.println("Last 10 Ultrasonic readings:");
  } else if (sensorType == 'L') {
    startAddress = ldrStartAddress;
    Serial.println("Last 10 LDR readings:");
  } else if (sensorType == 'T') {
    startAddress = thermoStartAddress;
    Serial.println("Last 10 Temperature readings:");
  } else {
    Serial.println("Invalid sensor type.");
    return;
  }

  for (int i = 0; i < 10; i++) {
    int value;
    EEPROM.get(startAddress + i * sizeof(int), value);
    Serial.println(value);
  }

  while (Serial.available()) Serial.read(); 
}



void rgbLedControlMenu() {
  bool inLedMenu = true;
  while (inLedMenu) {
    Serial.println("RGB LED Control:");
    Serial.println("1. Manual Color Control");
    Serial.println("2. Toggle Automatic ON/OFF");
    Serial.println("3. Back to Main Menu");
    Serial.print("Enter choice: ");

    int choice = getUserChoice();
    switch (choice) {
      case 1:
        setManualColor();
        break;
      case 2:
        toggleAutomaticMode();
        break;
      case 3:
        inLedMenu = false;
        break;
      default:
        Serial.println("Invalid choice. Try again.");
        break;
    }
  }
}

void setManualColor() {
  Serial.println("Enter RGB values (0-255) for Red, Green, Blue, separated by commas. Example: 255,0,0");

  unsigned long lastCheck = 0;
  const long interval = 100;  
  bool awaitingInput = true;

  while (awaitingInput) {
    unsigned long currentMillis = millis();

    if (Serial.available()) {
      String rgbInput = Serial.readStringUntil('\n');
      rgbInput.trim();  

      if (isValidRgbInput(rgbInput)) {
        int red = getValue(rgbInput, ',', 0).toInt();
        int green = getValue(rgbInput, ',', 1).toInt();
        int blue = getValue(rgbInput, ',', 2).toInt();

        if (isValidColorValue(red) && isValidColorValue(green) && isValidColorValue(blue)) {
          manualRedValue = red;
          manualGreenValue = green;
          manualBlueValue = blue;
          updateLedColor(manualRedValue, manualGreenValue, manualBlueValue);
          Serial.println("RGB color set.");
          awaitingInput = false;  
        } else {
          Serial.println("Values must be between 0 and 255.");
          Serial.println("Enter RGB values (0-255) for Red, Green, Blue, separated by commas. Example: 255,0,0");
        }
      } else {
        Serial.println("Invalid format. Please enter values in the format R,G,B where R, G, and B are numbers between 0 and 255.");
        Serial.println("Enter RGB values (0-255) for Red, Green, Blue, separated by commas. Example: 255,0,0");
      }
    } else if (currentMillis - lastCheck >= interval) {
      lastCheck = currentMillis;
      
    }
  }
}


bool isValidColorValue(int value) {
  return value >= 0 && value <= 255;
}

bool isValidRgbInput(String input) {
  for (unsigned int i = 0; i < input.length(); i++) {
    if (!isDigit(input[i]) && input[i] != ',') {
      return false;
    }
  }
  return true;
}


String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void updateLedColor(int red, int green, int blue) {
  analogWrite(ledRedPin, red);
  analogWrite(ledGreenPin, green);
  analogWrite(ledBluePin, blue);
}

void toggleAutomaticMode() {
  automaticMode = !automaticMode;
  Serial.print("Automatic mode is now: ");
  Serial.println(automaticMode ? "ON" : "OFF");
  if (!automaticMode) {
    updateLedColor(manualRedValue, manualGreenValue, manualBlueValue);
  }
}

void logUltrasonicData(int distance) {
  static int address = ultrasonicStartAddress;
  EEPROM.put(address, distance);
  address += sizeof(int);

  if (address >= ultrasonicStartAddress + 20) {
    address = ultrasonicStartAddress;
  }
}

void logLdrData(int ldrValue) {
  static int address = ldrStartAddress;
  EEPROM.put(address, ldrValue);
  address += sizeof(int);

  if (address >= ldrStartAddress + 20) {
    address = ldrStartAddress;
  }
}

void logThermoData(int Tc) {
  static int address = thermoStartAddress;
  EEPROM.put(address, Tc);
  address += sizeof(int);

  if (address >= thermoStartAddress + 20) {
    address = thermoStartAddress;
  }
}

float readTemperature(int ThermistorPin, float R1, float c1, float c2, float c3) {
  int Vo;
  float R2, logR2, T, Tc;

  Vo = analogRead(A1); 
  R2 = R1 * (1023.0 / (float)Vo - 1.0); 
  logR2 = log(R2); 
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2)); 
  Tc = T - 273.15; 

  return Tc; 
}

