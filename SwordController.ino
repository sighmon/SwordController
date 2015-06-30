/*

 Sword controller for an art piece.
 
 The art piece is two swords dangling from wires. 
 When the piece is slowly accelerated and decelerated, the swords gently touch
 making for a kind of sword ballet.
 
 This software controls the speed and acceleration of the rotation.
 http://www.airspayce.com/mikem/arduino/AccelStepper/
 
 It also uses a BLE Mini to talk to the Arduino, so that the values
 stored in EEPROM can be edited after the piece is installed.
 http://redbearlab.com/blemini/

*/

#include <ble_mini.h>
#include <AccelStepper.h>
#include <EEPROM.h>

// Define EEPROM address to write values to
int accelerationAddress = 0;
int speedAddress = 1;
int stepsAddress = 2;
int delayAddress = 3;

// Define a stepper and the pins it will use
#define DIRECTION_PIN      6
#define ENABLE_PIN         7
#define STEP_PIN           8
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIRECTION_PIN);
int stepsPerRotation = 118;
int maxSpeed = 7;
float acceleration = 0.5;
int delayBetweenLoops = 3000;

// Pin 13 is the onboard LED pin.. for testing it's alive
#define DIGITAL_OUT_PIN    13

unsigned long currentMillis;        // store the current value from millis()
unsigned long previousMillis;       // for comparison with currentMillis
int samplingInterval = 250;          // how often to run the main loop (in ms)

void setup()
{
  BLEMini_begin(57600);
  Serial.begin(57600);
  
  stepper.setEnablePin(ENABLE_PIN);
  stepper.setPinsInverted(false,false,true);
  stepper.enableOutputs();
  // Change these to suit your stepper if you want
  stepper.setMaxSpeed(maxSpeed);
  stepper.setAcceleration(acceleration);
  stepper.setCurrentPosition(-stepsPerRotation);
  stepper.moveTo(stepsPerRotation);
  
  pinMode(DIGITAL_OUT_PIN, OUTPUT);
}

void loop()
{
  static boolean analog_enabled = false;
  static byte old_state = LOW;
  
  // If data is ready
  while ( BLEMini_available() >= 3 )
  {
    Serial.print("Got BLE: ");
    
    // read out command and data
    byte data0 = BLEMini_read();
    byte data1 = BLEMini_read();
    byte data2 = BLEMini_read();
    
    Serial.print(data0, HEX);
    Serial.print(", ");
    Serial.print(data1, HEX);
    Serial.print(", ");
    Serial.print(data2, HEX);
    Serial.println("");
    
    if (data0 == 0x00) {
      // It's a request for stored data
      // TODO: Acceleration
      
      // Send Speed
      BLEMini_write(0x02);
      BLEMini_write(maxSpeed & 0xFF);
      BLEMini_write(maxSpeed >> 8);
      
      // Send Steps
      BLEMini_write(0x02);
      BLEMini_write(stepsPerRotation & 0xFF);
      BLEMini_write(stepsPerRotation >> 8);
      
      // Send Delay
      BLEMini_write(0x02);
      BLEMini_write(delayBetweenLoops & 0xFF);
      BLEMini_write(delayBetweenLoops >> 8);
    }
    else if (data0 == 0x01)  // Command is to control digital out pin
    {
      if (data1 == 0x01) {
        digitalWrite(DIGITAL_OUT_PIN, HIGH);
        Serial.write("high");
      } else {
        digitalWrite(DIGITAL_OUT_PIN, LOW);
        Serial.write("low");
      }
    }
    else if (data0 == 0x02)
    {
      Serial.print(data1, HEX);
      // Write byte to EEPROM
      writeToEepromIfNeeded(accelerationAddress, data1);
    }
    else if (data0 == 0x03)
    {
      Serial.print(data1, HEX);
      // Write byte to EEPROM
      writeToEepromIfNeeded(speedAddress, data1);
    }
    else if (data0 == 0x04)
    {
      Serial.print(data1, HEX);
      // Write byte to EEPROM
      writeToEepromIfNeeded(stepsAddress, data1);
    }
    else if (data0 == 0x05)
    {
      Serial.print(data1, HEX);
      // Write byte to EEPROM
      writeToEepromIfNeeded(delayAddress, data1);
    }
  }
  
  if (stepper.distanceToGo() == 0) {
    // FIXME: this will stop BLE communication
    delay(delayBetweenLoops);
    stepper.moveTo(-stepper.currentPosition());
  }
  
  // FIXME: 
  stepper.run();
}

void writeToEepromIfNeeded(int address, byte data) {
  
  byte readByte = EEPROM.read(address);
  
  if (readByte != data) {
    // Needs to be written
    EEPROM.write(address, data);
  }
}

