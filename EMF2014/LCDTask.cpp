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
#include <FreeRTOS_ARM.h>
#include <glcd.h>

LCDTask::LCDTask()
    : _enabled(true) {

}

String LCDTask::getName() const
{
  return "LCD";
}

void LCDTask::task() {
  while(true) {
    if(_enabled){
      GLCD.WaitForUpdate();
      // Write framebuffer to display
      GLCD.Display();
    }
    // Sleep for 40ms, to limit updates to 25fps
    vTaskDelay(40);
  }
}

void LCDTask::disable()
{
  _enabled=false;
}

void LCDTask::enable()
{
  _enabled=true;
}
