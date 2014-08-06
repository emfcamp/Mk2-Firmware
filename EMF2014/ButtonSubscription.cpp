/*
 TiLDA Mk2

 ButtonSubscription

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

#include "ButtonSubscription.h"
#include "DebugTask.h"
#include <FreeRTOS_ARM.h>
#include "TiLDAButtonInterrupts.h"
#include "EMF2014Config.h"

ButtonSubscription::ButtonSubscription() {
    _queue = xQueueCreate(1, sizeof(Button));
    configASSERT(_queue);
    _buttons = 0;
}

void ButtonSubscription::addButtons(int buttons) {
    _buttons = _buttons | buttons;
    addQueueToButtons(buttons, _queue);
}

Button ButtonSubscription::waitForPress(TimeInTicks ticksToWait) {
    Button button;
    if(xQueueReceive( _queue, &button, ticksToWait) == pdTRUE) {
        return button;
    }
    return NONE;
}

Button ButtonSubscription::waitForPress() {
    return waitForPress(portMAX_DELAY);
}

void ButtonSubscription::clear() {
    xQueueReset( _queue);
}



