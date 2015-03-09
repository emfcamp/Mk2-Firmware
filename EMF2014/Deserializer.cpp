/*
 TiLDA Mk2

 Deserializer

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

#include "Deserializer.h"
#include "Serializable.h"
#include "Utils.h"
#include "Tilda.h"

#include "YoApp.h"

Serializable* Deserializer::deserialize(const byte* data) {
    uint16_t messageTypeId = data[1] * 256 + data[0];

    if (messageTypeId == MESSAGE_TYPE_ID_YO) {
        uint32_t badgeId = Utils::bytesToInt(data[2], data[3], data[4], data[5]);
        char name[11]; 
        for (uint8_t i=0; i<10; i++) name[i] = data[i + 6];
        name[10] = 0;
        return new Yo(badgeId, String(name));
    }

    Tilda::log("Deserializer: Couldn't handle message type id " + String(messageTypeId));
    return NULL;
}