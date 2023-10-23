int input0 = A0; // Define the 3 analog inputs: A0, A1, A2
int input1 = A1;
int input2 = A2;

int LEDR = 9;  //Define the 3 PWM digital outputs
int LEDG = 10;
int LEDB = 11;

int value0=0; // Create the 3 integral value type to store the analog reading values
int value1=0;
int value2=0;

void setup ()
{
  Serial.begin(9600); //Initiate the communication to display on the screen the information received from the board
  pinMode(LEDR,OUTPUT); // the digital pins will be output pins
  pinMode(LEDG,OUTPUT);
  pinMode(LEDB,OUTPUT);
}

void loop()
{

  int value0 = analogRead(input0); // Reads value of potentiometer number 0
  int value1 = analogRead(input1); // Reads value of potentiometer number 1
  int value2 = analogRead(input2); // Reads value of potentiometer number 2


  // analogWrite(LEDR, value0/4);
  // analogWrite(LEDG, value1/4);
  // analogWrite(LEDB, value2/4);

  analogWrite(LEDR,  map(value0, 0, 1023, 0, 255));
  analogWrite(LEDG,  map(value1, 0, 1023, 0, 255));
  analogWrite(LEDB,  map(value2, 0, 1023, 0, 255));
}