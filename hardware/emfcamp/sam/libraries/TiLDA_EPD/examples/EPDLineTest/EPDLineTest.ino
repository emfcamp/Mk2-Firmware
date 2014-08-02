/*
 TiLDA Mk2 epaper screen test display
 
 Switch two images on button press
 Includes Due debunce
 
 
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
#include <SPI.h>
#include <TiLDA_EPD.h>
#include <Images.h>

#include <limits.h>

#include <lwk.xbm>
#include <menu.xbm>

#define IMAGE_A 1
#define IMAGE_A_BITS lwk_bits
#define IMAGE_B 2
#define IMAGE_B_BITS menu_bits

EPD_Class EPD(EPD_2_7, EPD_PANEL_ON, EPD_BORDER_CONTROL, EPD_DISCHARGE, EPD_PWM, EPD_RESET, EPD_BUSY, 52);

int currentImage = 0;
int setImage = 0;

void setup() {
    // PMIC to CHARGE
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);
    pinMode(SRF_SLEEP, OUTPUT);
    digitalWrite(SRF_SLEEP, LOW);
    
    Serial.begin(115200);
    delay(500);
    Serial.println("TiLDA EPD Line tests");
    Serial.println("Pin setup");

    EPD.pinSetup();
    
    // set button pins and attach interrupts
    // dose not matter if we do not define all button interupt handlers
    tildaButtonSetup();
    tildaButtonAttachInterrupts();
    
    Serial.println("Clear Screen");
    // clear the screen
    int temperature = 24;   // fake it unitil we have either MPU or SAM3X temp working
    EPD.begin();
    EPD.setFactor(temperature);
    EPD.clear();
    EPD.end();
    Serial.println("Start Loop");
}

void loop() {
    
    if (setImage != currentImage){
        Serial.print("Button Pressed: ");
        // lets update the screen
        int temperature = 24;   // fake it unitil we have either MPU or SAM3X temp working
        EPD.begin(); // power up the EPD panel
        EPD.setFactor(temperature); // adjust for current temperature
        int linedelay = 10;
        int count = 0;
        switch (setImage) {
            case IMAGE_A:
                Serial.println("A");
                EPD.image(IMAGE_A_BITS);
                currentImage = IMAGE_A;
                break;
            case IMAGE_B:
                Serial.println("B");
                for( int line = 10; line < 11; line ++){
                  long stage_time = 630;
                  do {
                      unsigned long t_start = millis();
                      ++count;
                      EPD.line(line-1, 0, 0XAA, NULL, false, EPD_normal);
                      EPD.line(line, 0, 0xFF, NULL, false, EPD_normal);
                      EPD.line(line+1, 0, 0xAA, NULL, false, EPD_normal);
                      unsigned long t_end = millis();
            	      if (t_end > t_start) {
            		  stage_time -= t_end - t_start;
            	      } else {
            		  stage_time -= t_start - t_end + 1 + ULONG_MAX;
            	      }
            	   } while (stage_time > 0);
                }
                currentImage = IMAGE_B;
                Serial.println(count);
                break;
            
            default:
            break;
        }
        
        EPD.end();   // power down the EPD panel
    }
    
}



void buttonAPress(){
    setImage = IMAGE_A;
}

void buttonBPress(){
    setImage = IMAGE_B;
}
