/*
TiLDA Mk2

LCDTask

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

/*
Portions of this file are modified from the Arduino GLCD library
Copyright (c) 2009, 2010 Michael Margolis and Bill Perry

These are licsened under the GNU Lesser General Public License as below

GLCD is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.
GLCD is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with GLCD.  If not, see <http://www.gnu.org/licenses/>.

The functions affected are:

 * uint8_t ReadData(void);
 * void WriteData(uint8_t data);


Portions of this file are modified from ST7565 LCD library
Copyright (C) 2010 Limor Fried, Adafruit Industries

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// some of this code was written by <cstone@pobox.com> originally; it is in the public domain.

The functions affected are:

* void LCDTask::_display(void)
* void LCDTask::_command(uint8_t c)
* void LCDTask::_data(uint8_t c)
* void LCDTask::_set_brightness(uint8_t val)
* void LCDTask::_init(void)

*/

#include "LCDTask.h"
#include "ST7565.h"
#include "SPI.h"
#include "glcd.h"
#include "DebugTask.h"

LCDTask::LCDTask(){

}

void LCDTask::Init()
{
  debug::log("Starting LCD Init");
  pinMode(LCD_POWER, OUTPUT);
  digitalWrite(LCD_POWER, LOW);
  pinMode(LCD_BACKLIGHT, OUTPUT);
  digitalWrite(LCD_BACKLIGHT, LOW);

  debug::log("Call _init");
  _init();
}

void LCDTask::_command(uint8_t c) {
  //debug::log("Write command '" + String(c) + "'");

  digitalWrite(LCD_A0, LOW);
  SPI.transfer(LCD_CS,c);
}

void LCDTask::_data(uint8_t c) {
  //debug::log("Write screen data '" + String(c) + "'");
  digitalWrite(LCD_A0, HIGH);
  SPI.transfer(LCD_CS,c);
}

void LCDTask::_set_brightness(uint8_t val) {
  _command(CMD_SET_VOLUME_FIRST);
  _command(CMD_SET_VOLUME_SECOND | (val & 0x3f));
}

void LCDTask::_init(void) {
  // set pin directions
  debug::log("Set Pin Direction");
  pinMode(LCD_A0, OUTPUT);
  pinMode(LCD_RESET, OUTPUT);
  pinMode(LCD_CS, OUTPUT);

  // Reset Sequence LCD must not be selected, but in command mode
  debug::log("A0 LOW");
  digitalWrite(LCD_A0, LOW);
  
  debug::log("CS HIGH");
  digitalWrite(LCD_CS, HIGH);
  debug::logHWM();
  debug::log("RESET LOW");  
  digitalWrite(LCD_RESET, LOW);
  delay(200);
  debug::log("RESET HIGH");
  digitalWrite(LCD_RESET, HIGH);
  
  debug::log("CS HIGH");
  digitalWrite(LCD_CS, HIGH);

  debug::log("Setup SPI");
  // Setup Hardware SPI
  SPI.begin(LCD_CS);
  debug::log("Set SPI bit order");
  SPI.setBitOrder(LCD_CS, MSBFIRST);

  debug::log("Set LCD BIAS");
  // LCD bias select
  _command(CMD_SET_BIAS_9);
  
  debug::log("Set LCD ADC");
  // ADC select
  _command(CMD_SET_ADC_REVERSE);
  
   debug::log("Set LCD SHL");
  // SHL select
  _command(CMD_SET_COM_NORMAL);

  debug::log("Set LCD STATIC");
  // Static Off
  _command(CMD_SET_STATIC_OFF);
  
  debug::log("Set LCD START LINE");
  // Initial display line
  _command(CMD_SET_DISP_START_LINE);

  debug::log("LCD VC On");
  // turn on voltage converter (VC=1, VR=0, VF=0)
  _command(CMD_SET_POWER_CONTROL | 0x4);
  // wait for 50% rising
  //vTaskDelay((50/portTICK_PERIOD_MS));
  delay(50); 
  
  debug::log("LCD VR On");
  // turn on voltage regulator (VC=1, VR=1, VF=0)
  _command(CMD_SET_POWER_CONTROL | 0x6);
  // wait >=50ms
  //vTaskDelay((50/portTICK_PERIOD_MS));
  delay(50);
  
  debug::log("LCD VF On");
  // turn on voltage follower (VC=1, VR=1, VF=1)
  _command(CMD_SET_POWER_CONTROL | 0x7);
  // wait
  //vTaskDelay((10/portTICK_PERIOD_MS));
  delay(50);
  
  debug::log("LCD RESISTOR RATIO");
  // set lcd operating voltage (regulator resistor, ref voltage resistor)
  _command(CMD_SET_RESISTOR_RATIO | 0x7);


  // Library Initialisation
  _x = 0;
  _y = 0;


  debug::log("LCD ALLPTS On");
  // Power on Display
  _command(CMD_SET_ALLPTS_NORMAL);
  
  debug::log("LCD Display On");
  _command(CMD_DISPLAY_ON);
  //st7565_set_brightness(contrast);
  
  debug::log("LCD Contrast");
  _set_brightness(0x08);
  
  debug::log("Clear display ram");
  // Ensure display is cleared
  memset(_framebuffer,0xff,sizeof(_framebuffer));
  //_display();
}

