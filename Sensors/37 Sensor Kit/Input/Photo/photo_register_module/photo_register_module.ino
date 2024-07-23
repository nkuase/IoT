/******************************************
 *Website: www.elegoo.com
 * 
 *Time:2017.12.12
 *
 ******************************************/
int  sensorPin  =  A0;     // select the input  pin for  the potentiometer 
int  ledPin  =  13;   // select the pin for  the LED
int  sensorValue =  0;  // variable to  store  the value  coming  from  the sensor

void setup()
{
  pinMode(ledPin,OUTPUT);
  Serial.begin(9600);
}

void loop(){
  // Fixed R = 10K
  // Photocell R = 80 - 20M 
  // More light -> less registance -> Larger Value
  // However, in this program, a smaller value with stronger light
  // I guess it's due to the wrong wiring (gnd -> 5V, and 5V -> ground) in the module.
  // Don't rewire, instead change this software if you need to get a bigger value with stronger light. 
  sensorValue =  analogRead(sensorPin);
  digitalWrite(ledPin,  HIGH); 
  delay(sensorValue); 
  digitalWrite(ledPin,  LOW);
  delay(sensorValue);
  Serial.println(sensorValue,  DEC);
}

