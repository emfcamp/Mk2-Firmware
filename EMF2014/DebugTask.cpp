/*
 TiLDA Mk2

 DebugTask

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

#include "DebugTask.h"
#include <FreeRTOS_ARM.h>



namespace debug {
    // I tried to write this with a queue, but the C++ pointer gods
    // didn't approve of me, so I went for this option for now.
    static SemaphoreHandle_t serialPortMutex;

    void log(String text) {
        // ToDo: Add other debug outputs
        if (xSemaphoreTake(serialPortMutex, ( TickType_t ) 10) == pdTRUE ) {
            SerialUSB.println(text);
        }
        xSemaphoreGive(serialPortMutex);
    }

    void logFromISR(String text) {
        // ToDo: Add other debug outputs
        if (xSemaphoreTakeFromISR(serialPortMutex, NULL) == pdTRUE) {
            SerialUSB.println(text);
            BaseType_t xHigherPriorityTaskWoken;
            xSemaphoreGiveFromISR(serialPortMutex, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    void setupSerialPort() {
        SerialUSB.begin(115200);
        delay(250);

        serialPortMutex = xSemaphoreCreateMutex();
    }

    void task(void *pvParameters) {
        debug::log("Starting debugger task");

        while(true) {
            // Not sure what to do here
            vTaskDelay( 1000 );
        }
    }


    void initializeTask() {
        debug::setupSerialPort();

        BaseType_t taskHolder;
        taskHolder = xTaskCreate(task, "debugTask", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
        if (taskHolder != pdPASS) {
            debug::log("Failed to create debugger task");
            while(1);
        }
    }
}

