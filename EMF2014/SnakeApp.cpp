/*
 TiLDA Mk2

 SnakeApp

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
#include "SnakeApp.h"
#include "Tilda.h"
#include <glcd.h>
#include <M2tk.h>
#include "GUITask.h"
#include <fonts/allFonts.h>


App* SnakeApp::New() {
        return new SnakeApp;
}

SnakeApp::SnakeApp() {
    mButtonSubscription = Tilda::createButtonSubscription(UP | DOWN | LEFT | RIGHT);
}

SnakeApp::~SnakeApp() {
    delete mButtonSubscription;
}

String SnakeApp::getName() const {
        return "SnakeApp";
}

bool SnakeApp::keepAlive() const {
        return false;
}

void SnakeApp::setPixel(byte x, byte y, byte color) {
    GLCD.SetPixels(x * 3, y * 3, x * 3 + 2, y * 3 + 2, color == 1 ? BLACK : WHITE);
}

uint8_t SnakeApp::highscore = 0;

#define FRAME_DURATION 150;

void SnakeApp::task() {
        while(true) {
            Snake snake(21, 33, SnakeApp::setPixel);

            Tilda::getGUITask().clearRoot();
            Tilda::delay(80);
            GLCD.ClearScreen();
            Tilda::delay(80);
            snake.render_start();

            uint8_t score = 0;

            while(!snake.game_over()) {
                uint32_t nextFrame = Tilda::millisecondsSinceBoot() + FRAME_DURATION;
                int32_t timeout;

                if (score != snake.length()) {
                    score = snake.length();
                    if (score > highscore) {
                        highscore = score;
                    }
                    GLCD.SelectFont(System5x7);
                    GLCD.CursorToXY(0, 105);
                    GLCD.print("Score:  ");
                    GLCD.print(score);
                    GLCD.CursorToXY(0, 115);
                    GLCD.print("HiScore:");
                    GLCD.print(highscore);
                }

                snake.tick();
                snake.render_start();
                snake.render_tick();

                while ((timeout = nextFrame - Tilda::millisecondsSinceBoot()) > 0) {
                    Button button = mButtonSubscription->waitForPress(timeout);
                    if (button == UP) {
                        snake.dir_up();
                    } else if (button == DOWN) {
                        snake.dir_down();
                    } else if (button == LEFT) {
                        snake.dir_left();
                    } else if (button == RIGHT) {
                        snake.dir_right();
                    }
                }
            }

            Tilda::delay(1000);
        }
}

void SnakeApp::afterSuspension() {}
void SnakeApp::beforeResume() {}



/// Snake class

Snake::Snake(byte max_x, byte max_y, set_xy_fuct set_xy)
{
  _max_x = max_x;
  _max_y = max_y;
  _set_xy = set_xy;

  // Start with the snake in the middle
  _head_x = max_x/2;
  _head_y = max_y/2;

  _dir = EAST;

  memset(_ele, 0, SNAKE_ARRAY_SIZE);
  _snake_len = 2; // Initial snake length
  _game_over = false;
  _food_x = 0;
  _food_y = 0;
  _food_timer = 0;
}

void Snake::set_dir(direction dir)
{
  // Don't allow switching to opposite direction... would be suicide
  if (dir != (~(_dir) & 3))
    _dir = dir;
}

void Snake::tick()
{
  direction mov_dir;
  byte x;
  byte y;
  byte new_sid;

  if (_game_over)
    return;

  if (_food_timer == 0)  // time to spawn food
  {
    _food_x = random(_max_x-2)+1; // don't spawn on border
    _food_y = random(_max_y-2)+1;
  }

  // move head
  next_pos(_dir, &_head_x, &_head_y);

  // Test for hitting edge
  if
    (
      (_head_x == 0)        ||
      (_head_x == _max_x-1) ||
      (_head_y == 0)        ||
      (_head_y == _max_y-1)
    )
  {
    _game_over = true;
    return;
  }

  // Test for eating food
  if (_food_x != 0)
  {
    if ((_head_x == _food_x) &&
        (_head_y == _food_y))
    {
      if (_food_timer == 0)
      {
         // food spawned on the new location of the head... try spawning again next tick
        _food_x = 0;
        _food_y = 0;
      } else
      {
        // Food eaten!
        _food_timer = 0;
        _food_x = 0;
        _food_y = 0;
        if (_snake_len < (SNAKE_ARRAY_SIZE*8)) _snake_len++;
      }
    }
  }

  x = _head_x;
  y = _head_y;

  // Move tail. Well, recalc how each element links to the next
  mov_dir = _dir;
  for (int i=0; i < _snake_len; i++)
  {
    next_pos(mov_dir, &x, &y);

    // Lookout for heading touching tail
    if ((x==_head_x) && (y==_head_y) && (i < _snake_len - 1))
      _game_over = true;

    // Check food just spawned isn't on the snake
    if (x==_food_x && y==_food_y && _food_x > 0 && _food_y > 0 && _food_timer == 0)
    {
      _food_x = 0;
      _food_y = 0;
      _food_timer = 0;
    }

    mov(mov_dir, get_element(i), &mov_dir, &new_sid);
    set_element(i, new_sid);
  }

  if (_food_x!=0)
    _food_timer = 1;
}

void Snake::mov(direction mov_dir, byte con_sid, direction *new_dir, byte *new_sid)
{
  *new_sid = mov_dir;
  *new_dir = (direction)con_sid;
}

void Snake::next_pos(direction dir, byte *x, byte *y)
{
  switch (dir)
  {
    case NORTH:
      (*y)--;
      break;

    case EAST:
      (*x)++;
      break;

    case SOUTH:
      (*y)++;
      break;

    case WEST:
      (*x)--;
      break;
  }
}

void Snake::render_start() {
    // Draw border
    line_h(0       , 0, _max_x-1);
    line_h(_max_y-1, 0, _max_x-1);
    line_v(0       , 0, _max_y-1);
    line_v(_max_x-1, 0, _max_y-1);
}

void Snake::render_tick()
{
  byte x = _head_x;
  byte y = _head_y;
  byte dir;

  // Draw head
  set_xy(x, y, 1);

  // Delete tail
  for (int i=0; i < _snake_len; i++)
 {
    dir = ~(get_element(i)) & 3;

    next_pos((direction)dir, &x, &y);
    if (i == _snake_len - 1) {
        set_xy(x, y, 0);
    }
  }

  // Draw food
  if (_food_x > 0 && _food_y > 0)
    set_xy(_food_x, _food_y, 1);
}

void Snake::line_h(byte y, byte start_x, byte end_x)
{
  for (byte x=start_x; x <= end_x; x++)
    set_xy(x, y, 1);
}

void Snake::line_v(byte x, byte start_y, byte end_y)
{
  for (byte y=start_y; y <= end_y; y++)
    set_xy(x, y, 1);
}

void Snake::set_xy(byte x, byte y, byte val)
// Call the set_xy function that was passed in when constructed
{
  _set_xy(x, y, val);
}

byte Snake::get_element(short n)
// get the two bits from _ele that releate to element n
{

  byte a = _ele[n/4] & (3 << (n%4)*2);
  a >>= (n%4)*2;

  return a;
}

void Snake::set_element(byte n, byte val)
{
  byte a = _ele[n/4];

  a &= ~(3 << (n%4)*2); // zero the relevant two bits
  a |= val << (n%4)*2;  // set new val

  _ele[n/4] = a;
}

uint8_t Snake::length() {
    return _snake_len - 1;
}
