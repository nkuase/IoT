int Led = 13; //define LED port
int shockPin = 3;    // Use Pin 3 as our Input
int shockVal = HIGH; // This is where we record our shock measurement 

void setup ()
{
  pinMode(Led, OUTPUT); //define LED as a output port
  pinMode(shockPin, INPUT) ;
}
void loop ()
{
  shockVal = digitalRead (shockPin) ; // read the value from our sensor
  digitalWrite(Led, shockVal);
}
