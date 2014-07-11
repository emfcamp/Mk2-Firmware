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

 #define MAX_BUTTON_SUBSCRIPTIONS 10



namespace buttons {
    QueueHandle_t lightPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t aPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t bPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t upPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t rightPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t downPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t leftPressQueues[MAX_BUTTON_SUBSCRIPTIONS];
    QueueHandle_t centerPressQueues[MAX_BUTTON_SUBSCRIPTIONS];

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
        if (buttons & LIGHT)  _addToArray(lightPressQueues, _queue);
        if (buttons & A)      _addToArray(aPressQueues, _queue);
        if (buttons & B)      _addToArray(bPressQueues, _queue);
        if (buttons & UP)     _addToArray(upPressQueues, _queue);
        if (buttons & DOWN)   _addToArray(downPressQueues, _queue);
        if (buttons & LEFT)   _addToArray(leftPressQueues, _queue);
        if (buttons & RIGHT)  _addToArray(rightPressQueues, _queue);
        if (buttons & CENTER) _addToArray(centerPressQueues, _queue);
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
     * Helper functions
     */

    void _addToArray(QueueHandle_t queues[], QueueHandle_t queue) {
        for (uint8_t i=0; i<MAX_BUTTON_SUBSCRIPTIONS; i++) {
            if (queues[i] == NULL) {
                queues[i] = queue;
                break;
            }
        }
    }

    void _deferButtonHandling(Button button) {
        if (readyForInput) {
            BaseType_t xHigherPriorityTaskWoken;
            xTimerPendFunctionCallFromISR(_handleButtonPress, NULL, ( uint32_t ) button, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    void _handleButtonPress(void *pvParameter1, uint32_t ulParameter2) {
        Button button = ( Button ) ulParameter2;
             if (button == LIGHT)        _overwriteAllQueues(lightPressQueues, button);
        else if (button == A)            _overwriteAllQueues(aPressQueues, button);
        else if (button == B)            _overwriteAllQueues(bPressQueues, button);
        else if (button == UP)           _overwriteAllQueues(upPressQueues, button);
        else if (button == DOWN)         _overwriteAllQueues(downPressQueues, button);
        else if (button == LEFT)         _overwriteAllQueues(leftPressQueues, button);
        else if (button == RIGHT)        _overwriteAllQueues(rightPressQueues, button);
        else if (button == CENTER)       _overwriteAllQueues(centerPressQueues, button);
    }

    void _overwriteAllQueues(QueueHandle_t queues[], Button button) {
        for (uint8_t i=0; i<MAX_BUTTON_SUBSCRIPTIONS; i++) {
            if (queues[i] != NULL) {
                xQueueOverwrite(queues[i], &button);
            }
        }
    }
}



void buttonLightPress() {
    buttons::_deferButtonHandling(buttons::LIGHT);
}

void buttonAPress() {
    buttons::_deferButtonHandling(buttons::A);
}

void buttonBPress() {
    buttons::_deferButtonHandling(buttons::B);
}

void buttonUpPress(void) {
    buttons::_deferButtonHandling(buttons::UP);
}

void buttonDownPress(void) {
    buttons::_deferButtonHandling(buttons::DOWN);
}

void buttonLeftPress(void) {
    buttons::_deferButtonHandling(buttons::LEFT);
}

void buttonRightPress(void) {
    buttons::_deferButtonHandling(buttons::RIGHT);
}

void buttonCenterPress(void) {
    buttons::_deferButtonHandling(buttons::CENTER);
}

