/*
 TiLDA Mk2

 SettingsStore

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

#include "SettingsStore.h"
#include <DueFlashStorage.h>
#include <debug.h>
#include "Utils.h"
#include "IncomingRadioMessage.h"

#define BADGE_ID_UNKOWN 0;

SettingsStore::SettingsStore() {
    _badgeId = BADGE_ID_UNKOWN;
}

void SettingsStore::handleMessage(const IncomingRadioMessage& radioMessage) {
    uint32_t uniqueId[4];
    if (getUniqueId(uniqueId)) {
        bool ourUniqueId = true;

        for (int i = 0 ; i < 4 ; ++i) {
            uint32_t receivedUniqueId = Utils::bytesToInt(radioMessage.content()[(i * 4) + 3],
                                                            radioMessage.content()[(i * 4) + 2],
                                                            radioMessage.content()[(i * 4) + 1],
                                                            radioMessage.content()[(i * 4) + 0]);

            ourUniqueId = ourUniqueId && receivedUniqueId == uniqueId[i];

            if (!ourUniqueId)
                debug::log("**** " + String(uniqueId[i]) + ":" + String(receivedUniqueId));
        }

        if (ourUniqueId) {
            _badgeId = Utils::bytesToInt(radioMessage.content()[16], radioMessage.content()[17]);
            debug::log("got badge id: " + String(_badgeId));
        } else {
            debug::log("not our unique id");
        }
    }
}

bool SettingsStore::getUniqueId(uint32_t* unique_id) const {
    return flash_init(FLASH_ACCESS_MODE_128, 4) == FLASH_RC_OK &&
           flash_read_unique_id(unique_id, 4) == FLASH_RC_OK;
}

uint16_t SettingsStore::getBadgeId() const {
    return _badgeId;
}

void SettingsStore::setBadgeId(uint16_t badgeId) {
    _badgeId = badgeId;
}

bool SettingsStore::hasBadgeId() const {
    return _badgeId != BADGE_ID_UNKOWN;
}