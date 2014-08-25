/*
 TiLDA Mk2

 SnakeApp
 Yeah!

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
#include "App.h"
#include "RGBTask.h"
#include "GUITask.h"
#include "ButtonSubscription.h"

class SnakeApp: public App {
public:
    static App* New();
    ~SnakeApp();
    String getName() const;

    static void setPixel(byte x, byte y, byte color);
protected:
private:
    SnakeApp();
    SnakeApp(const SnakeApp&);

    bool keepAlive() const;

    void task();
    void afterSuspension();
    void beforeResume();

    ButtonSubscription* mButtonSubscription;

    static uint8_t highscore;
private:
};

// Snake Class

typedef void (*set_xy_fuct)(byte,byte,byte);

#define SNAKE_ARRAY_SIZE 200 // Snake length can at most be 1+ 4x this value

class Snake
{
  public:
    Snake(byte max_x, byte max_y, set_xy_fuct set_xy);
    void dir_up()     { set_dir(NORTH); };
    void dir_down()   { set_dir(SOUTH); };
    void dir_left()   { set_dir(WEST);  };
    void dir_right()  { set_dir(EAST);  };
    void tick();
    void render_start();
    void render_tick();
    byte game_over()  {return _game_over;}
    uint8_t length();

  private:
    enum direction {NORTH=0, EAST=1, SOUTH=3, WEST=2}; // numbered such that the inverse is the opposite direction

    byte _food_x;
    byte _food_y;
    byte _food_timer; /* Counts up on each tick. 0 = spawn, 1 to FOOD_TIMEOUT = show food  */
    byte _max_x;
    byte _max_y;
    byte _head_x;
    byte _head_y;
    direction _dir;
    short _snake_len;
    byte _ele[SNAKE_ARRAY_SIZE]; // snake - 2bits per element (not including head)
    bool _game_over;
  //  void (*_set_xy)(byte x, byte y, byte val, byte invert);

    void set_xy(byte x, byte y, byte val);
    set_xy_fuct _set_xy;
    void next_pos(direction dir, byte *x, byte *y);
    void mov(direction mov_dir, byte con_sid, direction *new_dir, byte *new_sid);
    void line_h(byte y, byte start_x, byte end_x);
    void line_v(byte x, byte start_y, byte end_y);
    void set_dir(direction dir);
    byte get_element(short n);
    void set_element(byte n, byte val);
};