void LCDTask::_display(void) {
  uint8_t col, maxcol, p;
  if (xSemaphoreTake(frameBufferMutex, ( TickType_t ) 10) == pdTRUE ) {
    for(p = 0; p < 8; p++) {
      _command(CMD_SET_PAGE | pagemap[p]);
      // start at the beginning of the row
      col = 0;
      maxcol = DISPLAY_WIDTH-1;
      _command(CMD_SET_COLUMN_LOWER | ((col+ST7565_STARTBYTES) & 0xf));
      _command(CMD_SET_COLUMN_UPPER | (((col+ST7565_STARTBYTES) >> 4) & 0x0F));
      _command(CMD_RMW);
  
      for(; col <= maxcol; col++) {
        _data(_framebuffer[pagemap[p]][col]);
      }
    }
    xSemaphoreGive(frameBufferMutex);
  }
}

void LCDTask::_updateDisplay() {
  debug::log("Queue display update");
  char discard = '*';
  xQueueOverwrite(_updateWaiting,&discard);
}

void LCDTask::GotoXY(uint8_t x, uint8_t y) {
  if( (x > DISPLAY_WIDTH-1) || (y > DISPLAY_HEIGHT-1) )	// exit if coordinates are not legal
  {
    return;
  }
  _x = x;
  _y = y;
}

uint8_t LCDTask::ReadData()
{
  uint8_t  data;
  if (xSemaphoreTake(frameBufferMutex, ( TickType_t ) 10) == pdTRUE ) {
    if(_x >= DISPLAY_WIDTH) {
      xSemaphoreGive(frameBufferMutex);
      return(0);
    }
    //Read from the frame buffer
    data = _framebuffer[_y/8][_x];
    xSemaphoreGive(frameBufferMutex);
    return(data);
  }
  return(0);
}

void LCDTask::WriteData(uint8_t data) {
  uint8_t displayData, yOffset, chip;
  if (xSemaphoreTake(frameBufferMutex, ( TickType_t ) 10) == pdTRUE ) {   
    debug::log("Write Data: " + String(data) + " x,y: " + String (_x) + "," + String(_y));
  
    if(_x >= DISPLAY_WIDTH) {
      xSemaphoreGive(frameBufferMutex);
      return;
    }
    yOffset = _y%8;
  
    if(yOffset != 0) {
      // first page
      displayData = _framebuffer[_y/8][_x];
  
      /*
      * Strip out bits we need to update.
      */
      displayData &= (_BV(yOffset)-1);
  
      displayData |= data << yOffset;
  
      _framebuffer[_y/8][_x] = displayData; // save to read cache
  
      // second page
      /*
      * Make sure to goto y address of start of next page
      * and ensure that we don't fall off the bottom of the display.
      */
      uint8_t ysave = _y;
      if(((ysave+8) & ~7) >= DISPLAY_HEIGHT) {
        _x++;
        xSemaphoreGive(frameBufferMutex);
        return;
      }
  
      _y = ((ysave+8) & ~7);
  
      displayData = _framebuffer[_y/8][_x];  
      /*
      * Strip out bits we need to update.
      */
      displayData &= ~(_BV(yOffset)-1);
  
      displayData |= data >> (8-yOffset);
  
      _framebuffer[_y/8][_x] = displayData; // save to read cache
      _x++;
      _y=ysave;
  
    } else {
  
      // just this code gets executed if the write is on a single page
      _framebuffer[_y/8][_x] = data; // save to read cache
      _x++;
    }
    //_updateDisplay();
    debug::log("WriteData end");
    xSemaphoreGive(frameBufferMutex);
  }
}

String LCDTask::getName() {
  return "LCDTask";
}

void LCDTask::task() {
  debug::log("Create mutex");
  frameBufferMutex = xSemaphoreCreateMutex();
  debug::log("create queue");
  _updateWaiting = xQueueCreate(1,sizeof(char));
  while(true) {
    char discard; // We don't actually care about the message, just that it is there
    xQueueReceive(_updateWaiting, &discard, 1000/portTICK_PERIOD_MS); // Sleep until there is a message
    debug::log("Updating display");

    // Write framebuffer to display
    _display();
    debug::log("Sleep 40ms");
    // Sleep for 40ms, to limit updates to 25fps
    vTaskDelay((40/portTICK_PERIOD_MS));
    debug::log("Loop");
  }
}
