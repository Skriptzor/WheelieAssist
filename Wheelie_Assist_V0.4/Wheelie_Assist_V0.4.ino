//#include "FastLED.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


/**
 *  =======================================================================
 *  ==                                                                   ==
 *  ==  GLOBAL ENVIRONMENT VARIABLES                                     ==
 *  ==                                                                   ==
 *  ==  Allow easier debugging and running in different configurations.  ==
 *  ==                                                                   ==
 *  =======================================================================
 */
#define HAS_SONAR false
#define HAS_GYRO false
#define HAS_BLE true
#define SERIAL_BAUD_RATE 115200 // 9600
#define VERSION 0.61


/**
 *  =======================================================================
 *  ==                                                                   ==
 *  ==  BLE Variables                                                    ==
 *  ==                                                                   ==
 *  ==  Anything required for Bluetooth to function.                     ==
 *  ==                                                                   ==
 *  =======================================================================
 */
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *globalCharacteristic;
char valueToString[40];
std::string randomAngleValue;
std::string randomAngle[6] = {
  "74",
  "0",
  "45",
  "90",
  "24",
  "42"
};


/**
 *  =======================================================================
 *  ==                                                                   ==
 *  ==  WHEELIE Variables                                                ==
 *  ==                                                                   ==
 *  ==  Anything required for detecting the actual wheelie angle and     ==
 *  ==  bike height from the ground.                                     ==
 *  ==                                                                   ==
 *  =======================================================================
 */
#define YAxis 33
#define SonarTrig 11
#define SonarEcho 12
#define Refresh 500


//Temporary, until ultrasonic is fixed
#define OverrideSonar 4 // PIN Number
unsigned int buttonState;
boolean isDoingWheelie = true; // This value is toggled so the button press flips the mode from doing a wheelie to not doing one

//These variable need to be 'SET' when the app is running.

//Can be 'set' once rider is on the bike.
int YAxisStart = 1770; //What is 'FLAT' for the Y Axis?

//Can be set by lifting the forks as high as possible. How could we make this possible for just 1 person to set?
// Could probably have a button on the app that sends a signal to indicate that the bike is at the heighest point
int ForkHeight = 4; //Height of extended fork (in CM)

//This can be calculated by YAxisStart+-75.
int YAxisEnd = 2167; //What is 90* (straight up) for the Y Axis ?

int currentWheelie = 0;
int highestWheelie = 0;

long SonarDuration, SonarCm; //Variables for Sonar distance calculations

// Simple Storage
unsigned int bikeY;
unsigned int mappedAngle;
unsigned int skipSonarRead;


/**
 *  =======================================================================
 *  ==                                                                   ==
 *  ==  Initial Setup                                                    ==
 *  ==                                                                   ==
 *  ==  Runs on first boot of the device to get ready for the loop.      ==
 *  ==                                                                   ==
 *  =======================================================================
 */
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("\n"); // Create an empty line so the first output is away from the random garbage
  Serial.println("ENVIRONMENT CONFIG");
  Serial.print("Bluetooth: ");
  Serial.print(HAS_BLE ? "Enabled\n" : "Disabled\n");
  Serial.print("Gyroscope: ");
  Serial.print(HAS_GYRO ? "Enabled\n" : "Disabled\n");
  Serial.print("Sonar: ");
  Serial.print(HAS_SONAR ? "Enabled\n" : "Disabled\n");
  Serial.println("");

  if (HAS_GYRO) {
    pinMode(YAxis, INPUT);  
  }

  if (HAS_SONAR) {
    pinMode(SonarTrig, OUTPUT);
    pinMode(SonarEcho, INPUT); 
  } else { // No Sonar so enable the button overrides
    pinMode(OverrideSonar, INPUT);
  }

  if (HAS_BLE) {
    setupBLE();
  }

  Serial.print("System ready. Wheelie Assist v");
  Serial.println(VERSION);
  delay(2000); 
}


/**
 *  =======================================================================
 *  ==                                                                   ==
 *  ==  Main Loop                                                        ==
 *  ==                                                                   ==
 *  ==  Runs continuously while the device is powered.                   ==
 *  ==                                                                   ==
 *  =======================================================================
 */
