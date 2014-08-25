/*
 TiLDA Mk2

 NotificationApp

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
#include "NotificationApp.h"
#include "Tilda.h"
#include <glcd.h>
#include <M2tk.h>
#include "GUITask.h"
#include "Utils.h"
#include "BadgeNotifications.h"
#include "HomeScreenApp.h"
#include "Tilda.h"


App* NotificationApp::New() {
        return new NotificationApp;
}

NotificationApp::NotificationApp() {}

String NotificationApp::getName() const {
        return "NotificationApp";
}


bool NotificationApp::keepAlive() const {

    return false;
}

//char notificationApp_m2_notification[];

uint8_t notificationApp_m2_total_lines = 0;
uint8_t notificationApp_m2_first_visible_line = 0;

void notificationApp_m2_uselessCallback(m2_el_fnarg_p fnarg) {}

const char *NotificationApp::notificationTextCallback(m2_rom_void_p element) {
    Utils::wordWrap(stringBuffer, notificationText.c_str(), 21, 6);
    return stringBuffer;
}

M2_LABEL(notificationApp_m2_labelHeader, "f0b1", "Message:");
M2_BOX(notificationApp_m2_line,"h3W64");
//M2_INFO(notificationApp_m2_mainContent, "W59l10E", &notificationApp_m2_first_visible_line, &notificationApp_m2_total_lines, notificationApp_m2_notification, notificationApp_m2_uselessCallback);
M2_LABELFN(notificationApp_m2_mainContent, "W59", NotificationApp::notificationTextCallback);

M2_LIST(notificationApp_m2_list) = { &notificationApp_m2_labelHeader, &notificationApp_m2_line, &notificationApp_m2_mainContent };
M2_VLIST(notificationApp_m2_vlist, NULL, notificationApp_m2_list);
M2_ALIGN(notificationApp_m2_top_el, "-1|2W64H64", &notificationApp_m2_vlist);

String NotificationApp::notificationText = "";
char NotificationApp::stringBuffer[250];

void NotificationApp::task() {
        GLCD.SetRotation(ROTATION_0);

        // Light up screen
        Tilda::markActivity();

        BadgeNotification* notification = Tilda::getBadgeNotifications().popNotification();

        if (notification == NULL) {
            notificationText = "No message found :(";
        } else {
            notificationText = notification->text();
        }

        delete notification;

        Tilda::getGUITask().setM2Root(&notificationApp_m2_top_el);

        while(true) {
            Tilda::delay(100000);
        }
}

void NotificationApp::afterSuspension() {}
void NotificationApp::beforeResume() {}
