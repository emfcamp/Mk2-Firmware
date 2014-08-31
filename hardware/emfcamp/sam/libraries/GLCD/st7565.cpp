/*
TiLDA Mk2

st7565.cpp

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

// some of this code was written by <cstone@pobox.com> originally; it is in the
public domain.

The functions affected are:

* void _display(void)
* void _command(uint8_t c)
* void _data(uint8_t c)
* void _set_brightness(uint8_t val)
* void _init(void)

*/

#include "ST7565.h"
#include "SPI.h"
#include "glcd.h"
#include <debug.h>
#include <FreeRTOS_ARM.h>

// Use this flag to show debug information
//#define GLCD_DEBUG

QueueHandle_t glcd_Device::_updateWaiting;
SemaphoreHandle_t glcd_Device::frameBufferMutex;
uint8_t glcd_Device::_framebuffer[DEVICE_HEIGHT / 8 * DEVICE_WIDTH];
uint8_t glcd_Device::_x;
uint8_t glcd_Device::_y;

uint8_t LCDDataDoneFlag;

void glcd_Device::_spiwrite(uint8_t c) { SPI.transfer(LCD_CS, c); }

void glcd_Device::_command(uint8_t c) {
    digitalWrite(LCD_A0, LOW);
    _spiwrite(c);
}

void glcd_Device::_data(uint8_t c) {
    digitalWrite(LCD_A0, HIGH);
    _spiwrite(c);
}

void glcd_Device::_set_brightness(uint8_t val) {
    _command(CMD_SET_VOLUME_FIRST);
    _command(CMD_SET_VOLUME_SECOND | (val & 0x3f));
}

void spiDMADoneCallback(void) { LCDDataDoneFlag = 1; }

void glcd_Device::Init(void) {
    frameBufferMutex = 0;
    _updateWaiting = 0;

    pinMode(LCD_POWER, OUTPUT);
    digitalWrite(LCD_POWER, LOW);
    //pinMode(LCD_BACKLIGHT, OUTPUT);
    // set pin directions
    pinMode(LCD_A0, OUTPUT);
    pinMode(LCD_RESET, OUTPUT);
    pinMode(LCD_CS, OUTPUT);

    // Reset Sequence LCD must not be selected, but in command mode
    digitalWrite(LCD_A0, LOW);
    #ifdef LCD_ADAFRUIT
    digitalWrite(LCD_CS, LOW);
    #else
    digitalWrite(LCD_CS, HIGH);
    #endif
    digitalWrite(LCD_RESET, LOW);
    delay(200);
    digitalWrite(LCD_RESET, HIGH);
    digitalWrite(LCD_CS, HIGH);

    // Setup Hardware SPI
    SPI.begin(LCD_CS);
    SPI.setBitOrder(LCD_CS, MSBFIRST);
    SPI.configureDMA();
    SPI.registerDMACallback(spiDMADoneCallback);

#ifdef LCD_ADAFRUIT
    // LCD bias select
    _command(CMD_SET_BIAS_7);
#else
    // LCD bias select
    _command(CMD_SET_BIAS_9);
#endif

    // ADC select
    _command(CMD_SET_ADC_REVERSE);
    // SHL select
    _command(CMD_SET_COM_NORMAL);

#ifndef LCD_ADAFRUIT
    // Static Off
    _command(CMD_SET_STATIC_OFF);
#endif
    // Initial display line
    _command(CMD_SET_DISP_START_LINE);
    // turn on voltage converter (VC=1, VR=0, VF=0)
    _command(CMD_SET_POWER_CONTROL | 0x4);
    // wait for 50% rising
    delay(50);
    // turn on voltage regulator (VC=1, VR=1, VF=0)
    _command(CMD_SET_POWER_CONTROL | 0x6);
    // wait >=50ms
    delay(50);
    // turn on voltage follower (VC=1, VR=1, VF=1)
    _command(CMD_SET_POWER_CONTROL | 0x7);
    // wait
    delay(50);

#ifdef LCD_ADAFRUIT
    _command(CMD_SET_RESISTOR_RATIO | 0x6);
#else
    // set lcd operating voltage (regulator resistor, ref voltage resistor)
    _command(CMD_SET_RESISTOR_RATIO | 0x7);
#endif
    // Library Initialisation
    _x = 0;
    _y = 0;

    // Power on Display
    _command(CMD_SET_ALLPTS_NORMAL);
    _command(CMD_DISPLAY_ON);
    // st7565_set_brightness(contrast);
#ifdef LCD_ADAFRUIT
    _set_brightness(0x18);
#else
    _set_brightness(0x08);
#endif
    // Ensure display is cleared
    memset(this->_framebuffer, 0x00, sizeof(_framebuffer));
}

uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

// Never call this directly, as it has no mutex
void glcd_Device::_do_display() {
    uint8_t col, maxcol, p;
    #ifdef GLCD_DEBUG
        debug::log("[glcd_Device::_do_display()]");
        debug::logHWM();
    #endif

    static uint8_t txBuffer[1024];
    //txBuffer=(uint8_t*)malloc(1024*sizeof(uint8_t));
    switch (_rotation)
    {
        case ROTATION_0:
            memcpy(txBuffer,_framebuffer,sizeof(txBuffer));
            break;
        case ROTATION_90:
        case ROTATION_270:
        {
            uint16_t i = 0;
            memset(txBuffer,0,1024); //Clear txBuffer
            // Loop array
            for (; i < 1024; i++) {
                uint8_t x,y,nx,ny;
                uint8_t byte = _framebuffer[i];
                x = i % DEVICE_HEIGHT; // Height is width when portrait
                // Loop round byte
                uint8_t b=0;
                //debug::logByteArray(&byte,1);
                for (; b < 8; b++) {
                    y = i / DEVICE_HEIGHT * 8 + b;
                    if (_rotation == ROTATION_90) {
                        // Set the pixel rotated new x,y = DEVICE_WIDTH-y,x
                        nx = DEVICE_WIDTH-y-1;
                        ny = x;
                    } else {
                        nx = y;
                        ny = DEVICE_HEIGHT-x-1;
                    }
                    if (byte & (1 << b)) {
                        txBuffer[nx+ (ny/8)*128] |= _BV((ny%8)); // From adafruit lib
                    }
                }
            }

            break;
        }
        case ROTATION_180:
            for (uint16_t i = 0; i < 1024; i++)
            {
                txBuffer[1023-i] = reverse(_framebuffer[i]);
            }
            break;
    }

    #ifdef GLCD_DEBUG
        debug::log("[glcd_Device::_do_display()] Starting write to screen");
    #endif
    for (p = 0; p < 8; p++) {
        #ifdef GLCD_DEBUG
            debug::log("page: " + String(p) + " mapped: " + String(pagemap[p]));
        #endif
        _command(CMD_SET_PAGE | pagemap[p]);
        // start at the beginning of the row
        col = 0;
        maxcol = this->CurrentWidth() - 1;
        _command(CMD_SET_COLUMN_LOWER | ((col + ST7565_STARTBYTES) & 0xf));
        _command(CMD_SET_COLUMN_UPPER |
                 (((col + ST7565_STARTBYTES) >> 4) & 0x0F));
        _command(CMD_RMW);

        uint8_t rxBuffer[128]; //discarded
        //debug::logByteArray(txBuffer + DEVICE_WIDTH * pagemap[p], 128);
        ::LCDDataDoneFlag = 0;
        digitalWrite(LCD_A0, HIGH); // Select Data Mode
        SPI.transferDMA(LCD_CS, txBuffer + DEVICE_WIDTH  * p , rxBuffer, 128,
                        SPI_LAST);
        while (::LCDDataDoneFlag == 0) {
            vTaskDelay(10);
        }
        ::LCDDataDoneFlag = 0;
    }
    //free(txBuffer);
}

void glcd_Device::Display(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        if (frameBufferMutex == 0) {
            frameBufferMutex = xSemaphoreCreateMutex();
        }
        if (xSemaphoreTake(frameBufferMutex, (TickType_t)100) == pdTRUE) {
            _do_display();
            xSemaphoreGive(frameBufferMutex);
        } else {
        }
    } else {
        _do_display();
    }
}

