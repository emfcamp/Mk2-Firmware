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
#include "SoundTask.h"
#include "AppManager.h"
#include "PMICTask.h"
#include "LCDTask.h"
#include "GUITask.h"
#include "IMUTask.h"

RGBTask* Tilda::_rgbTask = NULL;
SoundTask* Tilda::_soundTask = NULL;
AppManager* Tilda::_appManager = NULL;
RTC_clock* Tilda::_realTimeClock = NULL;
BadgeNotifications* Tilda::_badgeNotifications = NULL;
DataStore* Tilda::_dataStore = NULL;
LCDTask* Tilda::_lcdTask = NULL;
GUITask* Tilda::_guiTask = NULL;
SettingsStore* Tilda::_settingsStore = NULL;
BatterySaverTask* Tilda::_batterySaverTask = NULL;
RadioReceiveTask* Tilda::_radioReceiveTask = NULL;

Tilda::Tilda() {}

ButtonSubscription* Tilda::createButtonSubscription(uint16_t buttons) {
    ButtonSubscription* result = new ButtonSubscription;
    result->addButtons(buttons);
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

void Tilda::playMelody(const uint16_t melody[], const uint16_t tempo[], const uint16_t length) {
    if(_soundTask) {
        _soundTask->playMelody(melody, tempo, length);
    }
}

void Tilda::openApp(app_ctor aNew) {
    if (_appManager) {
        _appManager->open(aNew);
    }
}

RTC_clock& Tilda::getClock() {
    return *_realTimeClock;
}

BadgeNotifications& Tilda::getBadgeNotifications() {
    return *_badgeNotifications;
}

DataStore& Tilda::getDataStore() {
    return *_dataStore;
}

LCDTask& Tilda::getLCDTask() {
    return *_lcdTask;
}

GUITask& Tilda::getGUITask() {
    return *_guiTask;
}

SettingsStore& Tilda::getSettingsStore() {
    return *_settingsStore;
}

AppManager& Tilda::getAppManager() {
    return *_appManager;
}

float Tilda::getBatteryVoltage() {
    return PMIC.getBatteryVoltage();
}

uint8_t Tilda::getBatteryPercent() {
    return PMIC.getBatteryPercent();
}

uint8_t Tilda::getChargeState() {
    return PMIC.getChargeState();
}

Orientation_t Tilda::getOrientation() {
    return (Orientation_t)imuTask.getOrientation();
}

uint32_t Tilda::millisecondsSinceBoot() {
    return xTaskGetTickCount();
}

void Tilda::markActivity() {
    _batterySaverTask->markActivity();
}

char* Tilda::radioChannelIdentifier() {
    return _radioReceiveTask->channelIdentifier();
}

uint8_t Tilda::radioRssi() {
    return _radioReceiveTask->rssi();
}

char* Tilda::getUserNameLine1() {
    return _settingsStore->getUserNameLine1();
}

char* Tilda::getUserNameLine2() {
    return _settingsStore->getUserNameLine2();
}
