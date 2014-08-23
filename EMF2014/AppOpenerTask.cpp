/*
 TiLDA Mk2

 App Opener Task

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
#include "AppOpenerTask.h"

#include <FreeRTOS_ARM.h>
#include <debug.h>

#include "ButtonSubscription.h"
#include "Tilda.h"
#include "HomeScreenApp.h"
#include "FlashLightApp.h"


AppOpenerTask::AppOpenerTask(AppManager& aAppManager)
    :mAppManager(aAppManager)
{}

String AppOpenerTask::getName() const {
    return "AppOpener";
}

void AppOpenerTask::task() {
    ButtonSubscription* buttonSubscription = Tilda::createButtonSubscription(LIGHT | CENTER);

    while(true) {
        Button button = buttonSubscription->waitForPress();
        if (button == LIGHT) {
            if (mAppManager.getActiveAppName() == "FlashLight") {
                mAppManager.open(HomeScreenApp::New);
            } else {
                mAppManager.open(FlashLightApp::New);
            }
        } else if (button == CENTER) {
            delay(CENTER_BUTTON_PRESS_DURATION_FOR_HOME_SCREEN);
            if (digitalRead(BUTTON_CENTER) == LOW) {
                mAppManager.open(HomeScreenApp::New);
            }
        }
    }

    delete buttonSubscription;
}
