/*
 TiLDA Mk2

 RGBTask

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

#include "RGBTask.h"

#include <FreeRTOS_ARM.h>
#include <debug.h>

RGBTask::RGBTask() {
}

void RGBTask::setColor(RGBLed led, RGBColor color) {
    if (led == LED1 || led == BOTH) {
        analogWrite(LED1_RED, color.red);
        analogWrite(LED1_GREEN, color.green);
        analogWrite(LED1_BLUE, color.blue);
    }
    if (led == LED2 || led == BOTH) {
        analogWrite(LED2_RED, color.red);
        analogWrite(LED2_GREEN, color.green);
        analogWrite(LED2_BLUE, color.blue);
    }
}

void RGBTask::setColor(RGBColor color) {
    setColor(BOTH, color);
}

String RGBTask::getName() const {
    return "RGBTask";
}

void RGBTask::task() {
    while(true) {
        // ToDo: Do something here that turn the led of when inactivity occurs 
        // ToDo: Add some logic for blinking, fading, sending morse code, ...
        vTaskDelay((1000/portTICK_PERIOD_MS));
    }
}
