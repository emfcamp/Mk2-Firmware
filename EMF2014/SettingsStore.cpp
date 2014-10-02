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
#include <Sha1.h>
#include "Utils.h"

SettingsStore::SettingsStore() {
    mBadgeIdAlreadyCalculated = false;
}

SettingsStore::~SettingsStore() {}

char* SettingsStore::getUserNameLine1() {
    return name1;
}

char* SettingsStore::getUserNameLine2() {
    return name2;
}

bool SettingsStore::getUniqueId(uint32_t* unique_id) const {
    return flash_init(FLASH_ACCESS_MODE_128, 4) == FLASH_RC_OK &&
           flash_read_unique_id(unique_id, 4) == FLASH_RC_OK;
}

const uint32_t SettingsStore::getBadgeId() {
    if (!mBadgeIdAlreadyCalculated) {
        uint32_t uniqueId[4];
        SettingsStore::getUniqueId(uniqueId);

        Sha1.init();
        Sha1.print(uniqueId[0]);
        Sha1.print(uniqueId[1]);
        Sha1.print(uniqueId[2]);
        Sha1.print(uniqueId[3]);

        byte* fullHash = Sha1.result();
        mBadgeId = Utils::bytesToInt(fullHash[0], fullHash[1], fullHash[2], fullHash[3]);
        delete fullHash;
        mBadgeIdAlreadyCalculated = true;
    }

    return mBadgeId;  
}