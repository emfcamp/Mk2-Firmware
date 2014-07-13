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
#include "DebugTask.h"
#include <FreeRTOS_ARM.h>
#include "RGBTask.h"

/** 
 * Button Task class
 */
ButtonTask *ButtonTask::mainButtonTask = NULL;
ButtonTask::ButtonTask() {
    ButtonTask::mainButtonTask = this;
}

String ButtonTask::getName() {
    return "ButtonTask";
}

void ButtonTask::task() {
    while(true) {
        // ToDo: Add some activity tracking in here for backlight etc..
        vTaskDelay(1000);
    }
}

void ButtonTask::deferButtonHandling(Button button) {
    /*
    BaseType_t xHigherPriorityTaskWoken;
    xTimerPendFunctionCallFromISR(_handleButtonPressHelper, NULL, ( uint32_t ) button, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);*/
}

void ButtonTask::_handleButtonPressHelper(void *pvParameter1, uint32_t ulParameter2) {
    if (mainButtonTask != NULL) {
        Button button = ( Button ) ulParameter2;
        mainButtonTask->_handleButtonPress(button);
    }
}

void ButtonTask::_handleButtonPress(Button button) {
         if (button == LIGHT)        _overwriteAllQueues(lightPressQueues, button);
    else if (button == A)            _overwriteAllQueues(aPressQueues, button);
    else if (button == B)            _overwriteAllQueues(bPressQueues, button);
    else if (button == UP)           _overwriteAllQueues(upPressQueues, button);
    else if (button == DOWN)         _overwriteAllQueues(downPressQueues, button);
    else if (button == LEFT)         _overwriteAllQueues(leftPressQueues, button);
    else if (button == RIGHT)        _overwriteAllQueues(rightPressQueues, button);
    else if (button == CENTER)       _overwriteAllQueues(centerPressQueues, button);
}

void ButtonTask::_overwriteAllQueues(QueueHandle_t queues[], Button button) {
    for (uint8_t i=0; i<MAX_BUTTON_SUBSCRIPTIONS; i++) {
        if (queues[i] != NULL) {
            xQueueOverwrite(queues[i], &button);
        }
    }
}
    
/**
 * ButtonSubscription class
 */
ButtonSubscription::ButtonSubscription(int buttons, ButtonTask buttonTask) {
    _buttons = buttons;
    _queue = xQueueCreate(1, sizeof(Button));
    configASSERT(_queue);
    if (buttons & LIGHT)  _addToArray(buttonTask.lightPressQueues, _queue);
    if (buttons & A)      _addToArray(buttonTask.aPressQueues, _queue);
    if (buttons & B)      _addToArray(buttonTask.bPressQueues, _queue);
    if (buttons & UP)     _addToArray(buttonTask.upPressQueues, _queue);
    if (buttons & DOWN)   _addToArray(buttonTask.downPressQueues, _queue);
    if (buttons & LEFT)   _addToArray(buttonTask.leftPressQueues, _queue);
    if (buttons & RIGHT)  _addToArray(buttonTask.rightPressQueues, _queue);
    if (buttons & CENTER) _addToArray(buttonTask.centerPressQueues, _queue);
    //debug::log("queue created with " + String(buttons));
}

Button ButtonSubscription::waitForPress(TickType_t ticksToWait) {
    Button button;
    debug::log("Wait for press");
    _queue;
    debug::log("Wait for press2");
    if(xQueueReceive( _queue, &button, ticksToWait) == pdTRUE) {
        return button;
    }
    return NONE;
}

Button ButtonSubscription::waitForPress() {
    return ButtonSubscription::waitForPress(portMAX_DELAY);
}

void ButtonSubscription::clear() {
    xQueueReset( _queue);
}

void ButtonSubscription::_addToArray(QueueHandle_t queues[], QueueHandle_t queue) {
    for (uint8_t i=0; i<MAX_BUTTON_SUBSCRIPTIONS; i++) {
        if (queues[i] == NULL) {
            queues[i] = queue;
            break;
        }
    }
}

/**
 * Interrupts
 */

void buttonLightPress() {
    ButtonTask::deferButtonHandling(LIGHT);
}

void buttonAPress() {
    ButtonTask::deferButtonHandling(A);
}

void buttonBPress() {
    ButtonTask::deferButtonHandling(B);
}

void buttonUpPress(void) {
    ButtonTask::deferButtonHandling(UP);
}

void buttonDownPress(void) {
    ButtonTask::deferButtonHandling(DOWN);
}

void buttonLeftPress(void) {
    ButtonTask::deferButtonHandling(LEFT);
}

void buttonRightPress(void) {
    ButtonTask::deferButtonHandling(RIGHT);
}

void buttonCenterPress(void) {
    ButtonTask::deferButtonHandling(CENTER);
}