// Never call this directly, as it has no mutex
void glcd_Device::_updateDisplay() {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        while (_updateWaiting == 0) {
            vTaskDelay((100 / portTICK_PERIOD_MS));
        }
        uint8_t discard = 1;
        xQueueOverwrite(_updateWaiting, &discard);
    }
}

void glcd_Device::GotoXY(uint8_t x, uint8_t y) {
    if ((x > this->CurrentWidth() - 1) ||
        (y > this->CurrentHeight() - 1)) // exit if coordinates are not legal
    {
        return;
    }
    _x = x;
    _y = y;
}

// Never call this directly, as it has no mutex
uint8_t glcd_Device::_do_ReadData() {
    if (_x >= this->CurrentWidth()) {
        return (0);
    }
    // Read from the frame buffer
    return this->_framebuffer[_y /8 * this->CurrentWidth() + _x];
}

uint8_t glcd_Device::ReadData() {
    uint8_t data;
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        if (frameBufferMutex == 0) {
            frameBufferMutex = xSemaphoreCreateMutex();
        }
        if (xSemaphoreTake(frameBufferMutex, (TickType_t)100) == pdTRUE) {
            data = _do_ReadData();
            xSemaphoreGive(frameBufferMutex);
            return data;
        } else {
        }
    } else {
        return _do_ReadData();
    }
}

// Never call this directly, as it has no mutex
void glcd_Device::_do_WriteData(uint8_t data) {
    uint8_t displayData, yOffset, chip;
    uint8_t current_width = this->CurrentWidth();
    if (_x >= current_width) {
        return;
    }

    yOffset = _y % 8;
    if (yOffset != 0) {
        // first page
        displayData = this->_framebuffer[_y / 8 * current_width + _x];

        /*
        * Strip out bits we need to update.
        */
        // displayData &= (_BV(yOffset)-1);

        displayData |= data << yOffset;
        this->_framebuffer[_y / 8 * current_width+ _x] = displayData; // save to read cache

        // second page
        /*
        * Make sure to goto y address of start of next page
        * and ensure that we don't fall off the bottom of the display.
        */
        uint8_t ysave = _y;
        if (((ysave + 8) & ~7) >= this->CurrentHeight()) {
            _x++;
            return;
        }

        //_y = ((ysave+8) & ~7);

        displayData = this->_framebuffer[_y / 8 * current_width + _x];
        /*
        * Strip out bits we need to update.
        */
        displayData &= ~(_BV(yOffset) - 1);

        displayData |= data >> (8 - yOffset);
        this->_framebuffer[_y / 8 * current_width + _x] = displayData; // save to read cache
        _x++;
        _y = ysave;
    } else {
        // just this code gets executed if the write is on a single page
        this->_framebuffer[_y / 8 * current_width + _x] = data; // save to read cache
        _x++;
    }
}

void glcd_Device::WriteData(uint8_t data) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        if (frameBufferMutex == 0) {
            frameBufferMutex = xSemaphoreCreateMutex();
        }
        if (xSemaphoreTake(frameBufferMutex, (TickType_t)100) == pdTRUE) {
            _do_WriteData(data);
            xSemaphoreGive(frameBufferMutex);
        } else {
        }
    } else {
        _do_WriteData(data);
    }
    _updateDisplay();
}

void glcd_Device::WaitForUpdate() {
    uint8_t discard;
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        if (_updateWaiting == 0) {
            _updateWaiting = xQueueCreate(1, sizeof(uint8_t));
        }
        if (_updateWaiting != 0) {
            xQueueReceive(_updateWaiting, &discard,
                          portMAX_DELAY); // Sleep until there is a message
        }
    }
}

uint8_t glcd_Device::CurrentWidth()
{
  switch(_rotation)
  {
    case ROTATION_0:
    case ROTATION_180:
      return DEVICE_WIDTH;
      break;
    default:
      return DEVICE_HEIGHT;
      break;
  }
}

uint8_t glcd_Device::CurrentHeight()
{
  switch(_rotation)
  {
    case ROTATION_0:
    case ROTATION_180:
      return DEVICE_HEIGHT;
      break;
    default:
      return DEVICE_WIDTH;
      break;
  }
}

rotation_t glcd_Device::GetRotation()
{
  return _rotation;
}

void glcd_Device::SetRotation(rotation_t rotation)
{
  _rotation = rotation;
}
