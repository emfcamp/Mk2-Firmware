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
#include "ScheduleApp.h"
#include "Tilda.h"
#include <glcd.h>
#include "logo.h"
#include <M2tk.h>
#include "GUITask.h"
#include "Schedule.h"
#include "DataStore.h"

App* ScheduleApp::New() {
    return new ScheduleApp;
}

ScheduleApp::ScheduleApp()
    :mSchedule(NULL)
{}

ScheduleApp::~ScheduleApp() {
    if (mSchedule) {
        for (int i ; i < SCHEDULE_NUM_DAYS ; ++i) {
            delete mSchedule[i];
        }
    }

    delete[] mSchedule;

    delete mFriMenu;
    delete mSatMenu;
    delete mSunMenu;
}

String ScheduleApp::getName() const {
    return "Schedule";
}

struct Menu {
    Menu() :mLabel(NULL) {}
    ~Menu() { delete mLabel; }

    char* mLabel;

    m2_el_2lmenu_t el;
    m2_el_space_t space;
    m2_el_slbase_t vsb;
    m2_el_list_t hlist;
    m2_el_align_t align;
};

void createMenu(m2_menu_entry* menu_data, Menu& menu) {
    uint8_t first;
    uint8_t cnt;
    M2_2LMENU(el, "l10e1w51", &first, &cnt, menu_data, '+', '-', '\0');
    menu.el = el;
    M2_SPACE(space, "W1h1");
    menu.space = space;
    M2_VSB(vsb, "l10w4r1", &first, &cnt);
    menu.vsb = vsb;
    M2_LIST(list) = {&menu.el, &menu.space, &menu.vsb};
    M2_HLIST(hlist, NULL, list);
    menu.hlist = hlist;
    M2_ALIGN(align, "-1|1W64H64", &menu.hlist);
    menu.align = align;
}

// The maximum number of events per location
#define MAX_EVENTS 20

class DayMenu {
public:
    static void setEntry(m2_menu_entry& entry, const char* label, m2_rom_void_p element) {
        entry.label = label;
        entry.element = element;
    }

    static bool setLocationsEvents(const Schedule& schedule, m2_menu_entry* entries, Menu& aMenu, int locationId) {
        int eventsForLocation = 0;

        debug::log("Event count: " + String(schedule.getEventCount()));

        for (int i = 0 ; i < schedule.getEventCount() && i < MAX_EVENTS ; ++i) {
            if (schedule.getEvents()[i].locationId == locationId) {
                // the event is for this loaction so add it to the menu

                String eventTitle = schedule.getEvents()[i].speaker + " + " + schedule.getEvents()[i].title;
                aMenu.mLabel = new char[eventTitle.length()];
                eventTitle.toCharArray(aMenu.mLabel, eventTitle.length());
                setEntry(entries[eventsForLocation], aMenu.mLabel, NULL);

                eventsForLocation++;
            }
        }

        // make sure the last element is null
        setEntry(entries[eventsForLocation], NULL, NULL);

        return eventsForLocation != 0;
    }

    char* stageName(int locationId) {
        switch (locationId) {
            case LOCATION_STAGE_A:      return "Stage A";
            case LOCATION_STAGE_B:      return "Stage B";
            case LOCATION_STAGE_C:      return "Stage C";
            case LOCATION_WORKSHOPS:    return "Workshops";
            case LOCATION_KIDS:         return "Kids";
            case LOCATION_VILLAGES:     return "Villages";
            case LOCATION_LOUNGE:       return "Lounge";
            case LOCATION_BAR:          return "Bar";
            case LOCATION_MODEL_FLYING: return "Model Flying";
            case LOCATION_CATERING:     return "Catering";
            case LOCATION_EMFFM:        return "EMF.FM";
        }

        return "Unknown";
    }


    DayMenu(const Schedule& schedule) {
        debug::log("DayMenu event count: " + String(schedule.getEventCount()));

        for (int i = 0 ; i < LOCATION_COUNT ; ++i) {
            if (setLocationsEvents(schedule, mEventMenuEntries[i], mEventMenus[i], i)) {
                createMenu(mEventMenuEntries[i], mEventMenus[i]);
                setEntry(mLocationEntries[i], stageName(i), &mEventMenus[i].el);
            } else {
                setEntry(mLocationEntries[i], stageName(i), NULL);
            }
            
        }

        // set the last item to null
        setEntry(mLocationEntries[LOCATION_COUNT], NULL, NULL);
        createMenu(mLocationEntries, mLocationsMenu);
    }

    Menu mLocationsMenu;
    m2_menu_entry mLocationEntries[LOCATION_COUNT + 1];

    Menu mEventMenus[LOCATION_COUNT];
    m2_menu_entry mEventMenuEntries[LOCATION_COUNT][MAX_EVENTS + 1];
};

void ScheduleApp::task() {
    mSchedule = new Schedule*[SCHEDULE_NUM_DAYS];
    mSchedule[SCHEDULE_FRIDAY] = Tilda::getDataStore().getSchedule(SCHEDULE_FRIDAY);
    mSchedule[SCHEDULE_SATURDAY] = Tilda::getDataStore().getSchedule(SCHEDULE_SATURDAY);
    mSchedule[SCHEDULE_SUNDAY] = Tilda::getDataStore().getSchedule(SCHEDULE_SUNDAY);

    mFriMenu = new DayMenu(*mSchedule[SCHEDULE_FRIDAY]);
    mSatMenu = new DayMenu(*mSchedule[SCHEDULE_SATURDAY]);
    mSunMenu = new DayMenu(*mSchedule[SCHEDULE_SUNDAY]);

    m2_menu_entry menu_data[] =
    {
        {"Friday", &mFriMenu->mLocationsMenu.el},
        {"Saturday", &mSatMenu->mLocationsMenu.el},
        {"Sunday", &mSunMenu->mLocationsMenu.el},
        {NULL, NULL},
    };

    Menu menu;
    createMenu(menu_data, menu);

    Tilda::getGUITask().setM2Root(&menu.el);

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

