/*
 TiLDA Mk2

 PMICTask
 This task looks after Power Management keeping and eye on battery voltage, lipo battery charging
 It notifies other task of the battery voltage, charge state.

 make current charge sate available
 read voltage at regular intervals and make available
 notify on significant points
 notify on charge state change
 allow interval period change (for sleeping at night)

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

#define PMIC_DEFAULT_RATE       15 * 60 * 1000 // 15 minutes
#define PMIC_CHARGING           LOW
#define PMIC_NOTCHARGING        HIGH

#define PMIC_BATTERY_FULL 652               // 100%
#define PMIC_BATTERY_GOOD 512               // 25%
#define PMIC_BATTERY_LOW 496                // 17%
#define PMIC_BATTERY_VERYLOW 481            // 9%
#define PMIC_BATTERY_FLAT 465               // 0%
#define PMIC_BATTERY_PERCENT_RATIO 0.534


static void PMICChargeStateInterrupt(void);

class PMICTask: public Task {
public:
    String getName() const;
    uint32_t getBatteryReading();
    float getBatteryVoltage();
    uint8_t getBatteryPercent();
    uint8_t getChargeState();
    int8_t setSampleRate(TickType_t ms);
    EventGroupHandle_t eventGroup;
protected:
    void task();
private:
    uint8_t chargeState;
    uint32_t batteryReading;
    TickType_t sampleRate;
};

extern PMICTask PMIC;
