/*
TiLDA Mk2

LCDTask
This task is purley responsiple with the output of framebuffer data to the EPD be it via a full or partial update.
This should only be called by the TiLDATask or the GUITask
This task use the SPI interface and has lock priority over the FlashTask for access

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

#ifndef _LCD_TASK_H_
#define _LCD_TASK_H_

#include <Arduino.h>
#include <FreeRTOS_ARM.h>
#include "EMF2014Config.h"
#include "Task.h"

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

#define ST7565_STARTBYTES 4

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// Colors
#define BLACK				0xFF
#define WHITE				0x00

const uint8_t pagemap[] = { 7, 6, 5, 4, 3, 2, 1, 0 };

class LCDTask: public Task {

private:
  // Control functions
  void _command(uint8_t);
  void _data(uint8_t);
  void _set_brightness(uint8_t);
  void _display();
  void _init();
  static uint8_t _x;
  static uint8_t _y;
  static uint8_t _framebuffer[DISPLAY_HEIGHT/8][DISPLAY_WIDTH];
  static QueueHandle_t _updateWaiting;
  void _updateDisplay();
  static SemaphoreHandle_t frameBufferMutex;
  void _do_display();
  uint8_t _do_ReadData(void);
  void _do_WriteData(uint8_t data);
  void _spiwrite(uint8_t c);
  static uint8_t LCDDataDoneFlag;

protected:

public:
  LCDTask();
  String getName();
  void task();
  void Init();
  uint8_t ReadData(void);
  void WriteData(uint8_t data);
  void GotoXY(uint8_t x, uint8_t y);
};

#endif // _LCD_TASK_H_
