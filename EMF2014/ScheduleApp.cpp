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

Schedule* ScheduleApp::mSchedule = NULL;

uint8_t LAST_SELECTED_DAY = 0;
uint8_t LAST_SELECTED_LOCATION = 0;
uint8_t LAST_SELECTED_TALK = 0;

uint8_t first;
uint8_t cnt;

#define SETUP_MENU(data,el,space,vsb,list,hlist,align) \
M2_X2LMENU(el,"l6e1W57",&first,&cnt,data,'+','-','\0');\
M2_SPACE(space,"w1h1");\
M2_VSB(vsb,"l6W1", &first, &cnt);\
M2_LIST(list)={&el,&space,&vsb};\
M2_HLIST(hlist, NULL, list);\
M2_ALIGN(align,"-1|1W64H64",&hlist);

M2_EXTERN_ALIGN(schedule_location_align);

m2_xmenu_entry schedule_days_data[] = {
    {"Friday",      &schedule_location_align, ScheduleApp::daysCallback},
    {"Saturday",    &schedule_location_align, ScheduleApp::daysCallback},
    {"Sunday",      &schedule_location_align, ScheduleApp::daysCallback},
    {NULL, NULL},
};

SETUP_MENU(schedule_days_data,
            schedule_days_el,
            schedule_days_space,
            schedule_days_vsb,
            schedule_days_list,
            schedule_days_hlist,
            schedule_days_align);

char schedule_label_back_str[] = "<-";
char schedule_label_talk_empty_str[] = "Empty.";

#define CHARS_PER_LINE 21

#define MAX_EVENT_DESCRIPTION 160
char schedule_label_talk_str[MAX_EVENT_DESCRIPTION];
M2_BUTTON(schedule_label_talk, "r1W64", schedule_label_talk_str, ScheduleApp::talkCallback);
M2_ALIGN(schedule_label_talk_align, "-1|2W64H64", &schedule_label_talk);

#define MAX_EVENTS_FOR_LOCATION 30
#define MAX_EVENT_STRING_LENGTH 20
m2_xmenu_entry schedule_talks_data[MAX_EVENTS_FOR_LOCATION];
Event* schedule_talks_events[MAX_EVENTS_FOR_LOCATION];
char schedule_talks_strings[MAX_EVENTS_FOR_LOCATION][MAX_EVENT_STRING_LENGTH];

SETUP_MENU(schedule_talks_data,
            schedule_talks_el,
            schedule_talks_space,
            schedule_talks_vsb,
            schedule_talks_list,
            schedule_talks_hlist,
            schedule_talks_align);

m2_xmenu_entry schedule_location_data[] = {
    {Schedule::getStageName(LOCATION_STAGE_A),      &schedule_talks_align, ScheduleApp::locationsCallback},
    {Schedule::getStageName(LOCATION_STAGE_B),      &schedule_talks_align, ScheduleApp::locationsCallback},
    {Schedule::getStageName(LOCATION_STAGE_C),      &schedule_talks_align, ScheduleApp::locationsCallback},
    {Schedule::getStageName(LOCATION_WORKSHOPS),    &schedule_talks_align, ScheduleApp::locationsCallback},
    {Schedule::getStageName(LOCATION_KIDS),         &schedule_talks_align, ScheduleApp::locationsCallback},
    {Schedule::getStageName(LOCATION_VILLAGES),     &schedule_talks_align, ScheduleApp::locationsCallback},
    {Schedule::getStageName(LOCATION_LOUNGE),       &schedule_talks_align, ScheduleApp::locationsCallback},
    {Schedule::getStageName(LOCATION_BAR),          &schedule_talks_align, ScheduleApp::locationsCallback},
    {Schedule::getStageName(LOCATION_MODEL_FLYING), &schedule_talks_align, ScheduleApp::locationsCallback},
    {Schedule::getStageName(LOCATION_CATERING),     &schedule_talks_align, ScheduleApp::locationsCallback},
    {Schedule::getStageName(LOCATION_EMFFM),        &schedule_talks_align, ScheduleApp::locationsCallback},
    {Schedule::getStageName(LOCATION_OTHER),        &schedule_talks_align, ScheduleApp::locationsCallback},
    {schedule_label_back_str,                       NULL,                  ScheduleApp::locationsBackCallback},
    {NULL,                                          NULL,                  NULL}
};

SETUP_MENU(schedule_location_data,
            schedule_location_el,
            schedule_location_space,
            schedule_location_vsb,
            schedule_location_list,
            schedule_location_hlist,
            schedule_location_align);

