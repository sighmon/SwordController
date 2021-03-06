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
int speedAddress = 2;
int stepsAddress = 4;
int delayAddress = 6;

// Define a stepper and the pins it will use
#define DIRECTION_PIN      6
#define ENABLE_PIN         7
#define STEP_PIN           8

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIRECTION_PIN);

int acceleration = 50;
int maxSpeed = 7;
int stepsPerRotation = 118;
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
  // Serial.print("I'm alive");

  // Read default values from EEPROM
  acceleration = readWordFromEEPROM(accelerationAddress);
  maxSpeed = readWordFromEEPROM(speedAddress);
  stepsPerRotation = readWordFromEEPROM(stepsAddress);
  delayBetweenLoops = readWordFromEEPROM(delayAddress);
  
  stepper.setEnablePin(ENABLE_PIN);
  stepper.setPinsInverted(false,false,true);
  stepper.enableOutputs();
  // Change these to suit your stepper if you want
  stepper.setMaxSpeed(maxSpeed);
  stepper.setAcceleration(acceleration/100);
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
    // read out command and data
    byte data0 = BLEMini_read();
    byte data1 = BLEMini_read();
    byte data2 = BLEMini_read();
    
    // Serial.print(data0, HEX);
    // Serial.print(", ");
    // Serial.print(data1, HEX);
    // Serial.print(", ");
    // Serial.print(data2, HEX);
    // Serial.println("");
    
    if (data0 == 0x00) {
      // It's a request for stored data

      // Send Acceleration
      BLEMini_write(0x02);
      BLEMini_write(acceleration & 0xFF);
      BLEMini_write(acceleration >> 8);
      
      // Send Speed
      BLEMini_write(0x03);
      BLEMini_write(maxSpeed & 0xFF);
      BLEMini_write(maxSpeed >> 8);
      
      // Send Steps
      BLEMini_write(0x04);
      BLEMini_write(stepsPerRotation & 0xFF);
      BLEMini_write(stepsPerRotation >> 8);
      
      // Send Delay
      BLEMini_write(0x05);
      BLEMini_write(delayBetweenLoops & 0xFF);
      BLEMini_write(delayBetweenLoops >> 8);
    }
    else if (data0 == 0x01)  // Command is to control digital out pin
    {
      if (data1 == 0x01) {
        digitalWrite(DIGITAL_OUT_PIN, HIGH);
//        Serial.write("high");
      } else {
        digitalWrite(DIGITAL_OUT_PIN, LOW);
//        Serial.write("low");
      }
    }
    else if (data0 == 0x02)  // It's the slider values for the stepper controls
    {
//      Serial.print(data1, HEX);
      // Write byte to EEPROM
      writeToEepromIfNeeded(accelerationAddress, data1);
      writeToEepromIfNeeded(accelerationAddress + 1, data2);
      acceleration = data1 + (data2 << 8);
    }
    else if (data0 == 0x03)
    {
//      Serial.print(data1, HEX);
      // Write byte to EEPROM
      writeToEepromIfNeeded(speedAddress, data1);
      writeToEepromIfNeeded(speedAddress + 1, data2);
      maxSpeed = data1 + (data2 << 8);
    }
    else if (data0 == 0x04)
    {
//      Serial.print(data1, HEX);
      // Write byte to EEPROM
      writeToEepromIfNeeded(stepsAddress, data1);
      writeToEepromIfNeeded(stepsAddress + 1, data2);
      stepsPerRotation = data1 + (data2 << 8);
    }
    else if (data0 == 0x05)
    {
//      Serial.print(data1, HEX);
      // Write byte to EEPROM
      writeToEepromIfNeeded(delayAddress, data1);
      writeToEepromIfNeeded(delayAddress + 1, data2);
      delayBetweenLoops = data1 + (data2 << 8);
    }
  }
  
  if (stepper.distanceToGo() == 0) {
    // FIXME: this will stop BLE communication
    delay(delayBetweenLoops);
    stepper.setCurrentPosition(sign(stepper.currentPosition()) * stepsPerRotation);
    stepper.setMaxSpeed(maxSpeed);
    stepper.setAcceleration(acceleration/100);
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

int sign(int n) {
  if (n >= 0) {
    return 1;
  } else {
    return -1;
  }
}

int readWordFromEEPROM(int address) {
  return (EEPROM.read(address) + (EEPROM.read(address + 1) << 8));
}

