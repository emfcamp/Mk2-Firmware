/*
 TiLDA Mk2

 Flash Light App

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
#include "FlashLightApp.h"

#include <FreeRTOS_ARM.h>
#include <debug.h>

#include "ButtonSubscription.h"
#include "RGBTask.h"
#include "AppManager.h"
#include "Tilda.h"
#include <glcd.h>
#include "logo.h"
#include <fonts/allFonts.h>

App* FlashLightApp::New() {
    return new FlashLightApp();
}

FlashLightApp::FlashLightApp()
    :mLightLevel(7)
{
    mButtonSubscription = Tilda::createButtonSubscription(UP | DOWN | LEFT | RIGHT);

}

FlashLightApp::~FlashLightApp() {
    delete mButtonSubscription;
}

// TODO: Add all the fancy features from https://github.com/emfcamp/Mk2-Firmware/blob/master/frRGBTask/frRGBTask.ino
String FlashLightApp::getName() const {
    return "FlashLight";
}

void FlashLightApp::updateLeds() {
    uint8_t actualLightLevel = 1 << mLightLevel;

    if (Tilda::getOrientation() != ORIENTATION_HUNG) {
        actualLightLevel = 2;
        if (!showDimMessage) {
            showDimMessage = true;
            updateMessage();
        }
    } else {
        if (showDimMessage) {
            showDimMessage = false;
            updateMessage();
        }
    }

    Tilda::setLedColor({actualLightLevel, actualLightLevel, actualLightLevel});
}

void FlashLightApp::updateMessage() {
    Tilda::getGUITask().clearRoot();
    GLCD.DrawBitmap(EMF_LOGO_XBM ,17, 8);
    GLCD.SelectFont(System5x7);
    if (showDimMessage) {
        GLCD.SetRotation(ROTATION_90);
        GLCD.CursorToXY(0, 105);
        GLCD.print(" Non-blind");
        GLCD.CursorToXY(0, 115);
        GLCD.print("   Mode");
    } else {
        GLCD.SetRotation(ROTATION_270);
        GLCD.CursorToXY(0, 105);
        GLCD.print("UP/DOWN for");
        GLCD.CursorToXY(0, 115);
        GLCD.print("brightness");
    }
}

void FlashLightApp::task() {
    GLCD.SetRotation(ROTATION_90);

    updateLeds();
    showDimMessage = true;
    updateMessage();

    while(true) {
        Button button = mButtonSubscription->waitForPress(100);
        if (button == UP || button == LEFT) {
            if (mLightLevel < 7) {
                mLightLevel++;
            } else {
                mLightLevel = 7;
            };
        } else if (button == DOWN || button == RIGHT) {
            if (mLightLevel > 1) {
                mLightLevel--;
            } else {
                mLightLevel = 0;
            }
        }
        updateLeds();
        GLCD.SelectFont(fixednums8x16);
        GLCD.CursorToXY(8,72);
        GLCD.print(mLightLevel);
    }
}

void FlashLightApp::afterSuspension() {
    Tilda::setLedColor({0, 0, 0});
}

void FlashLightApp::beforeResume() {
    if (mButtonSubscription) {
        // Clear Button Queue of any up/down button pressed that have occured durind suspension
        mButtonSubscription->clear();
    }
    updateLeds();
}
