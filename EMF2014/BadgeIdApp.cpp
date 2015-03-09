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
#include "BadgeIdApp.h"
#include "Tilda.h"
#include <glcd.h>
#include <M2tk.h>
#include "GUITask.h"
#include <fonts/fixednums15x31.h>

App* BadgeIdApp::New() {
        return new BadgeIdApp;
}

BadgeIdApp::BadgeIdApp() {}

String BadgeIdApp::getName() const {
        return "BadgeIdApp";
}

bool BadgeIdApp::keepAlive() const {
        return false;
}

// The BadgeId
char badgeIdApp_m2_label1_text[] = " EMFxxxxxxxx";

// Screen for when the ID is known
M2_LABEL(badgeIdApp_m2_label0, "f0", "Your Badge ID is:");
M2_LABEL(badgeIdApp_m2_label1, "f25w123b1", badgeIdApp_m2_label1_text);
M2_LIST(badgeIdApp_m2_list_dt) = {&badgeIdApp_m2_label0, &badgeIdApp_m2_label1};
M2_VLIST(badgeIdApp_m2_label_list_found, NULL, badgeIdApp_m2_list_dt);

void BadgeIdApp::task() {
    GLCD.SetRotation(ROTATION_0);

    uint32_t badgeId = Tilda::getSettingsStore().getBadgeId();
    uint8_t badgeIdBytes[] = {static_cast<byte>(badgeId >> 24), static_cast<byte>(badgeId >> 16), static_cast<byte>(badgeId >> 8), static_cast<byte>(badgeId)};
    badgeIdApp_m2_label1_text[4]  = "0123456789abcdef"[badgeIdBytes[0]>>4];
    badgeIdApp_m2_label1_text[5]  = "0123456789abcdef"[badgeIdBytes[0]&0xf];
    badgeIdApp_m2_label1_text[6]  = "0123456789abcdef"[badgeIdBytes[1]>>4];
    badgeIdApp_m2_label1_text[7]  = "0123456789abcdef"[badgeIdBytes[1]&0xf];
    badgeIdApp_m2_label1_text[8]  = "0123456789abcdef"[badgeIdBytes[2]>>4];
    badgeIdApp_m2_label1_text[9]  = "0123456789abcdef"[badgeIdBytes[2]&0xf];
    badgeIdApp_m2_label1_text[10] = "0123456789abcdef"[badgeIdBytes[3]>>4];
    badgeIdApp_m2_label1_text[11] = "0123456789abcdef"[badgeIdBytes[3]&0xf];

    Tilda::getGUITask().setM2Root(&badgeIdApp_m2_label_list_found);

    while(true) {
        Tilda::delay(300);
    }
}

void BadgeIdApp::afterSuspension() {}
void BadgeIdApp::beforeResume() {}
