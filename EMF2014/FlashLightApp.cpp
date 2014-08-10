/*
 TiLDA Mk2

 Flash Light App

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
#include "FlashLightApp.h"

#include <FreeRTOS_ARM.h>

#include "ButtonSubscription.h"
#include "DebugTask.h"
#include "RGBTask.h"
#include "AppManager.h"
#include "glcd.h"

// ToDo: Add all the fancy features from https://github.com/emfcamp/Mk2-Firmware/blob/master/frRGBTask/frRGBTask.ino
String FlashLightApp::getName() {
    return "FlashLight";
}

void FlashLightApp::updateLeds() {
    uint8_t actualLightLevel = 1 << _lightLevel;
    if (_lightLevel == 8) {
        actualLightLevel = 255;
    }
    _rgbTask.setColor({actualLightLevel, actualLightLevel, actualLightLevel});
}

void FlashLightApp::task() {
    ButtonSubscription _buttons;
    _pbuttons =& _buttons;
    _buttons.addButtons(UP | DOWN);
    updateLeds();
    while(true) {
        Button button = _buttons.waitForPress(( TickType_t ) 1000);
        if (button == UP) {
            if (_lightLevel < 8) {
                _lightLevel++;
            } else {
                _lightLevel = 8;
            }
            updateLeds();
        } else if (button == DOWN) {
            if (_lightLevel > 1) {
                _lightLevel--;
            } else {
                _lightLevel = 0;
            }
            updateLeds();
        }
    }
}

void FlashLightApp::afterSuspension() {
    _rgbTask.setColor({0, 0, 0});
}

void FlashLightApp::beforeResume() {
    if (_pbuttons) {
        // Clear Button Queue of any up/down button presse that have occured durind suspension
        _pbuttons->clear();
    }
    updateLeds();
}
