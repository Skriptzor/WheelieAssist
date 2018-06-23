//#include "FastLED.h"

#define YAxis 33
#define SonarTrig 11
#define SonarEcho 12
#define Ver 0.5
#define Refresh 100


//Temporary, until ultrasonic is fixed
#define OverideON 4
#define OverideOFF 2
int WheelieOveride = 0;

//These variable need to be 'SET' when the app is running.

//Can be 'set' once rider is on the bike.
int YAxisStart = 1770; //What is 'FLAT' for the Y Axis?

//Can be set by lifting the forks as high as possible. How could we make this possible for just 1 person to set?
int ForkHeight = 4; //Height of extended fork (in CM)

//This can be calculated by YAxisStart+-75.
int YAxisEnd = 2167; //What is 90* (straight up) for the Y Axis ?

int currentWheelie = 0;
int highestWheelie = 0;

long SonarDuration, SonarCm; //Variables for Sonar distance calculations

void setup() {
  Serial.begin(9600);
  
  pinMode(YAxis,INPUT);
  pinMode(OverideON,INPUT);
  pinMode(OverideOFF,INPUT);
  //pinMode(SonarTrig,OUTPUT);
  //pinMode(SonarEcho,INPUT);
  
  Serial.print("System ready. Wheelie Assist V");
  Serial.println(Ver);
  delay(2000); 
}

void loop() {
  int Y = analogRead(YAxis);
  int angle = map(Y,YAxisStart,YAxisEnd,0,90);

  int buttonStateON = digitalRead(OverideON);
  int buttonStateOFF = digitalRead(OverideOFF);
  
  if(buttonStateON == 1) { //overide, make the system think we're doing a wheelie.
    SonarCm = 5;
    highestWheelie = 0; //Reset highest wheelie.
    Serial.println("wheelie detected");
  }
  else if(buttonStateOFF == 1) { //override, make the system think the wheelie has ended.
    SonarCm = 0;
  }
  
  if(SonarCm < ForkHeight && highestWheelie > 0) {
    Serial.print("Congrats on the wheelie! You got to ");
    Serial.print(highestWheelie);
    Serial.println("*");
    highestWheelie = 0;
  }


  //Check to see if bike is flat again (Use ultrasonic).
  //If bike is flat and there's a highest wheelie above 0, send data to bluetooth function by Martin
  //After bluetooth function, set highest wheelie to 0.
  
  if(SonarCm > ForkHeight) {
    if(angle > highestWheelie) { //record highest wheelie. When should this reset?
      highestWheelie = angle;
    }
  }
  
  /*
  //Send short signal to Sonar to get a clean output on high signal. Requires 10ms for a return signal
  digitalWrite(SonarTrig, LOW);
  delayMicroseconds(5);
  digitalWrite(SonarTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(SonarTrig, LOW);

  pinMode(SonarEcho, INPUT);
  SonarDuration = pulseIn(SonarEcho, HIGH);
  SonarCm = (SonarDuration/2) / 29.1; //Convert sonar return duration into CM

  
  */
  delay(Refresh);
}
