/*
 TiLDA Mk2

 Ed Balls

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
#include "EdBallsApp.h"
#include "Tilda.h"
#include <glcd.h>
#include <M2tk.h>
#include "GUITask.h"
#include <fonts/allFonts.h>

// Ed Balls

App* EdBallsApp::New() {
    return new EdBallsApp;
}

EdBallsApp::EdBallsApp() {
    mButtonSubscription = Tilda::createButtonSubscription(UP | DOWN);
}

EdBallsApp::~EdBallsApp() {
    delete mButtonSubscription;
}

String EdBallsApp::getName() const {
        return "EdBallsApp";
}

bool EdBallsApp::keepAlive() const {
        return false;
}

void EdBallsApp::task() {
        Tilda::getGUITask().clearRoot(); // Clean screen
        GLCD.SetRotation(ROTATION_90);

        Tilda::setLedColor({0, 0, 0});

        while(true) {  // Make sure this loop goes on forever
          GLCD.SelectFont(System5x7);
          GLCD.CursorToXY(8, 60);
          GLCD.print("Ed Balls");             // Ed Balls
          Button button = mButtonSubscription->waitForPress(1000);
            if (button == UP) {
                Tilda::setLedColor({250, 129, 0}); // orange
            } else if (button == DOWN) {
                Tilda::setLedColor({171, 205, 239}); // turquoise
            } else {
                Tilda::setLedColor({0, 0, 0});   // LEDs off
            }
        }
}

void EdBallsApp::afterSuspension() {} // If keepAlive() is true this is being called when the task is suspended
void EdBallsApp::beforeResume() {} // If keepAlive() is true this is being called when the task is resumed
