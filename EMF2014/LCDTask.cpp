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

QueueHandle_t LCDTask::_updateWaiting;
SemaphoreHandle_t LCDTask::frameBufferMutex;

LCDTask::LCDTask(){

}

void LCDTask::Init()
{
  pinMode(LCD_POWER, OUTPUT);
  digitalWrite(LCD_POWER, LOW);
  pinMode(LCD_BACKLIGHT, OUTPUT);
  digitalWrite(LCD_BACKLIGHT, LOW);
  _init();
}

void LCDTask::_command(uint8_t c) {
  digitalWrite(LCD_A0, LOW);
  SPI.transfer(LCD_CS,c);
}

void LCDTask::_data(uint8_t c) {
  digitalWrite(LCD_A0, HIGH);
  SPI.transfer(LCD_CS,c);
}

void LCDTask::_set_brightness(uint8_t val) {
  _command(CMD_SET_VOLUME_FIRST);
  _command(CMD_SET_VOLUME_SECOND | (val & 0x3f));
}

void LCDTask::_init(void) {
  // set pin directions
  pinMode(LCD_A0, OUTPUT);
  pinMode(LCD_RESET, OUTPUT);
  pinMode(LCD_CS, OUTPUT);

  // Reset Sequence LCD must not be selected, but in command mode
  digitalWrite(LCD_A0, LOW);
  
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RESET, LOW);
  delay(200);
  digitalWrite(LCD_RESET, HIGH);
  
  digitalWrite(LCD_CS, HIGH);

  // Setup Hardware SPI
  SPI.begin(LCD_CS);
  SPI.setBitOrder(LCD_CS, MSBFIRST);

  // LCD bias select
  _command(CMD_SET_BIAS_9);
  
  // ADC select
  _command(CMD_SET_ADC_REVERSE);
  
  // SHL select
  _command(CMD_SET_COM_NORMAL);

  // Static Off
  _command(CMD_SET_STATIC_OFF);
  
  // Initial display line
  _command(CMD_SET_DISP_START_LINE);

  // turn on voltage converter (VC=1, VR=0, VF=0)
  _command(CMD_SET_POWER_CONTROL | 0x4);
  // wait for 50% rising
  //vTaskDelay((50/portTICK_PERIOD_MS));
  delay(50); 
  
  // turn on voltage regulator (VC=1, VR=1, VF=0)
  _command(CMD_SET_POWER_CONTROL | 0x6);
  // wait >=50ms
  //vTaskDelay((50/portTICK_PERIOD_MS));
  delay(50);
  
  // turn on voltage follower (VC=1, VR=1, VF=1)
  _command(CMD_SET_POWER_CONTROL | 0x7);
  // wait
  //vTaskDelay((10/portTICK_PERIOD_MS));
  delay(50);
  
  // set lcd operating voltage (regulator resistor, ref voltage resistor)
  _command(CMD_SET_RESISTOR_RATIO | 0x7);


  // Library Initialisation
  _x = 0;
  _y = 0;


  // Power on Display
  _command(CMD_SET_ALLPTS_NORMAL);
  
  _command(CMD_DISPLAY_ON);
  //st7565_set_brightness(contrast);
  
  _set_brightness(0x08);
  
  // Ensure display is cleared
  debug::log("[LCDTask::_init()] Address of framebuffer:" + String((long)this->_framebuffer));
  memset(this->_framebuffer,0x00,sizeof(_framebuffer));
}

// Never call this directly, as it has no mutex
void LCDTask::_do_display() {
  
  debug::log("[LCDTask::_do_display()] Address of framebuffer:" + String((long)this->_framebuffer));

  uint8_t col, maxcol, p;
  for(p = 0; p < 8; p++) {
    _command(CMD_SET_PAGE | pagemap[p]);
    // start at the beginning of the row
    col = 0;
    maxcol = DISPLAY_WIDTH-1;
    //debug::log("[LCDTask::_do_display] page: " + String(p));
    //debug::logByteArray(_framebuffer[pagemap[p]],128);    
    _command(CMD_SET_COLUMN_LOWER | ((col+ST7565_STARTBYTES) & 0xf));
    _command(CMD_SET_COLUMN_UPPER | (((col+ST7565_STARTBYTES) >> 4) & 0x0F));
    _command(CMD_RMW);
    for(; col <= maxcol; col++) {
      _data(this->_framebuffer[pagemap[p]][col]);
    }
  }  
}


void LCDTask::_display(void) {
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
    if (frameBufferMutex == 0) {
      frameBufferMutex = xSemaphoreCreateMutex();
    }
    if (xSemaphoreTake(frameBufferMutex, ( TickType_t ) 10) == pdTRUE ) {
      _do_display();
      xSemaphoreGive(frameBufferMutex);
    }
  } else {
    _do_display();  
  }
}

