/*
 TiLDA Mk2
 
 MessageCheckTask

 This is a consumer task that checks messages received by the radio task for validity

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
#include "Task.h"

//Unidentified Badges
#define RID_RANGE_UNINDENTIFIED_BADGE 0x0000 
// Badge IDs (16384 badges max)
#define RID_RANGE_BADGE_ID_START 0x0001
#define RID_RANGE_BADGE_ID_END 0x8FFF
//Special backend service (e.g. badge id negotiation)
#define RID_RANGE_SPECIAL_START 0x9000
#define RID_RANGE_SPECIAL_END 0x9FFF 
//Content (e.g. Schedule Saturday, Weather forecast)
#define RID_RANGE_CONTENT_START 0xA000
#define RID_RANGE_CONTENT_END 0xAFFF 
//Special non-content broadcasts (e.g. start reply-window, reply with badge id)
#define RID_RANGE_NON_CONTENT_START 0xB000
#define RID_RANGE_NON_CONTENT_END 0xBFFF 
//Reserved
#define RID_RANGE_RESERVED_START 0xC000
#define RID_RANGE_RESERVED_RESERVED_END 0xFFFF

#define RID_START_TRANSMIT_WINDOW 0xB001
#define RID_BADGE_ID 0xB002

class RadioMessageHandler;
class IncomingRadioMessage;

class MessageCheckTask: public Task {
private:
    struct HandlerItem {
        RadioMessageHandler* mHandler;
        uint16_t mRangeStart;
        uint16_t mRangeEnd;
    };

public:
    MessageCheckTask();
    ~MessageCheckTask();

	String getName() const;

    void subscribe(RadioMessageHandler* aHandler, uint16_t aRangeStart, uint16_t aRangeEnd); 
    void unsubscribe(RadioMessageHandler* aHandler); 

	void addIncomingMessage(IncomingRadioMessage *message);

private:
    MessageCheckTask(const MessageCheckTask&) {}

protected:
	void task();

private:
	QueueHandle_t mIncomingMessages;

    HandlerItem** mHandlers;
    SemaphoreHandle_t mHandlersSemaphore;
};
