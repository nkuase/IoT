/******************************************
 *Website: www.elegoo.com
 * 
 *Time:2017.12.12
 *
 ******************************************/
int  sensorPin  =  A0;     // select the input  pin for  the potentiometer 
int  digitalSensorPin = 3;
int  ledPin  =  13;   // select the pin for  the LED
int  sensorValue =  0;  // variable to  store  the value  coming  from  the sensor

// lowest and highest sensor readings:
const int sensorMin = 0;     //  sensor minimum
const int sensorMax = 1024;  // sensor maximum

void setup()  {
  // initialize serial communication @ 9600 baud:
  Serial.begin(9600);  
  pinMode (sensorPin, INPUT) ;// output interface defines the flame sensor
  pinMode (digitalSensorPin, INPUT) ;// output interface defines the flame sensor
  pinMode (ledPin, OUTPUT);
}

void loop() {
  // read the sensor on analog A0:
  int sensorReading  = analogRead(sensorPin);
  int digitalSensorReading = digitalRead(digitalSensorPin) ;
  // map the sensor range (four options):
  // ex: 'long  int map(long int, long int, long int, long int, long int)'
	int range = map(sensorReading,  sensorMin, sensorMax, 0, 3);
  
  digitalWrite(ledPin,  digitalSensorReading); 

  // range value:
  switch (range) {
  case 0:    // A fire closer than 1.5 feet away.
    Serial.print("** Close  Fire **: "); Serial.println(digitalSensorReading);
    break;
  case 1:    // A fire between 1-3 feet away.
    Serial.print("**  Distant Fire **: "); Serial.println(digitalSensorReading);
    break;
  case 2:    // No fire detected.
    Serial.print("No  Fire: "); Serial.println(digitalSensorReading);
    break;
  }
  delay(100);  // delay between reads
}
