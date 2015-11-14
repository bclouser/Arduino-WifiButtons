
# Wifi Buttons 
 A simple avr application which uses an esp8266 wifi chip to send mqtt messages when a button is pressed.

## Building 
 - Arduino-Makefile is a submodule of this repo so run "git submodule init", and "git submodule update" first.
 - You will then need to configure the Arduino-Makefile/Arduino.mk file to point to the right folders
 -- Just try running the makefile first, it is pretty smart and might just figure it out.
 - You will need to install the arduino libraries espduino, and SoftwareSerial (if it isn't already installed)
 -- NOTE: If you clone the he espduino git repo, the actual espduino library folder is one directory deep, so you have to actually move it out in order for arduino to see it.



