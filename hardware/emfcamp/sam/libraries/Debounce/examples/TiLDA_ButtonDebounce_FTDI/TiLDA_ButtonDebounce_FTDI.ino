/*
 TiLDA Mk2 button debounce tester (SRF/FTDI)
 
 Simple script to test button debounce and report preses over serial 
 
 This sekcth uses the SRF/FTDI serial port

 Debounce helper based on information form 
 http://forum.arduino.cc/index.php?topic=156474.0
 
 
 The MIT License (MIT)
 
 Copyright (c) 2014 Electromagnetic Field LTD
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <Debounce.h>

// intrupt flags
boolean buttonLight = 0;
boolean buttonScreenLeft = 0;
boolean buttonScreenRight = 0;
boolean buttonA = 0;
boolean buttonB = 0;
boolean buttonUp = 0;
boolean buttonRight = 0;
boolean buttonDown = 0;
boolean buttonLeft = 0;
boolean buttonCenter = 0;

void setup() {
    
    // PMIC to CHARGE
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);
    
    // set button pins and attach interrupts
    // dose not matter if we do not define all button interupt handlers
    tildaButtonSetup();
    tildaButtonAttachInterrupts();
    
    // setup LED's
    
    // setup  Serial
    Serial.begin(115200);
    delay(250);
    Serial.println("TiLDA Mk2 button debounce tester");
}

void loop() {
    if (buttonLight) {
        Serial.println("Light Pressed");
        buttonLight = 0;
    }
    if (buttonScreenLeft) {
        Serial.println("Screen Left Pressed");
        buttonScreenLeft = 0;
    }
    if (buttonScreenRight) {
        Serial.println("Screen Right Pressed");
        buttonScreenRight = 0;
    }
    if (buttonA) {
        Serial.println("A Pressed");
        buttonA = 0;
    }
    if (buttonB) {
        Serial.println("B Pressed");
        buttonB = 0;
    }
    if (buttonUp) {
        Serial.println("Up Pressed");
        buttonUp = 0;
    }
    if (buttonRight) {
        Serial.println("Right Pressed");
        buttonRight = 0;
    }
    if (buttonDown) {
        Serial.println("Down Pressed");
        buttonDown = 0;
    }
    if (buttonLeft) {
        Serial.println("Left Pressed");
        buttonLeft = 0;
    }
    if (buttonCenter) {
        Serial.println("Center Pressed");
        buttonCenter = 0;
    }

}

void buttonLightPress(){
    buttonLight = 1;
}

void buttonScreenLeftPress(){
    buttonScreenLeft = 1;
}

void buttonScreenRightPress(){
    buttonScreenRight = 1;
}

void buttonAPress(){
    buttonA = 1;
}

void buttonBPress(){
    buttonB = 1;
}

void buttonUpPress(){
    buttonUp = 1;
}

void buttonRightPress(){
    buttonRight = 1;
}

void buttonDownPress(){
    buttonDown = 1;
}

void buttonLeftPress(){
    buttonLeft = 1;
}

void buttonCenterPress(){
    buttonCenter = 1;
}
