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

#define MAX_APPS 10

AppManager::AppItem::AppItem(App* (*aNew)())
    :mNew(aNew)
{
    mApp = mNew();
    mApp->start();
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

String AppManager::getActiveAppName() const {
    if (mActiveAppItem) {
        return mActiveAppItem->mApp->getName();
    }
    return "";
}

AppManager::AppItem* AppManager::createAndAddApp(App* (*aNew)()) {
    for (int i = 0 ; i < MAX_APPS ; ++i) {
        if (mAppItems[i] == NULL) {
            mAppItems[i] = new AppItem(aNew);
            return mAppItems[i];
        } 
    }

    return NULL;
}

AppManager::AppItem* AppManager::getExistingApp(App* (*aNew)()) {
    for (int i = 0 ; i < MAX_APPS ; ++i) {
        if (mAppItems[i]->mNew == aNew) {
            return mAppItems[i];
        } 
    }

    return NULL;
}

void AppManager::open(App* (*aNew)()) {
    if (mActiveAppItem) {
        debug::log("Current active app: " + mActiveAppItem->mApp->getName());

        if (mActiveAppItem->mNew == aNew) {
            debug::log("Same app so we do nothing");
            return;
        }
    }

    // stop the active app
    if (mActiveAppItem) {
        if (mActiveAppItem->mApp->keepAlive()) {
            // This app should not be detroyed so just suspend it
            mActiveAppItem->mApp->suspend();
        } else {
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

    if (existingApp) {
        // the app already exists so just restart it
        existingApp->mApp->start();
        mActiveAppItem = existingApp;
    } else {
        // We need to create the app
        mActiveAppItem = createAndAddApp(aNew);
    }

    debug::log("New active app: " + mActiveAppItem->mApp->getName());
}
