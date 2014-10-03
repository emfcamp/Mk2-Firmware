/*
 TiLDA Mk2

 RadioTask

 This class handles radio communication in some sort of organised fashion. 

 See: https://github.com/sebnil/DueFlashStorage/blob/master/flash_efc.cpp#L845

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
#include "Serializable.h"
#include "Task.h"

class RadioTask: public Task {
public:
    RadioTask();
    String getName() const;
    void listen(uint8_t channel, uint16_t panId);
    void close();
    Serializable* waitForMessage(TickType_t ticksToWait);
    Serializable* waitForMessage();
protected:
    void task();

private:
    void _enterAtMode();
    void _leaveAtMode();
    void _sleep();
    void _wakeUp();
    void _clearSerialBuffer();
    uint8_t _parsePacketBuffer(byte packetBuffer[], uint8_t packetBufferLength);

    QueueHandle_t _messages;
    QueueHandle_t _wakeUpSignal;
    uint8_t _channel; 
    uint16_t _panId;
    bool _listening;
    bool _changed;
    uint8_t _rssi;
};
