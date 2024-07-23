/******************************************
 *Website: www.elegoo.com
 * 
 *Time:2017.12.12
 *
 ******************************************/

int  Redpin =  11;     // select the pin for  the red LED 
int  Yellowpin  =10;       // select the pin  for  the blueLED 

void setup()
{
  pinMode(Redpin, OUTPUT);
  pinMode(Yellowpin, OUTPUT);
  Serial.begin(9600);
}

void loop()
{
  int val;
  for (val = 255; val > 0; val--) {
    analogWrite(Redpin, val);
    analogWrite(Yellowpin, 255 - val);
    delay(5);
  }
  for (val = 0; val < 255;  val++) {
    analogWrite(Redpin, val); 
    analogWrite(Yellowpin, 255 - val); 
    delay(5);
  }
  Serial.println(val,  DEC);
}

