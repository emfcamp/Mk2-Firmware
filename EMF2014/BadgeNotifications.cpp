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

#include "MessageCheckTask.h"
#include "BadgeNotifications.h"
#include "IncomingRadioMessage.h"
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

BadgeNotifications::BadgeNotifications(SettingsStore& aSettingsStore, MessageCheckTask& aMessageCheckTask, AppManager& aAppManager)
    :mSettingsStore(aSettingsStore),
    mMessageCheckTask(aMessageCheckTask),
    mAppManager(aAppManager),
    mBadgeNotification(NULL)

{
    if (mSettingsStore.hasBadgeId()) {
        uint16_t badgeId = mSettingsStore.getBadgeId();
        mMessageCheckTask.subscribe(this, badgeId, badgeId);
    }

    mSettingsStore.addObserver(this);

    mNotificationMutex = xSemaphoreCreateMutex();
    mMessageCheckTask.subscribe(this, 0xb003, 0xb003);

    badgeIdSubscriptionSet = false;
}

BadgeNotifications::~BadgeNotifications() {
    mSettingsStore.removeObserver(this);
    mMessageCheckTask.unsubscribe(this);
    delete mBadgeNotification;
}

RGBColor BadgeNotifications::getRGBColor(PackReader& aReader) {
    uint8_t red = static_cast<uint8_t>(Utils::getInteger(aReader));
    uint8_t green = static_cast<uint8_t>(Utils::getInteger(aReader));
    uint8_t blue = static_cast<uint8_t>(Utils::getInteger(aReader));
    return RGBColor(red, blue, green);
}

void BadgeNotifications::handleMessage(const IncomingRadioMessage& aIncomingRadioMessage) {
    // parse the radio message content into mBadgeNotification
    // <3 bytes rgb1> <3 bytes rgb2> <1 byte sound> <text>

    if (xSemaphoreTake(mNotificationMutex, portMAX_DELAY) == pdTRUE) {
        mReader.setBuffer((unsigned char*)aIncomingRadioMessage.content(), aIncomingRadioMessage.length());

        RGBColor rgb1 = getRGBColor(mReader);
        RGBColor rgb2 = getRGBColor(mReader);
        boolean sound = Utils::getBoolean(mReader);
        uint8_t type = static_cast<uint8_t>(Utils::getInteger(mReader));
        String text = Utils::getString(mReader);
        debug::log("BADGER NOTIFICATION");
        debug::log(text);

        delete mBadgeNotification;
        mBadgeNotification = new BadgeNotification(text, rgb1, rgb2, sound, type);
        xSemaphoreGive(mNotificationMutex);
        Tilda::openApp(NotificationApp::New);
    }

    // start the notification app to start it
    //mAppManager->open(NotificationApp::New);
}

void BadgeNotifications::badgeIdChanged(uint16_t badgeId) {
    if (!badgeIdSubscriptionSet) {
        badgeIdSubscriptionSet = true;
        mMessageCheckTask.subscribe(this, badgeId, badgeId);
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
