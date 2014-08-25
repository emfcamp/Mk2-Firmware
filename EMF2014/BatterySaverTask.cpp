/*
 TiLDA Mk2

 BatterySaverTask

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

#include "BatterySaverTask.h"
#include <debug.h>
#include <FreeRTOS_ARM.h>
#include "ButtonSubscription.h"
#include "Tilda.h"
#include "HomeScreenApp.h"

/**
 * BatterySaverTask class
 */
BatterySaverTask::BatterySaverTask() {
    backlightLit = true;
    currentBrightnessLevel = 0;
}

String BatterySaverTask::getName() const {
    return "BatterySaverTask";
}

void BatterySaverTask::markActivity() {
    lastActivity = Tilda::millisecondsSinceBoot();
    currentBrightnessLevel = 7;
    updateBacklightBrightnessLevel();
    backlightLit = true;
}

void BatterySaverTask::task() {
    ButtonSubscription* allButtons = Tilda::createButtonSubscription(LIGHT | A | B | UP | DOWN | LEFT | RIGHT | CENTER);

    updateBacklightBrightnessLevel();

    lastActivity = Tilda::millisecondsSinceBoot();
    while(true) {
        uint32_t nextWait;

        // Does the backlight need to be dimmed or brightened up?
        if (backlightLit == false && currentBrightnessLevel > 0) {
            nextWait = BACKLIGHT_BRIGHTNESS_UPDATE_TIME;
            currentBrightnessLevel--;
            updateBacklightBrightnessLevel();
        } else if (backlightLit == true && currentBrightnessLevel < 7) {
            nextWait = BACKLIGHT_BRIGHTNESS_UPDATE_TIME;
            currentBrightnessLevel++;
            updateBacklightBrightnessLevel();
        } else {
            nextWait = BACKLIGHT_TIMEOUT;
        }

        Button button = allButtons->waitForPress(nextWait);
        if (button == NONE) {
            if (Tilda::millisecondsSinceBoot() - lastActivity >= BACKLIGHT_TIMEOUT) {
                backlightLit = false;
            }

            if (Tilda::millisecondsSinceBoot() - lastActivity >= APP_KILL_TIMEOUT) {
                Tilda::openApp(HomeScreenApp::New);
            }
        } else {
            lastActivity = Tilda::millisecondsSinceBoot();
            backlightLit = true;
        }
    }

    delete allButtons;
}

void BatterySaverTask::updateBacklightBrightnessLevel() {
    uint8_t actualLightLevel = 1 << currentBrightnessLevel;
    analogWrite(LCD_BACKLIGHT, 255 - actualLightLevel);
}
