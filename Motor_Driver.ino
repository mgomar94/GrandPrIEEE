int pinA = 5;
int pinB = 2;
int freq = 100;

void setup() {
  // put your setup code here, to run once:

  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  digitalWrite(pinA, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  //turnOn();
  motorTester(30);
  motorTester(60);
  stopMotor();

}

void motorTester(int dutyCycle){
  int firstDelay = dutyCycle*20;
  int secondDelay = 2000 - firstDelay;
  digitalWrite(pinB, HIGH);
  delayMicroseconds(firstDelay);
  digitalWrite(pinB, LOW);
  delayMicroseconds(secondDelay);  
}


void turnOn(){
  digitalWrite(pinB, HIGH);
}
void stopMotor(){
  digitalWrite(pinA, LOW);
  digitalWrite(pinB, HIGH);
  delay(2000);
}

