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
#include "HomeScreenApp.h"
#include "FlashLightApp.h"
#include "SponsorsApp.h"
#include "AppManager.h"
#include "BadgeIdApp.h"
#include "SnakeApp.h"
#include "BlinkApp.h"
#include "HelloWorldApp.h"
#include "ScheduleApp.h"
#include "TetrisApp.h"
#include "WeatherApp.h"

// Add your app here to appear in the app list
static const AppDefinition APPS[] = {
        AppDefinition("Schedule",      ScheduleApp::New),
        AppDefinition("Weather",       WeatherApp::New),
        AppDefinition("Badge ID",      BadgeIdApp::New),
        AppDefinition("Snake",         SnakeApp::New),
        AppDefinition("Tetris",        TetrisApp::New),
        AppDefinition("HelloWorld",    HelloWorldApp::New),
        AppDefinition("Blink!",        BlinkApp::New),
        AppDefinition("Sponsors",      SponsorsApp::New)
};

AppManager::AppItem::AppItem(app_ctor aNew)
    :mNew(aNew)
{
    mApp = mNew();
}

AppManager::AppItem::~AppItem() {
    delete mApp;
}

AppManager::AppManager()
    :mActiveAppItem(NULL)
{
    mAppItems = new AppItem*[MAX_APPS];
    for (int i = 0 ; i < MAX_APPS ; ++i) {
        mAppItems[i] = NULL;
    }
}

AppManager::~AppManager() {
    // no need to delete the active ap as
    // it'll be in the list of all apps
    delete[] mAppItems;
}


uint8_t AppManager::getAppCount() {
    return sizeof(APPS) / sizeof(AppDefinition);
}

AppDefinition AppManager::getById(uint8_t id) {
    return APPS[id];
}

String AppManager::getActiveAppName() const {
    if (mActiveAppItem) {
        return mActiveAppItem->mApp->getName();
    }
    return "";
}

AppManager::AppItem* AppManager::createAndAddApp(app_ctor aNew) {
    for (int i = 0 ; i < MAX_APPS ; ++i) {
        if (mAppItems[i] == NULL) {
            mAppItems[i] = new AppItem(aNew);
            return mAppItems[i];
        }
    }

    return NULL;
}

AppManager::AppItem* AppManager::getExistingApp(app_ctor aNew) {
    for (int i = 0 ; i < MAX_APPS ; ++i) {
        if (mAppItems[i]->mNew == aNew) {
            return mAppItems[i];
        }
    }

    return NULL;
}

void AppManager::open(app_ctor aNew) {
    if (mActiveAppItem) {
        debug::log("Current active app: " + mActiveAppItem->mApp->getName());

        if (mActiveAppItem->mNew == aNew) {
            debug::log("Same app so we do nothing");
            return;
        }
    }

    // stop the active app
    if (mActiveAppItem) {
        mActiveAppItem->mApp->suspend();

        if (!mActiveAppItem->mApp->keepAlive()) {
            debug::log("Deleting task: " + mActiveAppItem->mApp->getName());

            // This app is getting destroyed and removed from the list
            for (int i = 0 ; i < MAX_APPS ; ++i) {
                if (mAppItems[i]->mNew == mActiveAppItem->mNew) {
                    delete mAppItems[i];
                    mAppItems[i] = NULL;
                    break;
                }
            }

            mActiveAppItem = NULL;
        }
    }

    // is the app already in our list?
    AppItem* existingApp = getExistingApp(aNew);
    mActiveAppItem = existingApp ? existingApp : createAndAddApp(aNew);
    mActiveAppItem->mApp->start();

    debug::log("New active app: " + mActiveAppItem->mApp->getName());
}

void AppManager::orientationCallback(uint8_t orientation) {
    if(mActiveAppItem) {
        mActiveAppItem->mApp->newOrientation(orientation);
    }
}
