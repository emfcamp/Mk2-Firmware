/*
 TiLDA Mk2
 
 TiLDATask
 This is the main cordinator task responsilbe for all the main logic of passing
 information between the other task decieding on what action may be requires at
 any given time
 
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

#include <debug.h>

#include "TiLDATask.h"

#include "EMF2014Config.h"
#include "RGBTask.h"
#include "ButtonTask.h"
#include "RadioReceiveTask.h"
#include "RadioTransmitTask.h"
#include "MessageCheckTask.h"
#include "AppOpenerTask.h"
#include "AppManager.h"
#include "HomeScreenApp.h"
#include "Tilda.h"
#include "SettingsStore.h"
#include "LCDTask.h"
#include "DataStore.h"
#include "PMICTask.h"
#include "GUITask.h"

TiLDATask::TiLDATask() {

}

String TiLDATask::getName() const {
    return "TiLDATask";
}

void TiLDATask::task() {
    RTC_clock* realTimeClock = new RTC_clock(RC);
    SettingsStore* settingsStore = new SettingsStore;
    AppManager* appManager = new AppManager;

    MessageCheckTask* messageCheckTask = new MessageCheckTask;
    DataStore* dataStore = new DataStore(*messageCheckTask);
    RGBTask* rgbTask = new RGBTask;
    ButtonTask* buttonTask = new ButtonTask;
    RadioReceiveTask* radioReceiveTask = new RadioReceiveTask(*messageCheckTask, *realTimeClock);
    RadioTransmitTask* radioTransmitTask = new RadioTransmitTask(*radioReceiveTask, *settingsStore, *messageCheckTask);
    LCDTask* lcdTask = new LCDTask;
    GUITask* guiTask = new GUITask;
    AppOpenerTask* appOpenerTask = new AppOpenerTask(*appManager);

    Tilda::setupTasks(appManager, rgbTask, realTimeClock, lcdTask, guiTask);
    realTimeClock->init();

    // Background tasks
    rgbTask->start();
    buttonTask->start();
    messageCheckTask->start();
    radioReceiveTask->start();
    radioTransmitTask->start();
    lcdTask->start();
    guiTask->start();
    appOpenerTask->start();
    PMIC.start();

    Tilda::openApp(HomeScreenApp::New);

    suspend();
}

