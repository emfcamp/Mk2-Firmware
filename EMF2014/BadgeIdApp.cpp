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
#include "BadgeIdApp.h"
#include "Tilda.h"
#include <glcd.h>
#include <M2tk.h>
#include "GUITask.h"
#include <fonts/fixednums15x31.h>


//=================================================
// Forward declaration of the toplevel element


// M2_2LMENU definition
// Option l4 = four visible lines
// Option e15 = first column has a width of 15 pixel
// Option W43 = second column has a width of 43/64 of the display width

//M2_2LMENU(el_2lmenu,"l4e15W43",&m2_2lmenu_first,&m2_2lmenu_cnt, m2_2lmenu_data,'+','-','\0');
//M2_SPACE(el_space, "W1h1");
//M2_VSB(el_vsb, "l4W2r1", &m2_2lmenu_first, &m2_2lmenu_cnt);
//M2_LIST(list_2lmenu) = { &el_2lmenu, &el_space, &el_vsb };
//M2_HLIST(el_hlist, NULL, list_2lmenu);
//M2_ALIGN(top_el_expandable_menu, "-1|1W64H64", &el_hlist);



//M2tk m2(&badge_id_app_label, NULL, NULL, m2_gh_dogm_fbs);

App* BadgeIdApp::New() {
        return new BadgeIdApp;
}

BadgeIdApp::BadgeIdApp() {}

String BadgeIdApp::getName() const {
        return "BadgeIdApp";
}

bool BadgeIdApp::keepAlive() const {
        return false;
}

void BadgeIdApp::task() {
        if (Tilda::getSettingsStore().hasBadgeId()) {
            M2_LABEL(label0, "f0", "Your Badge ID is:");
            const char* idString =  String(" EMF" + String(Tilda::getSettingsStore().getBadgeId())).c_str();
            M2_LABEL(label1, "f15w123b1", idString);
            M2_LABEL(label2, "f0", "Register it via\nschedule.emfcamp.org");
            M2_LIST(list_dt) = {&label0, &label1, &label2};
            M2_VLIST(label_list, NULL, list_dt);
            Tilda::getGUITask().setM2Root(&label_list);
        } else {
            M2_LABEL(label, "f0", "Your Badge hasn't received\nit's badge its ID yet.\nPlease wait a while and\ncome back later.");
            Tilda::getGUITask().setM2Root(&label);
        }
        while(true) {
                Tilda::delay(300);
        }
}

void BadgeIdApp::afterSuspension() {}
void BadgeIdApp::beforeResume() {}
