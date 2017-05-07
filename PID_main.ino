/*TSL201R_Scanning.ino Arduining 26 MAR 2014

Linear Array Sensor TSL201R (TAOS) is now AMS:
  -64 x 1 Linear Sensor Array 200 DPI

-Sketch used with the Arduino NANO.
  Data is Send at 115200 bauds (ASCII messages ending with CR and LF)
  One frame (64x1 image) has the followinf format:
      >5890    Indicates the integration period in microseconds.
      843      this value is discarded.
      234      value of the pixel 1
      242      value of the pixel 2
       .         .
       .         .
      245      value of the pixel 64

-Reading only one pixel in each scan.
-The image is completed after 1+64 scans.(first is discarded)
-First scan is repeated to permit a complete integration period). 
-Pixel integration Period is adjustable (potentiometer in analog channel 1).

*/
#include <Servo.h>
#define CLK     2
#define SI      3
#define VOUT    0   //pixel intensity in the analog channel 0
#define INTVAL  1   //integration time adjust in the analog channel 1.
#define PIXELS  127


/* ----- Variables ----- */
Servo servo1;
int intDelay;      //Integration Period = (intDelay + 535 ) microseconds.
int Value;         //pixel intensity value.
int maxIndex;
int pixels[PIXELS] = {};
int inputPixel;

int Kp =0.65, Ki = 1, Kd = 0.5; // Need to test and modify
int Tp = 1500; // default position for servo motor
int integral = 0, derivative = 0, error = 0, lastError = 0;
int Turn = 0;

int pinA = 13;
int MOTOR_PIN = 5;



//Motor Variable


int MAX_SPEED = 50;
int MED_FAST_SPEED = 45;
int MED_MED_SPEED = 40;
int MED_SLOW_SPEED = 35;
int SLOW_SPEED = 30;

int R_SHARP_ANGLE = 102;    // line center ranged from 10 pixels to the right
int R_MED_SHARP_ANGLE = 92;
int R_MED_MED_SHARP_ANGLE = 80;
int R_MED_SMOOTH_ANGLE = 70;
int L_SHARP_ANGLE = 22;    // line center ranged from 10 pixels to the left
int L_MED_SHARP_ANGLE = 32;
int L_MED_MED_SHARP_ANGLE = 44;
int L_MED_SMOOTH_ANGLE = 54;




