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
#include "HomeScreenApp.h"
#include "Tilda.h"
#include <glcd.h>
#include "logo.h"
#include <M2tk.h>
#include "GUITask.h"

#define HOMESCREEN_ORIENATION_CHANGE_BIT (1 << 0)

uint8_t homeScreenApp_m2_2lmenu_first;
uint8_t homeScreenApp_m2_2lmenu_cnt;

m2_xmenu_entry homeScreenApp_m2_app_list_menu[MAX_APPS+1]; //Leave space for NULL terminatior

M2_X2LMENU(el_2lmenu,"l10e1w51",&homeScreenApp_m2_2lmenu_first,&homeScreenApp_m2_2lmenu_cnt, &homeScreenApp_m2_app_list_menu,'+','-','\0');
//M2_SPACE(el_space, "w2h1");
//M2_VSB(el_vsb, "l10w4r1", &homeScreenApp_m2_2lmenu_first, &homeScreenApp_m2_2lmenu_cnt);
//M2_LIST(list_2lmenu) = { &el_2lmenu, &el_space, &el_vsb };
//M2_HLIST(el_hlist, NULL, list_2lmenu);
M2_LABELFN(el_header, "f1", HomeScreenApp::headerText);
M2_LABELFN(el_footer, "f3", HomeScreenApp::footerText);
M2_SPACE(el_vspace, "w1h2");
M2_LIST(list_vspace) = {&el_header, &el_vspace, &el_2lmenu, &el_footer};
M2_VLIST(el_vlist, NULL, list_vspace);
M2_ALIGN(homeScreenApp_m2_top_el_expandable_menu, "-1|2W64H64", &el_vlist);

App* HomeScreenApp::New() {
    return new HomeScreenApp;
}

HomeScreenApp::HomeScreenApp() {}

String HomeScreenApp::getName() const {
    return "HomeScreen";
}

bool HomeScreenApp::keepAlive() const {
    return true;
}

void HomeScreenApp::newOrientation(uint8_t orientation) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING && eventGroup != NULL) {
        xEventGroupSetBits(eventGroup,
                           HOMESCREEN_ORIENATION_CHANGE_BIT);
    }
}

const char *launch_app(uint8_t idx, uint8_t msg) {
  if ( msg == M2_STRLIST_MSG_SELECT  ) {
      Tilda::openApp(Tilda::getAppManager().getById(idx).mNew);
  }
  return "";
}

const char *HomeScreenApp::headerText(m2_rom_void_p element) {
    if (Tilda::getClock().has_been_set()) {
        int minutes = Tilda::getClock().get_minutes();
        int hours = Tilda::getClock().get_hours() + 1;
        String header;
        if (minutes < 10) {
            header = String(hours) + ":0" + String(minutes);
        } else {
            header = String(hours) + ":" + String(minutes);
        }
        char* buffer = new char[6];
        header.toCharArray(buffer, 6);
        return buffer;
    } else {
        return "Welcome!";
    }
}

const char *HomeScreenApp::footerText(m2_rom_void_p element) {

    String footer;
    if (Tilda::radioChannelIdentifier()[0] == '?') {
        footer = "            " +
                 String(Tilda::getBatteryPercent()) + "%";
    } else {
        footer = String(Tilda::radioChannelIdentifier()) +
                 " -" +
                 String(Tilda::radioRssi()) +
                 "    " +
                 String(Tilda::getBatteryPercent()) + "%";
    }
    char* buffer = new char[22];
    footer.toCharArray(buffer, 22);
    return buffer;
}

void HomeScreenApp::task() {
    eventGroup = xEventGroupCreate();
    GLCD.DrawBitmap(logo,0,8);
    //Tilda::delay(5000);

    const uint8_t app_count = Tilda::getAppManager().getAppCount();

    for (uint8_t i = 0; i < app_count; ++i) {
        uint8_t name_length = Tilda::getAppManager().getById(i).mName.length() + 1;
        char* entry_label = new char(name_length);
        Tilda::getAppManager().getById(i).mName.toCharArray(entry_label,name_length);
        homeScreenApp_m2_app_list_menu[i].label = entry_label;
        homeScreenApp_m2_app_list_menu[i].element=0;
        homeScreenApp_m2_app_list_menu[i].cb=launch_app;
    }
    homeScreenApp_m2_app_list_menu[app_count].label=0;
    homeScreenApp_m2_app_list_menu[app_count].element=0;
    homeScreenApp_m2_app_list_menu[app_count].cb=0;

    Tilda::getGUITask().setM2Root(&homeScreenApp_m2_top_el_expandable_menu);

     EventBits_t uxBits;
    while(true) {
        uxBits = xEventGroupWaitBits(eventGroup,
                                     HOMESCREEN_ORIENATION_CHANGE_BIT,
                                     pdFALSE,
                                     pdFALSE,
                                     15 * 1000 );

        if ((uxBits & HOMESCREEN_ORIENATION_CHANGE_BIT) != 0 ) {
            Orientation_t orientation = Tilda::getOrientation();
            if (orientation == ORIENTATION_HUNG) {
                GLCD.SetRotation(ROTATION_270);
                Tilda::getGUITask().clearRoot();
                GLCD.DrawBitmap(HOMESCREEN_HUNG_XBM ,0, 0);

            } else {
                GLCD.SetRotation(ROTATION_90);
                Tilda::getGUITask().setM2Root(&homeScreenApp_m2_top_el_expandable_menu);
            }

            xEventGroupClearBits(eventGroup,
                                 HOMESCREEN_ORIENATION_CHANGE_BIT);

        }
    }
}

void HomeScreenApp::afterSuspension() {}
void HomeScreenApp::beforeResume() {
    GLCD.SetRotation(ROTATION_90);
    Tilda::getGUITask().setM2Root(&homeScreenApp_m2_top_el_expandable_menu);
    // Turn of LEDs
    Tilda::setLedColor({0, 0, 0});
}