void ScheduleApp::talkCallback(m2_el_fnarg_p fnarg) {
    Tilda::getGUITask().setM2Root(&schedule_talks_align, LAST_SELECTED_TALK, 0, false);
}

const char* ScheduleApp::talksCallback(uint8_t talk, uint8_t msg) {
    debug::log("Talks: " + String(talk) + " " + String(msg));

    LAST_SELECTED_TALK = talk;

    Event* event = schedule_talks_events[talk];

    if (event) {
        // set the text that all talk screens point to
        RTC_date_time start = RTC_clock::from_unixtime(event->startTimestamp + TIMEZONE_OFFSET);
        RTC_date_time end = RTC_clock::from_unixtime(event->endTimestamp + TIMEZONE_OFFSET);
        debug::log("start: " + String(start.hour) + ":" + String(start.minute));
        String topLineStr;
        if (start.minute < 10) {
            topLineStr = String(start.hour) + ":0" + String(start.minute) + "-" + event->speaker;
        } else {
            topLineStr = String(start.hour) + ":" + String(start.minute) + "-" + event->speaker;
        }
        String formattedStr = topLineStr.substring(0, CHARS_PER_LINE) + "\n---\n" + event->title;

        Utils::wordWrap(schedule_label_talk_str, formattedStr.c_str(), CHARS_PER_LINE, 7);
    } else {
        debug::log("there is no event to display");
        Utils::wordWrap(schedule_label_talk_str, schedule_label_talk_empty_str, CHARS_PER_LINE, 7);
    }

    return "";
}

const char* ScheduleApp::talksBackCallback(uint8_t talk, uint8_t msg) {
    Tilda::getGUITask().setM2Root(&schedule_location_align, LAST_SELECTED_LOCATION, 0, false);
}

const char* ScheduleApp::locationsCallback(uint8_t location, uint8_t msg) {
    debug::log("Location: " + String(location) + " " + String(msg));
    LAST_SELECTED_LOCATION = (LocationId)location;

    delete mSchedule;
    mSchedule = Tilda::getDataStore().getSchedule(LAST_SELECTED_DAY, location);

    Event* events = mSchedule->getEvents();
    for (int i = 0 ; i < mSchedule->getEventCount() ; ++i) {
        // Create a string for the list
        // <start time> (title)
        RTC_date_time start = RTC_clock::from_unixtime(events[i].startTimestamp + TIMEZONE_OFFSET);
        String formattedStr;
        if (start.minute < 10) {
            formattedStr = String(start.hour) + ":0" + String(start.minute) + " " + events[i].title;
        } else {
            formattedStr = String(start.hour) + ":" + String(start.minute) + " " + events[i].title;
        }
        strncpy(schedule_talks_strings[i], formattedStr.c_str(), MAX_EVENT_STRING_LENGTH - 1);
        if (strlen(events[i].title) > MAX_EVENT_STRING_LENGTH - 1) {
            schedule_talks_strings[i][MAX_EVENT_STRING_LENGTH - 2] = '~';
        }

        m2_xmenu_entry entry;
        entry.label = schedule_talks_strings[i];
        entry.element = &schedule_label_talk_align;
        entry.cb = ScheduleApp::talksCallback;
        schedule_talks_data[i] = entry;
        schedule_talks_events[i] = &events[i];
    }

    // end the list with a back button and a null termination
    schedule_talks_data[mSchedule->getEventCount()] = {schedule_label_back_str, NULL, ScheduleApp::talksBackCallback};
    schedule_talks_data[mSchedule->getEventCount() + 1] = {NULL, NULL, NULL};

    return "";
}

const char* ScheduleApp::locationsBackCallback(uint8_t location, uint8_t msg) {
    Tilda::getGUITask().setM2Root(&schedule_days_align, LAST_SELECTED_DAY, 0, false);
}

const char* ScheduleApp::daysCallback(uint8_t day, uint8_t msg) {
    debug::log("Days: " + String(day) + " " + String(msg));
    // Just remember what day was seleted
    LAST_SELECTED_DAY = (ScheduleDay)day;
    return "";
}

App* ScheduleApp::New() {
    return new ScheduleApp;
}

ScheduleApp::ScheduleApp() {
}

ScheduleApp::~ScheduleApp() {
    delete mSchedule;
    mSchedule = NULL;
}

String ScheduleApp::getName() const {
    return "Schedule";
}

bool ScheduleApp::killByPressingB() const {
    return false;
}

void ScheduleApp::task() {
    GLCD.SetRotation(ROTATION_0);
    Tilda::getGUITask().setM2Root(&schedule_days_align);

    while(true) {
        Tilda::delay(100000);
    }
}