void setup(){
  pinMode(CLK, OUTPUT);
  pinMode(SI, OUTPUT);
  digitalWrite(CLK, LOW);
  digitalWrite(SI, LOW);
  pinMode(pinA, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(pinA, HIGH);
  servo1.attach(8);
  //Serial.begin(9600);
  Serial.begin(115200);
  Serial.flush();
  
}



void loop(){

  /* Scan 128 pixels */
  intDelay=1+analogRead(INTVAL)*10;  //read integration time from potentiometer.
  //Serial.print(">");                //Mark the start of a new frame
  delay(2);                         // used with 115200 bauds
  readPixel(0);                     //the first reading will be discarded.
  delayMicroseconds(intDelay);
  for(int i=0;i<PIXELS;i++){
    inputPixel = readPixel(i);
    pixels[i] = inputPixel;
    delayMicroseconds(intDelay*2);    //Delay added to the integration period.
  }

  /*
  maxIndex = findMax();
  error = PIXELS/2 - maxIndex;
  integral += error;
  derivative = error - lastError;
  Turn = Kp * error + Ki * integral + Kd * derivative;
  //Turn = Turn/10;   // Scale turn val based on servo motor input;
  if(abs(derivative) < 3){
    Turn = 0;  
    integral = 0;  
  }
  Tp += Turn;
  lastError = error;
  if(Tp > 2200){
    Tp = 2200;
    Turn = 0;
    integral -= 50;
  }
  else if(Tp < 850){
    Tp = 850;
    Turn = 0;
    integral += 50;
  }

  Serial.println(Tp);
  servo1.write(Tp);
  */

  int middle = findMiddleDerivative(3);
  Serial.println(middle);
  int angle = servoTurn(middle);
  //Serial.println(angle);
  servo1.write(angle);


  //Motor Speeds
  /*
  if(angle > R_SHARP_ANGLE || L_SHARP_ANGLE > angle) 
  {
    //Set the duty cycle of the motor to low speed
    int motorPower  = motorPWM(SLOW_SPEED);     //Currently 20% motor
    analogWrite(MOTOR_PIN, motorPower);
  }
  
  else if(angle > R_MED_SHARP_ANGLE || L_MED_SHARP_ANGLE > angle)
  {
    int motorPower = motorPWM(MED_SLOW_SPEED);
    analogWrite(MOTOR_PIN, motorPower);
  }
  
  else if(angle > R_MED_MED_SHARP_ANGLE || L_MED_MED_SHARP_ANGLE > angle)
  {
    int motorPower = motorPWM(MED_MED_SPEED);
    analogWrite(MOTOR_PIN, motorPower);
  }
  
  else if(angle > R_MED_SMOOTH_ANGLE || L_MED_SMOOTH_ANGLE > angle) 
  {
    //Set the duty cycle of the motor to medium speed
    int motorPower  = motorPWM(MED_FAST_SPEED);     //Currently 30% motor
    analogWrite(MOTOR_PIN, motorPower);
  }
  
  else 
  {
    //Set the duty cycle of the motor
    int motorPower  = motorPWM(MAX_SPEED);     //Currently 40% motor
    analogWrite(MOTOR_PIN, motorPower);
  }
  */
  digitalWrite(pinA, HIGH);
  motorTester(20);
  digitalWrite(pinA, HIGH);
  motorTester(80);
    
  //delay(2);

}

/*
 * Fix camera input below (Might not be reading in 128 per interval) Something about charge transfer.
 * Then check algorithm below.
 * When the car steers off road and can no longer see the tape, lock the wheels to turn back to where the tape
 * was last seen until it is found again.
 * Tape length is 1 inch, find out field of view of camera (1 foot?) and corresponding tape length in pixels.
 * 
 */


//------------------ Send the intensity of the pixel serially -----------------
int readPixel(int pixel){
  digitalWrite(CLK, LOW);
  digitalWrite(SI, HIGH);         
  digitalWrite(CLK, HIGH);
  digitalWrite(SI, LOW);
 

  for(int i=0;i<pixel; i++){    //Clock pulses before pixel reading
    digitalWrite(CLK, LOW);
    digitalWrite(CLK, HIGH);    //Select next pixel and reset integrator of actual pixel.
  }

  Value = analogRead(VOUT);
  //Serial.println(Value);
//  delay(8);       //used with 9600 bauds
  delay(1);         //used with 115200 bauds
  for(int i=0;i<=(PIXELS-pixel); i++){    //Clock pulses after pixel reading.
    digitalWrite(CLK, LOW);
    digitalWrite(CLK, HIGH);    //Select next pixel and reset integrator of actual pixel.
  }
  return Value;
}

/* use a window of 10 to find the index with highest val */
int findMax() {
  const int WindowSize = 7;
  int sum = 0;
  int maxSum = 0;
  int maxInd = 0;
  for(int i = 0; i < PIXELS-WindowSize+1; i++) {
    if(i == 0) {
      for(int j = 0; j < WindowSize; j++) {
        sum += pixels[j];
        maxSum = sum;
        maxInd = i + WindowSize / 2;
      }
    }
    else {
      sum = sum - pixels[i-1] + pixels[i+WindowSize-1];
      if(sum > maxSum) {
        maxSum = sum;
        maxInd = i + WindowSize / 2;
      }
    }
  }
  return maxInd;
}


/* Find the threshold to determine the location of the line 
 * Created: Yicheng April 20
 */
int findThreshold()
{
  int sum = 0;
  int max = 0;

  for(int i = 0; i < PIXELS; i++) 
  {
    sum += pixels[i];
    if(pixels[i] > max) 
    {
      max = pixels[i];
    }
  }

  int average = sum / PIXELS;

  if((max - average) < (10))
  {
    return -1;
  }
  
  // Adaptive threshold -- middle point max and average
  // May need to modify
  return (max + average) / 2;
}


int findMiddleDerivative(int interval){
  
  int maxSlope = 0;
  int minSlope = 999999;
  int leftEdge;
  int rightEdge;
  int currSlope;

  for(int i = 0; i < PIXELS-interval; i++) 
  {
    currSlope = (pixels[i+interval] - pixels[i])/interval;
    if(currSlope > maxSlope){
      maxSlope = currSlope;
      leftEdge = i + (interval/2);
    }
    if(currSlope < minSlope){
      minSlope = currSlope;
      rightEdge = i + (interval/2);
    }
  }

  return (leftEdge+rightEdge)/2;

}


/* Find the middle of the line 
 * Created: Yicheng, April 20
 */
int findMiddle()
{
  int threshold = findThreshold();
  int end;
  int count = 0;
  int currCount = 0;
  int middle;
  int start;

  if(threshold < 0){
    return 63;
  }
  
  for(int i = 0; i < PIXELS; i++) 
  {
    if(pixels[i] > threshold) 
    {
      if(currCount == 0)
        start = i;
      //end = i;
      currCount++;

      // Don't check the rest pixels if the next value drops significantly
      // May need to modify
      if(pixels[i+1] < (threshold - (pixels[i]-threshold))) {
        if(currCount > count) {
          count = currCount;
          middle = (i + start) / 2;
        }
        currCount = 0;
      }
    }

    // Not sure what this line is doing
    //if(pixels[i+1] == 128 || pixels[i+1] == 0) break;
  }

  return middle;
} 


/* 
 * Determine how much servo needs to trun
 */
 int servoTurn(int middle) 
 {
  if(middle < 21) 
    return 850;
  else if (middle > 105) 
    return 2200;
  else 
  {
    return 1500 + 16 * (middle - 63);
  }
 }


 /* Calculate the duty cycle for the motor
 *   Began: APRIL 11, 2015
 *   Edited: APRIL 11, 2015
 */
int motorPWM(int value)
{
  int motorDuty = 2.55 * value;       //2.55 * value inputed
  return motorDuty;
}

void motorTester(int dutyCycle){
  int firstDelay = dutyCycle*20;
  int secondDelay = 2000 - firstDelay;
  digitalWrite(MOTOR_PIN, HIGH);
  delay(firstDelay);
  digitalWrite(MOTOR_PIN, LOW);
  delay(secondDelay);  
}
