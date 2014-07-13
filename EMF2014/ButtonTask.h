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
#include "Task.h"

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

class ButtonTask: public Task {
friend class ButtonSubscription;
public:
    ButtonTask();
    String getName();
    static void deferButtonHandling(Button button);
    //void setUpButtonInterrupts();
protected:
    void task();
private:
    static void _handleButtonPressHelper(void *pvParameter1, uint32_t ulParameter2);
    void _handleButtonPress(Button button);
    void _overwriteAllQueues(QueueHandle_t queues[], Button button);

    QueueHandle_t lightPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t aPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t bPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t upPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t rightPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t downPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t leftPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t centerPressQueues[MAX_BUTTON_SUBSCRIPTIONS];

    //bool readyToHandleButtonPress;
    static ButtonTask *mainButtonTask;
};

class ButtonSubscription {
public:
    ButtonSubscription (int buttons, ButtonTask buttonTask);
    Button waitForPress(TickType_t ticksToWait);
    Button waitForPress();
    void clear();
private:
    void _addToArray(QueueHandle_t queues[], QueueHandle_t queue);

    int _buttons;
    QueueHandle_t _queue;
};

void setUpButtonInterrupts(ButtonTask buttonTask);

#endif // _BUTTON_TASK_H_