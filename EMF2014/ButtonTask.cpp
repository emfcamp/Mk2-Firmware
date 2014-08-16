/*
 TiLDA Mk2

 ButtonTask

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

#include "ButtonTask.h"
#include "debug.h"
#include <FreeRTOS_ARM.h>
#include "ButtonSubscription.h"
#include "Tilda.h"

/**
 * Button Task class
 */
String ButtonTask::getName() const {
    return "ButtonTask";
}

void ButtonTask::task() {
    ButtonSubscription allButtons = Tilda::createButtonSubscription(LIGHT | A | B | UP | DOWN | LEFT | RIGHT | CENTER);

    while(true) {
        Button button = allButtons.waitForPress(( TickType_t ) 1000);
        if (button != NONE) {
            // ToDo: Add some activity tracking in here for backlight etc..
            debug::log("Button pressed: " + String(button));
        }

        vTaskDelay((1000/portTICK_PERIOD_MS));
    }
}



