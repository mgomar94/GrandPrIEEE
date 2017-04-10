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
#define PIXELS  128

Servo servo1;
int intDelay;      //Integration Period = (intDelay + 535 ) microseconds.
int Value;         //pixel intensity value.
int maxIndex;
int pixels[PIXELS] = {};
int inputPixel;

int Kp =2, Ki = 1, Kd = 2; // Need to test and modify
int Tp = 1500; // default position for servo motor
int integral = 0, derivative = 0, error = 0, lastError = 0;
int Turn = 0  ;

void setup(){
  pinMode(CLK, OUTPUT);
  pinMode(SI, OUTPUT);
  digitalWrite(CLK, LOW);
  digitalWrite(SI, LOW);
  servo1.attach(8);
  //Serial.begin(9600);
  Serial.begin(115200);
  Serial.flush();
}



void loop(){
  intDelay=1+analogRead(INTVAL)*7;  //read integration time from potentiometer.
  //Serial.print(">");                //Mark the start of a new frame
  delay(2);                         // used with 115200 bauds
  readPixel(0);                     //the first reading will be discarded.
  delayMicroseconds(intDelay);

  for(int i=0;i<PIXELS;i++){
    inputPixel = readPixel(i);
    pixels[i] = inputPixel;
    delayMicroseconds(intDelay);    //Delay added to the integration period.
  }

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
  delay(2);
  

}

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

// use a window of 10 to find the index with highest val
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
