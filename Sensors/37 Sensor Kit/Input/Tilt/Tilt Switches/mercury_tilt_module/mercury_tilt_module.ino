/******************************************
 *Website: www.elegoo.com
 * 
 *Time:2017.12.12
 *
 ******************************************/
int  LedPinA  =  5; // To module pin 1
int  ButtonPinA  =  4; // To module pin 2
int buttonStateA = 0; 

/*
int  LedPinB  =  7;
int  ButtonPinB  =  6;
int buttonStateB = 0; 
*/

int brightness = 0; 

void setup()
{
  pinMode(LedPinA,  OUTPUT); 
  pinMode(ButtonPinA,  INPUT);
  Serial.begin(9600);
  //  pinMode(LedPinB,  OUTPUT); 
  // pinMode(ButtonPinB,  INPUT);
}


void loop()
{
  buttonStateA = digitalRead(ButtonPinA);

  if (buttonStateA  ==  HIGH) {
    if (brightness == 255) brightness = 0;
    else brightness++;

    Serial.print("buttonStateA is High: ");
    Serial.println(brightness);
  }  

  /*
  buttonStateB =  digitalRead(ButtonPinB);
  if (buttonStateB  ==  HIGH  &&  brightness !=  0)
  {
    brightness --;
  }
  */
  analogWrite(LedPinA,  brightness);   //
  //analogWrite(LedPinB,  255 - brightness); //  
  delay(25);
}
