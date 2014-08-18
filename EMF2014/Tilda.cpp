/*
 TiLDA Mk2

 TildaApi

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

#include "Tilda.h"
#include <FreeRTOS_ARM.h>
#include <debug.h>
#include "RGBTask.h"
#include "AppManager.h"
#include "PMICTask.h"

RGBTask *Tilda::_rgbTask = NULL;
AppManager *Tilda::_appManager = NULL;
RTC_clock *Tilda::_realTimeClock = NULL;

Tilda::Tilda() {}

ButtonSubscription* Tilda::createButtonSubscription(uint16_t buttons) {
    ButtonSubscription* result = new ButtonSubscription;
    result->addButtons(LIGHT | A | B | UP | DOWN | LEFT | RIGHT | CENTER);
    return result;
}

void Tilda::log(String text) {
    debug::log(text);
}

void Tilda::delay(uint16_t durationInMs) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        vTaskDelay(durationInMs / portTICK_PERIOD_MS);
    } else {
        delay(durationInMs);
    }
}

void Tilda::setLedColor(RGBLed led, RGBColor color) {
    if (_rgbTask) {
        _rgbTask->setColor(led, color);
    }
}

void Tilda::setLedColor(RGBColor color) {
    if (_rgbTask) {
        _rgbTask->setColor(color);
    }
}

void Tilda::openApp(App* (*New)()) {
    if (_appManager) {
        _appManager->open(New);
    }
}

RTC_clock* Tilda::getClock() {
    return _realTimeClock;
}


static float getBatteryVoltage() {
    return PMIC.getBatteryVoltage();
}

static uint8_t getBatteryPercent() {
    return PMIC.getBatteryPercent();
}
static uint8_t getChargeState() {
    return PMIC.getChargeState();
}


void Tilda::setupTasks(AppManager* appManager, RGBTask* rgbTask, RTC_clock* realTimeClock) {
    _appManager = appManager;
    _rgbTask = rgbTask;
    _realTimeClock = realTimeClock;
}