// Never call this directly, as it has no mutex
void LCDTask::_updateDisplay() {
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
    while(_updateWaiting==0)
    {
      vTaskDelay((100/portTICK_PERIOD_MS));
    }
    uint8_t discard = 1;
    xQueueOverwrite(_updateWaiting,&discard);  
  }
}

void LCDTask::GotoXY(uint8_t x, uint8_t y) {
  if( (x > DISPLAY_WIDTH-1) || (y > DISPLAY_HEIGHT-1) )	// exit if coordinates are not legal
  {
    return;
  }
  _x = x;
  _y = y;
}

// Never call this directly, as it has no mutex
uint8_t LCDTask::_do_ReadData(){
  debug::log("[LCDTask::_do_ReadData()] Address of framebuffer:" + String((long)this->_framebuffer));

  if(_x >= DISPLAY_WIDTH) {
    return(0);
  }
  //Read from the frame buffer
  return this->_framebuffer[_y/8][_x];
}

uint8_t LCDTask::ReadData()
{
  uint8_t  data;
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
    if (frameBufferMutex == 0) {
      frameBufferMutex = xSemaphoreCreateMutex();
    }
    if (xSemaphoreTake(frameBufferMutex, ( TickType_t ) 10) == pdTRUE ) {
      data = _do_ReadData();
      xSemaphoreGive(frameBufferMutex);
      return data;
    }
  } else {
    return _do_ReadData(); 
  }
}

// Never call this directly, as it has no mutex
void LCDTask::_do_WriteData(uint8_t data) {
  uint8_t displayData, yOffset, chip;
  debug::log("[LCDTask::_do_WriteData] Address of framebuffer:" + String((long)_framebuffer));

  
  if(_x >= DISPLAY_WIDTH) {
    return;
  }
  
  yOffset = _y%8;  
  if(yOffset != 0) {
    // first page
    displayData = this->_framebuffer[_y/8][_x];
    
    /*
    * Strip out bits we need to update.
    */
    //displayData &= (_BV(yOffset)-1);
    
    displayData |= data << yOffset;
         
    //debug::log("Write Data: " + String(displayData) + " x,page: " + String (_x) + "," + String(_y/8));  
    this->_framebuffer[_y/8][_x] = displayData; // save to read cache
    
    // second page
    /*
    * Make sure to goto y address of start of next page
    * and ensure that we don't fall off the bottom of the display.
    */
    uint8_t ysave = _y;
    if(((ysave+8) & ~7) >= DISPLAY_HEIGHT) {
      _x++;      
      return;
    }
    
    //_y = ((ysave+8) & ~7);
    
    displayData = this->_framebuffer[_y/8][_x];  
    /*
    * Strip out bits we need to update.
    */
    displayData &= ~(_BV(yOffset)-1);
    
    displayData |= data >> (8-yOffset);
    
     //debug::log("Write Data: " + String(displayData) + " x,page: " + String (_x) + "," + String(_y/8));  
    this->_framebuffer[_y/8][_x] = displayData; // save to read cache
    _x++;
    _y=ysave;
  } else {  
    // just this code gets executed if the write is on a single page
     //debug::log("Write Data: " + String(data) + " x,page: " + String (_x) + "," + String(_y/8));  
    this->_framebuffer[_y/8][_x] = data; // save to read cache
    _x++;
  }
}

void LCDTask::WriteData(uint8_t data) {
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
    if (frameBufferMutex == 0) {
      frameBufferMutex = xSemaphoreCreateMutex();
    }
    if (xSemaphoreTake(frameBufferMutex, ( TickType_t ) 10) == pdTRUE ) {  
      _do_WriteData(data);
      xSemaphoreGive(frameBufferMutex);
    }
  } else {
    _do_WriteData(data);   
  }
  _updateDisplay();
}

String LCDTask::getName() {
  return "LCDTask";
}

void LCDTask::task() {
  frameBufferMutex = 0;
  _updateWaiting = 0;
  while(true) {
    uint8_t discard; // We don't actually care about the message, just that it is there
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
      if (_updateWaiting==0) {
        _updateWaiting = xQueueCreate(1,sizeof(uint8_t));
      }
      if (_updateWaiting != 0){
        xQueueReceive(_updateWaiting, &discard, portMAX_DELAY); // Sleep until there is a message
      } else {
        debug::log("LCDTask::task() Error Creating queue");
      }
      // Write framebuffer to display
      _display();
      // Sleep for 40ms, to limit updates to 25fps
      vTaskDelay((40/portTICK_PERIOD_MS));
      } else {
      delay(100); // Scheduler not running (why are we??, wait 1/10th second)
    }
  }
}
