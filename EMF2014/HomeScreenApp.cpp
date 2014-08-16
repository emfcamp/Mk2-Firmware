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
#include "DebugTask.h"
#include "glcd.h"
#include "HomeScreenApp.h"
#include "M2tk.h"
#include "utility/m2ghglcd.h"
#include "allBitmaps.h"
#include "ButtonSubscription.h"

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

M2_2LMENU(el_2lmenu,"l5e15W49",&m2_2lmenu_first,&m2_2lmenu_cnt, m2_2lmenu_data,'+','-','\0');
M2_SPACE(el_space, "W1h1");
M2_VSB(el_vsb, "l5W2r1", &m2_2lmenu_first, &m2_2lmenu_cnt);
M2_LIST(list_2lmenu) = { &el_2lmenu, &el_space, &el_vsb };
M2_HLIST(el_hlist, NULL, list_2lmenu);
M2_ALIGN(top_el_expandable_menu, "-1|1W64H64", &el_hlist);

// m2 object and constructor
  


uint8_t m2_es_button_subscription(m2_p ep, uint8_t msg) 
{
  static ButtonSubscription* _buttons_p;
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
    if (_buttons_p == 0) {
      debug::log("[m2_es_button_subscription] Create ButtonSubscription");
      _buttons_p = new ButtonSubscription();
      _buttons_p->addButtons(UP | DOWN | LEFT | RIGHT | A | B | CENTER);
    }
  }
      
  switch(msg)
  {
    case M2_ES_MSG_GET_KEY:
    {
      Button button;
      if (_buttons_p) {
       button = _buttons_p->waitForPress(( TickType_t ) 1000);
       debug::log("[m2_es_button_subscription] button pressed:" + String(button));
      } else {
        // read button state manually 
        

      }
      switch (button)
      {
      case UP:
        debug::log("[m2_es_button_subscription] returning M2_KEY_DATA_UP");
        return M2_KEY_DATA_UP | M2_KEY_EVENT_MASK;
        break;
      case DOWN:
        debug::log("[m2_es_button_subscription] returning M2_KEY_DATA_DOWN");
        return M2_KEY_DATA_DOWN | M2_KEY_EVENT_MASK;
        break;
      case LEFT:
        debug::log("[m2_es_button_subscription] returning M2_KEY_DATA_PREV");
        return M2_KEY_PREV | M2_KEY_EVENT_MASK;
        break;
      case RIGHT:
        debug::log("[m2_es_button_subscription] returning M2_KEY_DATA_NEXT");
        return M2_KEY_NEXT | M2_KEY_EVENT_MASK;
        break;
      case A:
        debug::log("[m2_es_button_subscription] returning M2_KEY_DATA_SELECT");
        return M2_KEY_SELECT | M2_KEY_EVENT_MASK;
        break;
      case B:
        debug::log("[m2_es_button_subscription] returning M2_KEY_DATA_EXIT");
        return M2_KEY_EXIT | M2_KEY_EVENT_MASK;
        break;
      case CENTER:
        debug::log("[m2_es_button_subscription] returning M2_KEY_DATA_HOME");
        return M2_KEY_HOME | M2_KEY_EVENT_MASK;
        break;   
      }
      /* check, if the user has pressed a button, return M2_KEY_... */
      /* ... */
      /* return M2_KEY_NONE if there is no button pressed. */
      return M2_KEY_NONE; 
    }  
    case M2_ES_MSG_INIT:
      /* code, which will be executed once at startup of the controller */
      
      
      return 0;
  }
  return 0;
}

M2tk m2(&top_el_expandable_menu, m2_es_button_subscription, m2_eh_6bs, m2_gh_glcd_ffs);
//M2tk m2(&top_el_expandable_menu, m2_es_arduino, m2_eh_4bs, m2_gh_glcd_ffs);
String HomeScreenApp::getName() {
    return "HomeScreen";
}

void HomeScreenApp::task() {   
   /*
    m2.draw();
    while(true) {
        m2.checkKey();
        if ( m2.handleKey() ) {
            debug::log("HomeScreenApp::task() Redraw Required");
            m2.draw();
        } 
    }
    */
    GLCD.DrawBitmap(logo,0,0);
    while(true){};
}

void HomeScreenApp::afterSuspension() {}
void HomeScreenApp::beforeResume() {}
