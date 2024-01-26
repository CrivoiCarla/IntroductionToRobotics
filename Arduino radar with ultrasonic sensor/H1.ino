#include <Servo.h>
const int trigPin = 8;
const int echoPin = 9;
const int servoPin = 11;

long duration;
int distance;

Servo myservo;
int i = 15;         
int direction = 1;  
unsigned long previousMillis = 0;
int interval = 15;  

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  myservo.attach(servoPin);
  Serial.begin(9600);
}


void loop() {

  myservo.write(i);
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    calculateDistance();
    Serial.print(i);
    Serial.print(",");
    Serial.print(distance);
    Serial.print(".");

    i += direction;  
    if (i <= 15 || i >= 165) {
      direction *= -1;  
    }
  }
}

int calculateDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance =  duration / 29 / 2;
  return distance;
}