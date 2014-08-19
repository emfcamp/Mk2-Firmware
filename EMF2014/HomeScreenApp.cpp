/*
 TiLDA Mk2

 Task

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
#include "HomeScreenApp.h"
#include "Tilda.h"
#include <glcd.h>
#include "logo.h"

App* HomeScreenApp::New() {
    return new HomeScreenApp;
}

HomeScreenApp::HomeScreenApp() {}

String HomeScreenApp::getName() const {
    return "HomeScreen";
}

bool HomeScreenApp::keepAlive() const {
    return true;
}

void HomeScreenApp::task() {
    GLCD.DrawBitmap(logo,0,8);
    while(true) {
        Tilda::setLedColor({2, 0, 0});
        Tilda::delay(300);
        Tilda::setLedColor({0, 2, 0});
        Tilda::delay(300);
        Tilda::setLedColor({0, 0, 2});
        Tilda::delay(300);
        Tilda::setLedColor({2, 2, 0});
        Tilda::delay(300);
        Tilda::setLedColor({0, 2, 2});
        Tilda::delay(300);
        Tilda::setLedColor({2, 0, 2});
        Tilda::delay(300);
    }
}

void HomeScreenApp::afterSuspension() {}
void HomeScreenApp::beforeResume() {}
