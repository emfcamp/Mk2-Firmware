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

#include "BlinkApp.h"
#include "Tilda.h"

App* BlinkApp::New() {
    return new BlinkApp;
}

BlinkApp::BlinkApp() {}

String BlinkApp::getName() const {
    return "BlinkApp";
}

bool BlinkApp::keepAlive() const {
    return false;
}

void BlinkApp::task() {
    Tilda::getGUITask().clearRoot(); // Clean screen

    while(true) {
        Tilda::setLedColor({255, 0, 0}); // red
        Tilda::delay(300);
        Tilda::setLedColor({0, 255, 0}); // green
        Tilda::delay(300);
        Tilda::setLedColor({0, 0, 255}); // blue
        Tilda::delay(300);
    }
}

