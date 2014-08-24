/*
 TiLDA Mk2

 Task

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
#include <rtc_clock.h>
#include "ScheduleApp.h"
#include "Tilda.h"
#include <glcd.h>
#include "logo.h"
#include <M2tk.h>
#include "GUITask.h"
#include "Schedule.h"
#include "DataStore.h"

uint8_t first;
uint8_t cnt;

const char* talks_callback(uint8_t talk, uint8_t msg);
const char* locations_callback(uint8_t talk, uint8_t msg);

#define SETUP_MENU(data,el,space,vsb,list,hlist,align) \
M2_X2LMENU(el,"l10e1w51",&first,&cnt,data,'+','-','\0');\
M2_SPACE(space,"W1h1");\
M2_VSB(vsb,"l10w4r1", &first, &cnt);\
M2_LIST(list)={&el,&space,&vsb};\
M2_HLIST(hlist,NULL, list);\
M2_ALIGN(align,"-1|1W64H64",&hlist);

char schedule_label_talk_empty_str[] = "Empty.";
M2_LABEL(schedule_label_talk_empty, NULL, schedule_label_talk_empty_str);
M2_ALIGN(schedule_label_talk_empty_align, "-1|1W64H64", &schedule_label_talk_empty);

const char* schedule_label_talk_str = NULL;
M2_LABELPTR(schedule_label_talk, NULL, &schedule_label_talk_str);
M2_ALIGN(schedule_label_talk_align, "-1|1W64H64", &schedule_label_talk);

#define MAX_EVENTS_FOR_LOCATION 50
m2_xmenu_entry schedule_talks_data[MAX_EVENTS_FOR_LOCATION];
Event* schedule_talks_events[MAX_EVENTS_FOR_LOCATION];

SETUP_MENU(schedule_talks_data,
            schedule_talks_el,
            schedule_talks_space,
            schedule_talks_vsb,
            schedule_talks_list,
            schedule_talks_hlist,
            schedule_talks_align);

m2_xmenu_entry schedule_location_data[] = {
    {Schedule::getStageName(LOCATION_STAGE_A),      &schedule_talks_align, locations_callback},
    {Schedule::getStageName(LOCATION_STAGE_B),      &schedule_talks_align, locations_callback},
    {Schedule::getStageName(LOCATION_STAGE_C),      &schedule_talks_align, locations_callback},
    {Schedule::getStageName(LOCATION_WORKSHOPS),    &schedule_talks_align, locations_callback},
    {Schedule::getStageName(LOCATION_KIDS),         &schedule_talks_align, locations_callback},
    {Schedule::getStageName(LOCATION_VILLAGES),     &schedule_talks_align, locations_callback},
    {Schedule::getStageName(LOCATION_LOUNGE),       &schedule_talks_align, locations_callback},
    {Schedule::getStageName(LOCATION_BAR),          &schedule_talks_align, locations_callback},
    {Schedule::getStageName(LOCATION_MODEL_FLYING), &schedule_talks_align, locations_callback},
    {Schedule::getStageName(LOCATION_CATERING),     &schedule_talks_align, locations_callback},
    {Schedule::getStageName(LOCATION_EMFFM),        &schedule_talks_align, locations_callback},
    {Schedule::getStageName(LOCATION_COUNT),        &schedule_talks_align, locations_callback},
    {NULL, NULL},
};

SETUP_MENU(schedule_location_data,
            schedule_location_el,
            schedule_location_space,
            schedule_location_vsb,
            schedule_location_list,
            schedule_location_hlist,
            schedule_location_align);

ScheduleDay LAST_SELECTED_DAY = SCHEDULE_FRIDAY;
LocationId LAST_SELECTED_LOCATION = LOCATION_STAGE_A;

const char* talks_callback(uint8_t talk, uint8_t msg) {
    debug::log("Talks: " + String(talk) + " " + String(msg));
    schedule_label_talk_str = schedule_talks_data[talk].label;

    Event* event = schedule_talks_events[talk];
    RTC_date_time start = RTC_clock::from_unixtime(event->startTimestamp + TIMEZONE_OFFSET);
    RTC_date_time end = RTC_clock::from_unixtime(event->endTimestamp + TIMEZONE_OFFSET);

    debug::log("start: " + String(start.hour) + ":" + String(start.minute));

    return "";
}

Schedule* schedule = NULL;

const char* locations_callback(uint8_t location, uint8_t msg) {
    debug::log("Location: " + String(location) + " " + String(msg));
    LAST_SELECTED_LOCATION = (LocationId)location;

    delete schedule;
    Schedule* schedule = Tilda::getDataStore().getSchedule(LAST_SELECTED_DAY, location);

    Event* events = schedule->getEvents();
    for (int i ; i < schedule->getEventCount() ; ++i) {
        m2_xmenu_entry entry;
        entry.label = events[i].title;
        entry.element = &schedule_label_talk_align;
        entry.cb = talks_callback;
        schedule_talks_data[i] = entry;
        schedule_talks_events[i] = &events[i];
    }

    // null terminate the event list
    schedule_talks_data[schedule->getEventCount()] = {NULL, NULL, NULL};

    return "";
}

const char* days_callback(uint8_t day, uint8_t msg) {
    debug::log("Days: " + String(day) + " " + String(msg));
    // Just remember what day was seleted
    LAST_SELECTED_DAY = (ScheduleDay)day;
    return "";
}

m2_xmenu_entry schedule_days_data[] = {
    {"Friday",      &schedule_location_align, days_callback},
    {"Saturday",    &schedule_location_align, days_callback},
    {"Sunday",      &schedule_location_align, days_callback},
    {NULL, NULL},
};

SETUP_MENU(schedule_days_data,
            schedule_days_el,
            schedule_days_space,
            schedule_days_vsb,
            schedule_days_list,
            schedule_days_hlist,
            schedule_days_align);

App* ScheduleApp::New() {
    return new ScheduleApp;
}

ScheduleApp::ScheduleApp() {}

ScheduleApp::~ScheduleApp() {
    delete schedule;
    schedule = NULL;
}

String ScheduleApp::getName() const {
    return "Schedule";
}

void ScheduleApp::task() {
    Tilda::getGUITask().setM2Root(&schedule_days_align);

    while(true) {
        Tilda::setLedColor({2, 0, 0});
        Tilda::delay(300);
        Tilda::setLedColor({0, 2, 0});
        Tilda::delay(300);
        Tilda::setLedColor({0, 0, 2});
        Tilda::delay(300);
        Tilda::setLedColor({2, 2, 0});
        Tilda::delay(300);
        Tilda::setLedColor({0, 2, 2});
        Tilda::delay(300);
        Tilda::setLedColor({2, 0, 2});
        Tilda::delay(300);
    }
}

