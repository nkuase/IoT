/******************************************
 *Website: www.elegoo.com
 * 
 *Time:2017.12.12
 *
 ******************************************/

int LED = 13;//define the led's port
int buttonpin = 3; //define the port of light blocking module

void  setup() {
  pinMode(LED,OUTPUT);//define digital variable val
  pinMode(buttonpin,INPUT);//define light blocking module as a output port
}

void  loop() { 
  int val=digitalRead(buttonpin);//read the value of the digital interface 3 assigned to val 
  if(val == HIGH)//when the light blocking sensor have signal, LED blink
  {
    digitalWrite(LED,LOW);
  }
  else
  {
    digitalWrite(LED,HIGH);
  }
}
