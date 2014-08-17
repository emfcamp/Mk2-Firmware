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
    for (uint8_t i=0; i<MAX_APPS; i++) {
        _apps[i] = NULL;
    }
}

AppManager::~AppManager() {

}

void AppManager::add(App& app) {
    for (uint8_t i=0; i<MAX_APPS; i++) {
        if (_apps[i] == NULL) {
            _apps[i] =& app;
            break;
        }
    }
}   

App& AppManager::getByName(const String& name) const {
    for (uint8_t i=0; i<MAX_APPS; i++) {
        if (_apps[i] != NULL) {
            if (_apps[i]->getName() == name) {
                return *_apps[i];
            }
        } 
    }
    debug::log("Error: Trying to start App that doesn't exist: " + name);
    return *_apps[0];
}  

void AppManager::open(App& app) {
    debug::log("Opening " + app.getName());
    if (activeApp) {
        debug::log("Current Active App " + activeApp->getName());
    } 
    if (activeApp == &app) {
        return; // App is already active
    }
    if (activeApp != NULL) {
        activeApp->suspend();
    }
    app.start();
    activeApp = &app;
} 

String AppManager::getActiveAppName() const {
    if (activeApp) {
        return activeApp->getName();
    }
    return "";
}

void AppManager::open(const String& name) {
    open(getByName(name));
}