void loop() {  
  // Get the current sonar distance from the ground
  if (HAS_SONAR && !shouldSkipSonarRead()) {
    //Send short signal to Sonar to get a clean output on high signal. Requires 10ms for a return signal
    digitalWrite(SonarTrig, LOW);
    delayMicroseconds(5);
    digitalWrite(SonarTrig, HIGH);
    delayMicroseconds(10);
    digitalWrite(SonarTrig, LOW);
  
    
    SonarDuration = pulseIn(SonarEcho, HIGH);
    SonarCm = (SonarDuration/2) / 29.1; //Convert sonar return duration into CM

    // Check if the user has come back to the ground first
    // That way we can send off the users wheelie info quicker
    if(SonarCm < ForkHeight && highestWheelie > 0) {
      Serial.print("Congrats on the wheelie! You got to ");
      Serial.print(highestWheelie);
      if (HAS_BLE) {
        sendToPhone(highestWheelie); 
      }
      Serial.println("*");
      highestWheelie = 0;
    }
  }

  // Debug the angle without needing a sonar connected
  // Uses two buttons to trigger sonar
  if (!HAS_SONAR && HAS_GYRO) {
    buttonState = digitalRead(OverrideSonar);

    if (buttonState == 1 && isDoingWheelie) { // Make the system think we're doing a wheelie.
      SonarCm = 5;
      highestWheelie = 0; // Reset highest wheelie.
      Serial.println("wheelie detected");
      isDoingWheelie = false;
    } else if (buttonState == 1 && !isDoingWheelie) { // Make the system think the wheelie has ended.
      SonarCm = 0;
      isDoingWheelie = true;
    }
  }

  // Check if the current sonar distance is heigher than the extended fork height the user set
  // so we can save their wheelie angle
  if (HAS_GYRO && HAS_SONAR && SonarCm > ForkHeight) {
    skipSonarRead = 150; // TODO: Play around with this value so it isn't super slow or really quick either
    bikeY = analogRead(YAxis);
    mappedAngle = map(bikeY, YAxisStart, YAxisEnd, 0, 90);

    if(mappedAngle > highestWheelie) {
      highestWheelie = mappedAngle;
    }
  }

  if (HAS_BLE && !HAS_GYRO && !HAS_SONAR) {
    sendToPhone(0);
  }
  
  delay(Refresh);
}


// Helper to skip the sonar read function for a short period of time
boolean shouldSkipSonarRead() {
  if (skipSonarRead > 0) {
    skipSonarRead = skipSonarRead - 1;
    return true;
  }
  return false;
}


/**
 *  =======================================================================
 *  ==                                                                   ==
 *  ==  Bluetooth Setup                                                  ==
 *  ==                                                                   ==
 *  ==  Helps configure the device for Bluetooth communication.          ==
 *  ==                                                                   ==
 *  =======================================================================
 */
void setupBLE() {
  Serial.println("Setting up Bluetooth");

  BLEDevice::init("MyESP32"); // TODO: The name needs to be uniq as no other rider should have the same named device
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  // Store the characteristic pointer so we can reference it later
  globalCharacteristic = pCharacteristic;
  
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  Serial.println("Bluetooth Ready");
  Serial.println("");
}


/**
 *  =======================================================================
 *  ==                                                                   ==
 *  ==  Send Data over Bluetooth                                         ==
 *  ==                                                                   ==
 *  ==  Helper function to convert an Integer to a String and            ==
 *  ==  send over Bluetooth.                                             ==
 *  ==                                                                   ==
 *  =======================================================================
 */
void sendToPhone(int wheelieAngle) {
  if (!HAS_GYRO && !HAS_SONAR && wheelieAngle == 0) {
//    sprintf(valueToString, "%s", random(0, 90));
  } else {
    sprintf(valueToString, "%s", wheelieAngle);
  }
  
  globalCharacteristic->setValue(valueToString);
  globalCharacteristic->notify();
}
