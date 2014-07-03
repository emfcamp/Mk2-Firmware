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
#include "LinkedList.h"


/*
 ToDo: Remove me
 Just as a reminder for wiring up a non-prototype
#define BUTTON_LIGHT          (43u) // should be 60 but seems I forgot to swap the trace with SRF_AT_COMMAND
#define BUTTON_SCREEN_RIGHT   (46u)
#define BUTTON_SCREEN_LEFT    (29u)
#define BUTTON_A              (47u)
#define BUTTON_B              (48u)
#define BUTTON_UP             (27u)
#define BUTTON_RIGHT          (25u)
#define BUTTON_DOWN           (26u)
#define BUTTON_LEFT           (28u)
#define BUTTON_CENTER         (24u)
*/

namespace buttons {
    LinkedList<QueueHandle_t> lightPressQueues;
    LinkedList<QueueHandle_t> screenLeftPressQueues;
    LinkedList<QueueHandle_t> screenRightPressQueues;
    LinkedList<QueueHandle_t> aPressQueues;
    LinkedList<QueueHandle_t> bPressQueues;
    LinkedList<QueueHandle_t> upPressQueues;
    LinkedList<QueueHandle_t> rightPressQueues;
    LinkedList<QueueHandle_t> downPressQueues;
    LinkedList<QueueHandle_t> leftPressQueues;
    LinkedList<QueueHandle_t> centerPressQueues;

    // A flag that determines whether the task is ready to receive and handle button presses
    boolean readyForInput = false;

    /**
     * Allows other tasks to subscribe to a set of buttons and
     * wait for them to be pressed
     */
    ButtonSubscription::ButtonSubscription(int buttons) {
        _buttons = buttons;
        _queue = xQueueCreate(1, sizeof(Button));
        configASSERT(_queue);
        if (buttons & LIGHT)         lightPressQueues.add(_queue);
        if (buttons & SCREEN_LEFT)   screenLeftPressQueues.add(_queue);
        if (buttons & SCREEN_RIGHT)  screenRightPressQueues.add(_queue);
        if (buttons & A)             aPressQueues.add(_queue);
        if (buttons & B)             bPressQueues.add(_queue);
        if (buttons & UP)            upPressQueues.add(_queue);
        if (buttons & DOWN)          downPressQueues.add(_queue);
        if (buttons & LEFT)          leftPressQueues.add(_queue);
        if (buttons & RIGHT)         rightPressQueues.add(_queue);
        if (buttons & CENTER)        centerPressQueues.add(_queue);
    }

    Button ButtonSubscription::waitForPress(TickType_t ticksToWait) {
        Button button;
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

    void task(void *pvParameters) {
        debug::log("Starting Button task");

        readyForInput = true;
        while(true) {
            // ToDo: Add some activity tracking in here for backlight etc..
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    void initializeTask() {
        BaseType_t taskHolder;
        taskHolder = xTaskCreate(task, "buttonTask", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
        if (taskHolder != pdPASS) {
            debug::log("Failed to create button task");
            while(1);
        }
    }

    /**
     * These are helper functions for interrupts. Their job is to defer button presses
     * to a timer task and fill the right queues from there
     */
    void overwriteAllQueues(LinkedList<QueueHandle_t> &queues, Button button) {
        for (int i = 0; i < queues.size(); i++) {
            xQueueOverwrite(queues.get(i), &button);
        }
    }

    void handleButtonPress(void *pvParameter1, uint32_t ulParameter2) {
        Button button = ( Button ) ulParameter2;
             if (button == LIGHT)        overwriteAllQueues(lightPressQueues, button);
        else if (button == SCREEN_LEFT)  overwriteAllQueues(screenLeftPressQueues, button);
        else if (button == SCREEN_RIGHT) overwriteAllQueues(screenRightPressQueues, button);
        else if (button == A)            overwriteAllQueues(aPressQueues, button);
        else if (button == B)            overwriteAllQueues(bPressQueues, button);
        else if (button == UP)           overwriteAllQueues(upPressQueues, button);
        else if (button == DOWN)         overwriteAllQueues(downPressQueues, button);
        else if (button == LEFT)         overwriteAllQueues(leftPressQueues, button);
        else if (button == RIGHT)        overwriteAllQueues(rightPressQueues, button);
        else if (button == CENTER)       overwriteAllQueues(centerPressQueues, button);
    }

    void deferButtonHandling(Button button) {
        if (readyForInput) {
            BaseType_t xHigherPriorityTaskWoken;
            xTimerPendFunctionCallFromISR(handleButtonPress, NULL, ( uint32_t ) button, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}



void buttonLightPress() {
    buttons::deferButtonHandling(buttons::LIGHT);
}

void buttonScreenLeftPress(void) {
    buttons::deferButtonHandling(buttons::SCREEN_LEFT);
}

void buttonScreenRightPress(void) {
    buttons::deferButtonHandling(buttons::SCREEN_RIGHT);
}

void buttonAPress() {
    buttons::deferButtonHandling(buttons::A);
}

void buttonBPress() {
    buttons::deferButtonHandling(buttons::B);
}

void buttonUpPress(void) {
    buttons::deferButtonHandling(buttons::UP);
}

void buttonDownPress(void) {
    buttons::deferButtonHandling(buttons::DOWN);
}

void buttonLeftPress(void) {
    buttons::deferButtonHandling(buttons::LEFT);
}

void buttonRightPress(void) {
    buttons::deferButtonHandling(buttons::RIGHT);
}

void buttonCenterPress(void) {
    buttons::deferButtonHandling(buttons::CENTER);
}