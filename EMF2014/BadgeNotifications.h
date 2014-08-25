/*
 TiLDA Mk2

 BadgeNotifications
 This class collects, organises and exposes Notifications.

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
#include <TinyPacks.h>

#include "EMF2014Config.h"
#include "RGBTask.h"
#include "SettingsStore.h"

class MessageCheckTask;
class AppManager;

class BadgeNotification {
public:
    BadgeNotification(String text, RGBColor led1, RGBColor led2, boolean sound, uint8_t type) :_text(text), _led1(led1), _led2(led2), _sound(sound), _type(type) {}
    String text() const;
    RGBColor led1() const;
    RGBColor led2() const;
    boolean sound() const;
    uint8_t type() const;
private:
    String _text;
    RGBColor _led1;
    RGBColor _led2;
    boolean _sound;
    uint8_t _type;
};

class BadgeNotifications : public SettingsStoreObserver, public RadioMessageHandler {
public:
    BadgeNotifications(SettingsStore& aSettingsStore, MessageCheckTask& aMessageCheckTask, AppManager& aAppManager);
    ~BadgeNotifications();

    BadgeNotification* popNotification();

private: // from RadioMessageHandler
    void handleMessage(const IncomingRadioMessage&);

private: // from SettingsStoreObserver
    void badgeIdChanged(uint16_t badgeId);

private:
    static RGBColor getRGBColor(PackReader& aReader);

    bool badgeIdSubscriptionSet;

private:
    MessageCheckTask& mMessageCheckTask;
    SettingsStore& mSettingsStore;
    AppManager& mAppManager;
    BadgeNotification* mBadgeNotification;
    SemaphoreHandle_t mNotificationMutex;

    PackReader mReader;
};
