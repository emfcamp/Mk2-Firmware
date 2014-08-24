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
#include "SponsorsApp.h"
#include "Tilda.h"
#include <glcd.h>
#include <M2tk.h>
#include "GUITask.h"
#include "logo.h"


App* SponsorsApp::New() {
    return new SponsorsApp;
}

SponsorsApp::SponsorsApp() {}

String SponsorsApp::getName() const {
    return "SponsorsApp";
}

bool SponsorsApp::keepAlive() const {
    return true;
}

void SponsorsApp::_draw() {
  if (_page == 0) {
    GLCD.DrawBitmap(SPONSORS_1_XBM, 0, 0);
  } else {
    GLCD.DrawBitmap(SPONSORS_2_XBM, 0, 0);
  }
}

void SponsorsApp::task() {
    Tilda::getGUITask().clearRoot();
    GLCD.SetRotation(ROTATION_0);
    _page = 0;
    while(true) {
        _draw();
        Tilda::delay(5000);
        _page = (_page + 1) % 2;
    }
}

void SponsorsApp::afterSuspension() {}
void SponsorsApp::beforeResume() {
  Tilda::getGUITask().clearRoot();
  GLCD.ClearScreen();
  GLCD.SetRotation(ROTATION_0);
  _draw();
}
