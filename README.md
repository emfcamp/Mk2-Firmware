Firmware
========

Firmware that runs on the TiLDA v2 Badge
To use this code you need Arduino IDE version 1.5.5 or later
The eaiest way to work with this repo is to set it as the skecthbook folder in the 
File->Prefrences menu (restart the IDE after changeing this setting)
That way you will not get any conflicts with your existing sketchbook and libraries folders
(Note Arduino 1.5.5 or later IDE use a diffrent prefrence file to Arduino 1.0.x IDE's so you can keep 1.0 pointing to your existing skecthbook)

The TiLDA code base if split in 3 distinct sections, first the custom TiLDA board definition kept in the hardware/emfcamp/sam/ folder. Second the custom TiLDA hardware libreries, these are in hardware/emfcamp/sam/libraries and will only be used if a TiLDA board is selected from the Tools->Board menu. Thrid is the main firmware sketch, this is in the EMF2014/ folder, this has the base .ino and the .c[pp] & .h files for each task
There may also be other example or test skecths either in there own director or as examples under the TiLDA specfic libraries folders


hardware/ 
This folder contains TiLDA Mk2 board definition for use with the Arduino IDE (version -> 1.5.5) 
to use copy the emfcamp folder to your ~/Sketchbook/hardware/ folder
You can then select "TiLDA Mk2" from the Tools->Board menu
To use the (FTDI) upload port you will need to manually erase the board before uploading a new sketch, short the Erase pins for 1 second

EMF2014/
This is the main sketch for the EFM 2014 firmware.
It is complies using the "TiLDA Mk2" board form the the Tools->Board menu
The code enters via the EMF2014.ino like any other Arduino skecth. 
There a sevral .c[pp] & .h files that make up the FreeRTOS tasks used by the main app


frRGBTask/
This was the first task worked on and is a full functional test of just the RGB task.
Some of the buttons are setup to test the diffrent RGB modes.
