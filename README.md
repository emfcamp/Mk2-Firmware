TiLDA v2 Firmware
=================

#### Firmware on the TiLDA v2 Badge
* To use this code you need Arduino IDE version 1.5.5 or later which can be downloaded [here](http://arduino.cc/en/Main/Software#toc3).
* The easiest way to work with this repo is to set it as the sketchbook folder in the IDE's File->Prefrences menu (restart the IDE after changing this setting).
* This way you will not have any conflicts with your existing sketchbook and library folders.
* N.B. Arduino 1.5.5 stores its preferences in a seperate location to that of the Arduino 1.0.x IDE).


* The TiLDA code base if split in 3 distinct sections:
  * The custom TiLDA board definition is kept in **hardware/emfcamp/sam/**.
  * The custom TiLDA hardware libraries are kept in **hardware/emfcamp/sam/libraries** and will only be used if a TiLDA board is selected from the _Tools->Board_ menu.
  * Third is the main firmware sketch which is kept in **EMF2014/**, this has the _.ino_, _.c[pp]_ and _.h_ files for each task.
  * There may also be other examples or test sketches either in their own directory or as examples in TiLDA library folders.


#### hardware/
* This folder contains _TiLDA Mk2_ board definition for use with the Arduino IDE (version -> 1.5.5).
* To use: copy the **emfcamp** folder to your **~/sketchbook/hardware/** folder.
* You can then select _TiLDA Mk2_ from the _Tools->Board_ menu.
* To use the **FTDI** upload port you will need to manually erase the board before uploading a new sketch (short the **Erase** pins for 1 second).

#### EMF2014/
* This is the main sketch for the EMF 2014 firmware.
* It is complied using the **TiLDA Mk2** board form the the _Tools->Board_ menu.
* void setup() and void loop() can be found in EMF2014.ino
* There are several _.c[pp]_ and _.h_ files that make up the _FreeRTOS_ tasks used by the main app.


#### frRGBTask/
* This is the first fully fuctional test of the RGB Task.
* Some of the buttons are set up to test the different RGB modes.
