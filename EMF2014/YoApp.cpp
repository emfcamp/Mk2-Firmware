/*
 TiLDA Mk2

 YoApp

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
#include "YoApp.h"

#include <FreeRTOS_ARM.h>
#include <debug.h>

#include "ButtonSubscription.h"
#include "RGBTask.h"
#include "AppManager.h"
#include "Tilda.h"
#include <glcd.h>
#include "logo.h"
#include <fonts/allFonts.h>

uint16_t Yo::getMessageTypeId() { 
    return MESSAGE_TYPE_ID_YO;
};

void Yo::serialize(byte* data) {
    uint8_t index = 0;

    // Badge id
    data[index++] = static_cast<byte>(badgeId);
    data[index++] = static_cast<byte>(badgeId >> 8);
    data[index++] = static_cast<byte>(badgeId >> 16);
    data[index++] = static_cast<byte>(badgeId >> 24);

    // Name
    for (uint8_t i=0; i<name.length();  i++) {
        data[index++] = name[i];
    }
}

App* YoApp::New() {
    return new YoApp();
}

YoApp::YoApp() {
    mButtonSubscription = Tilda::createButtonSubscription(A);

    // Start listening to the radio
    Tilda::getRadioTask().listen(1, 1);
}

YoApp::~YoApp() {
    delete mButtonSubscription;
}

String YoApp::getName() const {
    return "Yo!";
}

void YoApp::addYoToList(Yo& yo) {
    if (mYosDisplayed.size() == MAX_YOS_ON_SCREEN) {
        mYosDisplayed.pop_back();
    }

    mYosDisplayed.push_front(yo);
    
    updateScreen();
}

void YoApp::updateScreen() {
    Tilda::getGUITask().clearRoot();
    GLCD.SelectFont(System5x7);
    GLCD.SetRotation(ROTATION_90);
    GLCD.CursorToXY(0, 0);
    GLCD.print("- Press A -");

    for(uint8_t i = 0; i != mYosDisplayed.size(); i++) {
        GLCD.CursorToXY(0, 15 + i*25);
        GLCD.print(mYosDisplayed[i].name);
        GLCD.CursorToXY(20, 25 + i*25);
        GLCD.print("Yo!");
    }
}

void YoApp::task() {
    GLCD.SetRotation(ROTATION_90);

    updateScreen();

    while(true) {
        Button button = mButtonSubscription->waitForPress(100);
        if (button == A) {   
            Yo yo = Yo(Tilda::getBadgeId(), Tilda::getUserNameLine1());    
            addYoToList(yo);
            Tilda::getRadioTask().sendMessage(yo);
        }

        Serializable* message = Tilda::getRadioTask().waitForMessage(1000);
        if (message != NULL && message->getMessageTypeId() == MESSAGE_TYPE_ID_YO) {
            Tilda::log("Got a Yo!");
            addYoToList(*(Yo*)message);
        }
    }
}

void YoApp::afterSuspension() {
    Tilda::getRadioTask().close();
}

void YoApp::beforeResume() {
    if (mButtonSubscription) {
        // Clear Button Queue of any up/down button pressed that have occured durind suspension
        mButtonSubscription->clear();
    }
    updateScreen();
    Tilda::getRadioTask().listen(1, 1);
}
