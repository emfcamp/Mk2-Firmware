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


//=================================================
// Forward declaration of the toplevel element
M2_EXTERN_ALIGN(top_el_expandable_menu);

// Left entry: Menu name. Submenus must have a '.' at the beginning
// Right entry: Reference to the target dialog box (In this example all menus call the toplevel element again
m2_menu_entry m2_2lmenu_data[] =
{
  { "Menu 1", NULL },
  { ". Sub 1-1", &top_el_expandable_menu },
  { ". Sub 1-2", &top_el_expandable_menu },
  { "Menu 2", &top_el_expandable_menu },
  { "Menu 3", NULL },
  { ". Sub 3-1", &top_el_expandable_menu },
  { ". Sub 3-2", &top_el_expandable_menu },
  { "Menu 4", &top_el_expandable_menu },
  { "Menu 5", NULL },
  { ". Sub 5-1", &top_el_expandable_menu },
  { ". Sub 5-2", &top_el_expandable_menu },
  { ". Sub 5-3", &top_el_expandable_menu },
  { NULL, NULL },
};

// The first visible line and the total number of visible lines.
// Both values are written by M2_2LMENU and read by M2_VSB
uint8_t m2_2lmenu_first;
uint8_t m2_2lmenu_cnt;

// M2_2LMENU definition
// Option l4 = four visible lines
// Option e15 = first column has a width of 15 pixel
// Option W43 = second column has a width of 43/64 of the display width

M2_2LMENU(el_2lmenu,"l4e15W43",&m2_2lmenu_first,&m2_2lmenu_cnt, m2_2lmenu_data,'+','-','\0');
M2_SPACE(el_space, "W1h1");
M2_VSB(el_vsb, "l4W2r1", &m2_2lmenu_first, &m2_2lmenu_cnt);
M2_LIST(list_2lmenu) = { &el_2lmenu, &el_space, &el_vsb };
M2_HLIST(el_hlist, NULL, list_2lmenu);
M2_ALIGN(top_el_expandable_menu, "-1|1W64H64", &el_hlist);


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

void HomeScreenApp::task() {
    GLCD.DrawBitmap(logo,0,8);
    Tilda::delay(5000);
    Tilda::getGUITask().setM2Root(&top_el_expandable_menu);
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

void HomeScreenApp::afterSuspension() {}
void HomeScreenApp::beforeResume() {
    Tilda::getGUITask().setM2Root(&top_el_expandable_menu);
}
