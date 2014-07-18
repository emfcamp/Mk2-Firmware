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
        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
            if (xSemaphoreTake(serialPortMutex, ( TickType_t ) 10) == pdTRUE ) {
                DEBUG_SERIAL.println(text);
            }
            xSemaphoreGive(serialPortMutex);
        } else {
            DEBUG_SERIAL.println(text);
        }
    }

    void logByteArray(byte in[], int len) {
        // ToDo: Add other debug outputs
        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
            if (xSemaphoreTake(serialPortMutex, ( TickType_t ) 10) == pdTRUE ) {
                //DEBUG_SERIAL.print(String((char*)in));
                int i;
                for (i=0; i<len; i++) {
                    DEBUG_SERIAL.print("0123456789abcdef"[in[i]>>4]);
                    DEBUG_SERIAL.print("0123456789abcdef"[in[i]&0xf]);
                    DEBUG_SERIAL.print(" ");
                }
                DEBUG_SERIAL.println();
            }
            xSemaphoreGive(serialPortMutex);
        } else {
            DEBUG_SERIAL.println("Can't print hash, scheduler not running.");
        }
    }

    void logFromISR(String text) {
        // ToDo: Add other debug outputs
        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
            if (xSemaphoreTakeFromISR(serialPortMutex, NULL) == pdTRUE) {
                DEBUG_SERIAL.println(text);
                BaseType_t xHigherPriorityTaskWoken;
                xSemaphoreGiveFromISR(serialPortMutex, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        } else {
            DEBUG_SERIAL.println(text);
        }
    }

    /**
     * Lights the debug led and waits for a key to be pressed in 
     * the serial console before sending the text
     */
    void stopWithMessage(String text) {
        digitalWrite(DEBUG_LED, HIGH);
        waitForKey();
        DEBUG_SERIAL.println(text);
        while(true);
    }

     void waitForKey() {
        while (!DEBUG_SERIAL.available());
    }

    void setup() {
        DEBUG_SERIAL.begin(115200);
        delay(250);

        pinMode(DEBUG_LED, OUTPUT);       
        digitalWrite(DEBUG_LED, LOW);

        serialPortMutex = xSemaphoreCreateMutex();
    }
}

String DebugTask::getName() {
    return "DebugTask";
}

void DebugTask::task() {
    while(true) {
        // Not sure what to do here
        debug::log("Still alive.");
        vTaskDelay( 50000 );
    }
}

// I'm not even sure if this works...
void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName ) {
    DEBUG_SERIAL.println("Stack Overflow");
}



