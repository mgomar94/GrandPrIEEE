#include <Servo.h>
Servo servo1;

void setup()
{
  servo1.attach(8);
}
void loop()
{
  servo1.write(820);
  delay(800);
  servo1.write(1500);
  delay(800);
  servo1.write(2210);
  delay(800);
} 

