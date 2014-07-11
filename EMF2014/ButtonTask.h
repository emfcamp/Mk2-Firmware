/*
 TiLDA Mk2

 ButtonTask
 Buttons are handled via interrupt callbacks

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

#ifndef _BUTTON_TASK_H_
#define _BUTTON_TASK_H_

#include <Arduino.h>
#include <FreeRTOS_ARM.h>
#include "EMF2014Config.h"

void buttonLightPress();

namespace buttons {
    enum Button {
        NONE         = 0,
        LIGHT        = 1,
        A            = 2,
        B            = 4,
        UP           = 8,
        DOWN         = 16,
        LEFT         = 32,
        RIGHT        = 64,
        CENTER       = 128
    };

    class ButtonSubscription {
    public:
        ButtonSubscription(int buttons);
        Button waitForPress(TickType_t ticksToWait);
        Button waitForPress();
        void clear();
    private:
        int _buttons;
        QueueHandle_t _queue;
    };

    void initializeTask();

    /** 
     * Helper functions
     */
    void _addToArray(QueueHandle_t queues[], QueueHandle_t queue);
    void _deferButtonHandling(Button button);
    void _handleButtonPress(void *pvParameter1, uint32_t ulParameter2);
    void _overwriteAllQueues(QueueHandle_t queues[], Button button);
}

#endif // _BUTTON_TASK_H_