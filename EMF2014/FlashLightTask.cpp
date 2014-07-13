/*
 TiLDA Mk2

 Flash Light Task

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
#include "FlashLightTask.h"

#include <FreeRTOS_ARM.h>

#include "ButtonSubscription.h"
#include "DebugTask.h"
#include "RGBTask.h"

// ToDo: Add all the fancy features from https://github.com/emfcamp/Mk2-Firmware/blob/master/frRGBTask/frRGBTask.ino
String FlashLightTask::getName() {
    return "FlashLight";
}

void FlashLightTask::task() {
    ButtonSubscription triggerButton(LIGHT);

    bool lightIsOn = false;
    while(true) {
        Button button = triggerButton.waitForPress(( TickType_t ) 1000);
        if (button == LIGHT) {
            lightIsOn = !lightIsOn;
            if (lightIsOn) {
                debug::log("LIGHT ON");
                _rgbTask.setColor({255, 255, 255});
            } else {
                debug::log("LIGHT OFF");
                _rgbTask.setColor({0, 0, 0});
            }
        }
    }

}
