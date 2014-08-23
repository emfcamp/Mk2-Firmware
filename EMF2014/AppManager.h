/*
 TiLDA Mk2

 AppManager
 Makes sure only one App is running at any given time

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
#include "App.h"

typedef App* (*app_ctor)();

struct AppDefinition {
    AppDefinition(String aName, app_ctor aNew):mName(aName),mNew(aNew){}
    String mName;
    app_ctor mNew;
};


// Orientation change callback
void AppManagerOrientationCallback(uint8_t newOrientation);


class AppManager {
private:
    class AppItem {
    public:
        AppItem(app_ctor aNew);
        ~AppItem();

    public:
        App* mApp;
        app_ctor mNew; // the function pointer to create the app is used as an app id
    };

public:
	AppManager();
	~AppManager();

    // The app list
    uint8_t getAppCount();
    AppDefinition getById(uint8_t id);

    // Your app should have a `static App* New()` that creates the app
    // which can be passed in here like `MyApp::New`
	void open(app_ctor aNew);
	String getActiveAppName() const;
    
    void orientationCallback(uint8_t orientation);

private:
    AppItem* createAndAddApp(app_ctor aNew);
    AppItem* getExistingApp(app_ctor aNew);

    AppItem* mActiveAppItem;
    AppItem** mAppItems;
};
