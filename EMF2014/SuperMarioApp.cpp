/*
 TiLDA Mk2

 Task

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

#include <FreeRTOS_ARM.h>
#include <debug.h>
#include "SuperMarioApp.h"
#include "Tilda.h"
#include <glcd.h>
#include <M2tk.h>
#include "GUITask.h"
#include <fonts/allFonts.h>
#include "pitches.h"

/* Mario main theme melody */
const uint16_t melody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
  NOTE_G6, 0, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,
  NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
  NOTE_AS3, NOTE_AS4, 0,
  0,
  NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
  NOTE_AS3, NOTE_AS4, 0,
  0,
  NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
  NOTE_DS3, NOTE_DS4, 0,
  0,
  NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
  NOTE_DS3, NOTE_DS4, 0,
  0, NOTE_DS4, NOTE_CS4, NOTE_D4,
  NOTE_CS4, NOTE_DS4,
  NOTE_DS4, NOTE_GS3,
  NOTE_G3, NOTE_CS4,
  NOTE_C4, NOTE_FS4, NOTE_F4, NOTE_E3, NOTE_AS4, NOTE_A4,
  NOTE_GS4, NOTE_DS4, NOTE_B3,
  NOTE_AS3, NOTE_A3, NOTE_GS3,
  0, 0, 0
};

/* Mario main theme tempo */
const uint16_t tempo[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  6, 18, 18, 18,
  6, 6,
  6, 6,
  6, 6,
  18, 18, 18, 18, 18, 18,
  10, 10, 10,
  10, 10, 10,
  3, 3, 3
};

// Use this as a boilerplate app class. If you create a new one, make sure to also register it in the AppManager
// otherwise it won't show up on the HomeScreen

App* SuperMarioApp::New() {
    return new SuperMarioApp;
}

SuperMarioApp::SuperMarioApp() {
    mButtonSubscription = Tilda::createButtonSubscription(UP | DOWN | LEFT | RIGHT); // Define the buttons you're interested in here
}

SuperMarioApp::~SuperMarioApp() {
    delete mButtonSubscription;
}

String SuperMarioApp::getName() const {
    return "SuperMarioApp";
}

bool SuperMarioApp::keepAlive() const {
        return false; // Should this app be suspended or killed when another app is started?
}

void SuperMarioApp::task() {
    // Do some setup in here
    Tilda::getGUITask().clearRoot(); // Clean screen

    GLCD.SetRotation(ROTATION_180);   // Orientation

    GLCD.SelectFont(System5x7);      // Font
    Tilda::setLedColor({0, 0, 0});   // LEDs off

    int x = 32;
    int y = 64;
    int step = 8;
    GLCD.DrawBitmap(SUPER_MARIO_TEST_XBM, y, x);
    uint16_t length = sizeof(melody) / sizeof(uint16_t);
    Tilda::playMelody(melody, tempo, length);

    while(true) { // Make sure this loop goes on forever     
        Button button = mButtonSubscription->waitForPress(1000); // Wait for up to a second for a button press
        // Make sure all the buttons you're interested in are defined in the constructor
        if (button == UP) {
            Tilda::getGUITask().clearRoot();
            y -= step;
            GLCD.DrawBitmap(SUPER_MARIO_TEST_XBM, y, x);
            //Tilda::setLedColor({255, 0, 0}); // red
        } else if (button == DOWN) {
            Tilda::getGUITask().clearRoot();
            y += step;
            GLCD.DrawBitmap(SUPER_MARIO_TEST_XBM, y, x);
            //Tilda::setLedColor({0, 255, 0}); // green
        } else if (button == LEFT) {
            Tilda::getGUITask().clearRoot();
            x += step;
            GLCD.DrawBitmap(SUPER_MARIO_TEST_XBM, y, x);
            //Tilda::setLedColor({0, 0, 255}); // blue
        } else if (button == RIGHT) {
            Tilda::getGUITask().clearRoot();
            x -= step;
            GLCD.DrawBitmap(SUPER_MARIO_TEST_XBM, y, x);
            //Tilda::setLedColor({255, 255, 255}); // white
        } else {
            //Tilda::setLedColor({255, 255, 255});   // LEDs off
        }
    }
}

void SuperMarioApp::afterSuspension() {} // If keepAlive() is true this is being called when the task is suspended
void SuperMarioApp::beforeResume() {} // If keepAlive() is true this is being called when the task is resumed
