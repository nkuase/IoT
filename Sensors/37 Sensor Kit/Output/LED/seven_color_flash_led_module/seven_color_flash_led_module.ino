/******************************************
 *Website: www.elegoo.com
 * 
 *Time:2017.12.12
 *
 ******************************************/

/*
Blink
Turns on an LED on for  two second, then off  for  two second, repeatedly.
This  example code is  in  the public  domain.
*/
int led = 13;

void setup() {
// initialize  the digital  pin  as an output.
// Pin  13 has an LED connected on most Arduino boards:
  pinMode(led, OUTPUT);
  Serial.begin(9600);
}

void loop()  {
  digitalWrite(led,  HIGH);  // set  the LED on 
  Serial.println(HIGH,  DEC);
  delay(200); // wait  for  a second 
  digitalWrite(led,  LOW);  // set  the LED off 
  delay(200);  // wait  for  a second
  Serial.println(LOW,  DEC);
}
