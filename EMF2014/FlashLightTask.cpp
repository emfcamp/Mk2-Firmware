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
#include <FreeRTOS_ARM.h>

#include "ButtonTask.h"
#include "DebugTask.h"
#include "RGBTask.h"


// ToDo: Add all the fancy features from https://github.com/emfcamp/Mk2-Firmware/blob/master/frRGBTask/frRGBTask.ino

namespace flashLight {
    void task(void *pvParameters) {
        debug::log("Starting Flash Light task");
        buttons::ButtonSubscription triggerButton = buttons::ButtonSubscription(buttons::LIGHT);

        bool lightIsOn = false;
        while(true) {
            buttons::Button button = triggerButton.waitForPress(( TickType_t ) 1000);
            if (button == buttons::LIGHT) {
                lightIsOn = !lightIsOn;
                if (lightIsOn) {
                    debug::log("LIGHT ON");
                    rgb::fadeToColor({255, 255, 255});
                } else {
                    debug::log("LIGHT OFF");
                    rgb::fadeToColor({0, 0, 0});
                }
            }
        }
    }

    void initializeTask() {
        BaseType_t taskHolder;
        taskHolder = xTaskCreate(task, "flashLightTask", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
        if (taskHolder != pdPASS) {
            debug::log("Failed to create flash light task");
            while(1);
        }
    }
}
