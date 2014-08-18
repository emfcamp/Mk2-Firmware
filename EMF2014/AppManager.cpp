/*
 TiLDA Mk2

 AppManager

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
#include <FreeRTOS_ARM.h>
#include <debug.h>

#include "EMF2014Config.h"

#include "AppManager.h"

AppManager::AppManager() {
    mActiveAppItem.mApp = NULL;
    mActiveAppItem.mNew = NULL;
}

AppManager::~AppManager() {
    delete mActiveAppItem.mApp;
}

String AppManager::getActiveAppName() const {
    if (mActiveAppItem.mApp) {
        return mActiveAppItem.mApp->getName();
    }
    return "";
}

void AppManager::open(App* (*aNew)()) {
    if (mActiveAppItem.mApp)
        debug::log("Current active app: " + mActiveAppItem.mApp->getName());

    if (mActiveAppItem.mNew == aNew) {
        debug::log("Same app");
        return;
    }

    // destroy the old active app, create and start the new one
    delete mActiveAppItem.mApp;
    mActiveAppItem.mNew = aNew;
    mActiveAppItem.mApp = mActiveAppItem.mNew();
    mActiveAppItem.mApp->start();

    debug::log("New active app: " + mActiveAppItem.mApp->getName());
}
