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
#include "BatterySaverTask.h"
#include "RadioReceiveTask.h"
#include "RadioTransmitTask.h"
#include "MessageCheckTask.h"
#include "AppOpenerTask.h"
#include "AppManager.h"
#include "HomeScreenApp.h"
#include "SponsorsApp.h"
#include "BadgeIdApp.h"
#include "NotificationApp.h"
#include "SnakeApp.h"
#include "Tilda.h"
#include "SettingsStore.h"
#include "LCDTask.h"
#include "DataStore.h"
#include "PMICTask.h"
#include "BadgeNotifications.h"
#include "GUITask.h"
#include "IMUTask.h"
#include <glcd.h>

TiLDATask::TiLDATask() {

}

String TiLDATask::getName() const {
    return "TiLDATask";
}

void TiLDATask::task() {
    Tilda::_realTimeClock = new RTC_clock(RC);
    Tilda::_appManager = new AppManager;

    MessageCheckTask* messageCheckTask = new MessageCheckTask;
    SettingsStore* settingsStore = new SettingsStore(*messageCheckTask);
    Tilda::_dataStore = new DataStore(*messageCheckTask);
    Tilda::_rgbTask = new RGBTask;
    Tilda::_settingsStore = settingsStore;
    Tilda::_batterySaverTask = new BatterySaverTask;
    RadioReceiveTask* radioReceiveTask = new RadioReceiveTask(*messageCheckTask, *Tilda::_realTimeClock);
    Tilda::_radioReceiveTask = radioReceiveTask;
    RadioTransmitTask* radioTransmitTask = new RadioTransmitTask(*radioReceiveTask, *settingsStore, *messageCheckTask);
    AppOpenerTask* appOpenerTask = new AppOpenerTask(*Tilda::_appManager);
    Tilda::_lcdTask = new LCDTask;
    GLCD.SetRotation(ROTATION_90);
    Tilda::_badgeNotifications = new BadgeNotifications(*settingsStore, *messageCheckTask, *Tilda::_appManager);
    Tilda::_guiTask = new GUITask;

    Tilda::_realTimeClock->init();

    // Background tasks
    Tilda::_rgbTask->start();
    Tilda::_batterySaverTask->start();
    messageCheckTask->start();
    radioReceiveTask->start();
    radioTransmitTask->start();
    Tilda::_lcdTask->start();
    Tilda::_guiTask->start();
    appOpenerTask->start();
    PMIC.start();
    imuTask.start();

    Tilda::openApp(HomeScreenApp::New);

    suspend();
}

