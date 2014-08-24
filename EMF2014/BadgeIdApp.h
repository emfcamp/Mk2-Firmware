/*
 TiLDA Mk2

 HomeScreenApp
 This is just a placeholder at the moment - Later this will be showing some sort of menu

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

#pragma once

#include <Arduino.h>
#include <FreeRTOS_ARM.h>
#include "EMF2014Config.h"
#include "App.h"
#include "RGBTask.h"
#include "GUITask.h"

class BadgeIdApp: public App {
public:
    static App* New();

    String getName() const;
protected:
private:
    BadgeIdApp();
    BadgeIdApp(const BadgeIdApp&);

    bool keepAlive() const;

    void task();
    void afterSuspension();
    void beforeResume();
private:
};
