/*
 TiLDA Mk2

 RGBTask
 This task handles the two RGB LEDs for the front of the badge.
 Setting them to a requested color and also provides the Torch functionality
 RGB Request are of type RGBRequest_t are sent to the RGBRequestQueue by other tasks

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

#pragma once

#include <Arduino.h>
#include <FreeRTOS_ARM.h>
#include "EMF2014Config.h"
#include "Task.h"

#define INITIAL_COLOR 0, 0, 0
#define COLOR_CHANGE_PER_FRAME 3
#define FRAME_RATE_CHANGE 10 / portTICK_PERIOD_MS
#define FRAME_RATE_SLEEP 100 / portTICK_PERIOD_MS

enum RGBLed {
    LED1,
    LED2,
    BOTH
};

class RGBColor {
public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    RGBColor (uint8_t setRed, uint8_t setGreen, uint8_t setBlue): red(setRed), green(setGreen), blue(setBlue) {};
private:
};

class RGBTask: public Task {
public:
    RGBTask();

    String getName() const;
    void setColor(RGBLed led, RGBColor color);
    void setColor(RGBColor color);

private:
    RGBTask(const RGBTask&) {}

protected:
    void task();
};
