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
#include "DebugTask.h"
#include "glcd.h"
#include "HomeScreenApp.h"

#include "allBitmaps.h"

String HomeScreenApp::getName() {
    return "HomeScreen";
}

void HomeScreenApp::task() {
    int phase=0;
    debug::log("[HomeScreenApp::task()] Address of GLCD:" + String((long)&GLCD));
    GLCD.FillRect(0,48,128,8);
    while(true) {
       /*
        _rgbTask.setColor({255, 0, 0});
        vTaskDelay((300/portTICK_PERIOD_MS));
        _rgbTask.setColor({0, 255, 0});
        vTaskDelay((300/portTICK_PERIOD_MS));
        _rgbTask.setColor({0, 0, 255});
        vTaskDelay((300/portTICK_PERIOD_MS));
        */
        /*
       if (phase==0)
       {
         GLCD.FillRect(0,0,128,8);
         GLCD.FillRect(0,16,128,8);
         GLCD.FillRect(0,32,128,8);
         GLCD.FillRect(0,48,128,8);
         phase=1;
       } else {
         GLCD.FillRect(0,8,128,8);
         GLCD.FillRect(0,24,128,8);
         GLCD.FillRect(0,40,128,8);
         GLCD.FillRect(0,56,128,8);
         phase=0;
       }*/
       vTaskDelay((1000/portTICK_PERIOD_MS));
    }
}

void HomeScreenApp::afterSuspension() {}
void HomeScreenApp::beforeResume() {}
