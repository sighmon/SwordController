# Sword controller.

The art piece by [Roy Ananda](http://www.acsa.sa.edu.au/the-school/staff/academic-staff/roy-ananda/) is two swords dangling from wires. When the piece is slowly accelerated and decelerated, the swords gently touch making for a kind of sword ballet.
 
This software controls the speed and acceleration of the rotation of the swords using a recycled stepper motor controlled by the [AccelStepper](http://www.airspayce.com/mikem/arduino/AccelStepper/) Arduino library.
 
It also uses a [BLE Mini](http://redbearlab.com/blemini/) to talk to the Arduino, so that the values sent from an iOS device can be stored in EEPROM and then edited after the piece is installed.

The majority of the work was done at [Hackerspace](http://hackadl.org) by Pix & I on and around 29th June 2015.