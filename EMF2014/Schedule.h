/*
 TiLDA Mk2

 Schedule

 This handles the periodic wake of the radio for all our need communication with the gateway's.
 Incoming request are passed back to the TiLDATask
 Outgoing request from TiLDATask are sent at the next opportunity.

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
#include "Utils.h"

enum LocationId : uint8_t {
    LOCATION_STAGE_A,
    LOCATION_STAGE_B,
    LOCATION_STAGE_C,
    LOCATION_WORKSHOPS,
    LOCATION_KIDS,
    LOCATION_VILLAGES,
    LOCATION_LOUNGE,
    LOCATION_BAR,
    LOCATION_MODEL_FLYING,
    LOCATION_CATERING,
    LOCATION_EMFFM,
    LOCATION_OTHER,
    LOCATION_COUNT
};

class Event {
public:
    Event();
    ~Event();
    Event& operator=(const Event& that);

private:
    Event(Event&);    

public:
    LocationId locationId;
    uint8_t typeId;
    uint32_t startTimestamp;
    uint32_t endTimestamp;
    char* speaker;
    char* title;
};

class Schedule {
public:
    Schedule(Event* aEvents, int32_t aEventCount);
    Schedule(const Schedule&);

    ~Schedule();

    int32_t getEventCount() const;
    Event* getEvents() const;

    static char* getStageName(uint8_t aLocationId);

private:
    Schedule();

private:
    Event* mEvents;
    int32_t mEventCount;
};

enum ScheduleDay: uint8_t {
    SCHEDULE_FRIDAY = 0,
    SCHEDULE_SATURDAY = 1,
    SCHEDULE_SUNDAY = 2,
    SCHEDULE_NUM_DAYS
};
