/*
 TiLDA Mk2

 BadgeNotifications

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

#include "BadgeNotifications.h"
#include "NotificationApp.h"
#include "AppManager.h"
#include "Utils.h"
#include "Tilda.h"
#include <debug.h>

String BadgeNotification::text() const { return _text; }
RGBColor BadgeNotification::led1() const { return _led1; }
RGBColor BadgeNotification::led2() const { return _led2; }
boolean BadgeNotification::sound() const { return _sound; }
uint8_t BadgeNotification::type() const { return _type; }

BadgeNotifications::BadgeNotifications(AppManager& aAppManager)
    :mAppManager(aAppManager),
    mBadgeNotification(NULL)

{
    mNotificationMutex = xSemaphoreCreateMutex();
}

BadgeNotifications::~BadgeNotifications() {
    delete mBadgeNotification;
}

RGBColor BadgeNotifications::getRGBColor(PackReader& aReader) {
    uint8_t red = static_cast<uint8_t>(Utils::getInteger(aReader));
    uint8_t green = static_cast<uint8_t>(Utils::getInteger(aReader));
    uint8_t blue = static_cast<uint8_t>(Utils::getInteger(aReader));
    return RGBColor(red, blue, green);
}

void BadgeNotifications::pushNotification(String text, RGBColor rgb1, RGBColor rgb2, boolean sound, uint8_t type) {
    if (xSemaphoreTake(mNotificationMutex, portMAX_DELAY) == pdTRUE) {
        delete mBadgeNotification;
        mBadgeNotification = new BadgeNotification(text, rgb1, rgb2, sound, type);
        xSemaphoreGive(mNotificationMutex);
        Tilda::openApp(NotificationApp::New);
    }
}


BadgeNotification* BadgeNotifications::popNotification() {
    BadgeNotification* notification = NULL;

    if (mBadgeNotification == NULL) {
        return NULL;
    }

    if (xSemaphoreTake(mNotificationMutex, portMAX_DELAY) == pdTRUE) {
        notification = new BadgeNotification(*mBadgeNotification);
        xSemaphoreGive(mNotificationMutex);
    }

    return notification;
}
