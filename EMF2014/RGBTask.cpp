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

 /*
 ToDo: Remove me
 Just as a reminder for wiring up a non-prototype
#define LED1_RED              (37u)
#define LED1_GREEN            (39u)
#define LED1_BLUE             (41u)
#define LED2_RED              (82u)
#define LED2_GREEN            (44u)
#define LED2_BLUE             (45u)
*/

#include "DebugTask.h"
#include <string>
#include <FreeRTOS_ARM.h>

namespace rgb {
    #define INITIAL_COLOR (0, 0, 0)
    #define COLOR_CHANGE_PER_FRAME 3
    #define FRAME_RATE_CHANGE 10 / portTICK_PERIOD_MS
    #define FRAME_RATE_SLEEP 100 / portTICK_PERIOD_MS

    /**
     * Color class
     */

    Color::Color (uint8_t setRed , uint8_t setGreen, uint8_t setBlue) {
        red = setRed;
        green = setGreen;
        blue = setBlue;
    }

    bool _moveTowards(uint8_t &current, uint8_t target, uint8_t step) {
        if (current == target) {
            return false;
        }
        if (current < target) {
            if (target - current <= step) {
                current = target;
            } else {
                current += step;
            }
        } else {
            if (current - target <= step) {
                current = target;
            } else {
                current -= step;
            }
        }
        return true;
    }

    bool Color::moveTowards(Color target, uint8_t step) {
        bool redHasChanged   = _moveTowards(red,   target.red,   step);
        bool greenHasChanged = _moveTowards(green, target.green, step);
        bool blueHasChanged  = _moveTowards(blue,  target.blue,  step);
        return redHasChanged || greenHasChanged || blueHasChanged;
    }

    /**
     * Public functions
     */

    Color currentColor1 INITIAL_COLOR;
    Color currentColor2 INITIAL_COLOR;
    Color targetColor1 INITIAL_COLOR;
    Color targetColor2 INITIAL_COLOR;

    void fadeToColor(rgb::Led led, rgb::Color color) {
        if (led == LED1 || led == BOTH) {
            targetColor1 = color;
        }
        if (led == LED2 || led == BOTH) {
            targetColor2 = color;
        }
    }

    void fadeToColor(rgb::Color color) {
        rgb::fadeToColor(BOTH, color);
    }

    void setColor(rgb::Led led, rgb::Color color) {
        if (led == LED1 || led == BOTH) {
            targetColor1 = color;
            currentColor1 = color;
            analogWrite(LED1_RED, 255 - currentColor1.red);
            analogWrite(LED1_GREEN, 255 - currentColor1.green);
            analogWrite(LED1_BLUE, 255 - currentColor1.blue);
        }
        if (led == LED2 || led == BOTH) {
            targetColor2 = color;
            currentColor2 = color;
            analogWrite(LED2_RED, 255 - currentColor2.red);
            analogWrite(LED2_GREEN, 255 - currentColor2.green);
            analogWrite(LED2_BLUE, 255 - currentColor2.blue);
        }
    }

    void setColor(rgb::Color color) {
        rgb::setColor(BOTH, color);
    }

    /**
     * Task management
     */

    void task(void *pvParameters) {
        debug::log("Starting RGB task");
        uint8_t state = 0;
        while(true) {
            // This task is used for fading colors

            bool hasLed1Changed = rgb::currentColor1.moveTowards(rgb::targetColor1, COLOR_CHANGE_PER_FRAME);
            if (hasLed1Changed) {
                analogWrite(LED1_RED, 255 - currentColor1.red);
                analogWrite(LED1_GREEN, 255 - currentColor1.green);
                analogWrite(LED1_BLUE, 255 - currentColor1.blue);
            }

            bool hasLed2Changed = rgb::currentColor2.moveTowards(rgb::targetColor2, COLOR_CHANGE_PER_FRAME);
            if (hasLed2Changed) {
                analogWrite(LED2_RED, 255 - currentColor2.red);
                analogWrite(LED2_GREEN, 255 - currentColor2.green);
                analogWrite(LED2_BLUE, 255 - currentColor2.blue);
            }

            if (hasLed1Changed || hasLed2Changed) {
                vTaskDelay(FRAME_RATE_CHANGE);
            } else {
                vTaskDelay(FRAME_RATE_SLEEP);
            }
        }
    }

    void initializeTask() {
        BaseType_t taskHolder;
        taskHolder = xTaskCreate(task, "rgbTask", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
        if (taskHolder != pdPASS) {
            // tasked didn't get created
            debug::log("Failed to create rgb task");
            while(1);
        }
    }
}

