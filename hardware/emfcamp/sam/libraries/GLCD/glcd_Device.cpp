/*
glcd_Device.cpp - Arduino library support for graphic LCDs
Copyright (c) 2009, 2010 Michael Margolis and Bill Perry

vi:ts=4

This file is part of the Arduino GLCD library.

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

The glcd_Device class impliments the protocol for sending and receiving data and
commands to a GLCD device.
It uses glcd_io.h to for the io primitives and glcd_Config.h for user specific
configuration.

*/

#include "glcd_Device.h"

#include <debug.h>
#include "ST7565.h"
#include "SPI.h"

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

glcd_Device::glcd_Device() {_rotation = ROTATION_0;}

/**
* set pixel at x,y to the given color
*
* @param x X coordinate, a value from 0 to GLCD.Width-1
* @param y Y coordinate, a value from 0 to GLCD.Heigh-1
* @param color WHITE or BLACK
*
* Sets the pixel at location x,y to the specified color.
* x and y are relative to the 0,0 origin of the display which
* is the upper left corner.
* Requests to set pixels outside the range of the display will be ignored.
*
* @note If the display has been set to INVERTED mode then the colors
* will be automically reversed.
*
*/

void glcd_Device::SetDot(uint8_t x, uint8_t y, uint8_t color) {
    uint8_t data;

    if ((x >= this->CurrentWidth()) || (y >= this->CurrentHeight()))
        return;

    debug::log("GLCD_Device: SetDot");
    if (LockFrameBuffer()) {
        this->GotoXY(x, y - y % 8); // read data from display memory

        data = _do_ReadData();
        if (color == BLACK) {
            data |= 0x01 << (y % 8); // set dot
        } else {
            data &= ~(0x01 << (y % 8)); // clear dot
        }
        _do_WriteData(data); // write data back to display

        UnlockFrameBuffer();
    }
    _updateDisplay();
}

/**
* set an area of pixels
*
* @param x X coordinate of upper left corner
* @param y Y coordinate of upper left corner
* @param x2 X coordinate of lower right corner
* @param y2 Y coordinate of lower right corner
* @param color
*
* sets the pixels an area bounded by x,y to x2,y2 inclusive
* to the specified color.
*
* The width of the area is x2-x + 1.
* The height of the area is y2-y+1
*
*
*/

// set pixels from upper left edge x,y to lower right edge x1,y1 to the given
// color
// the width of the region is x1-x + 1, height is y1-y+1

void glcd_Device::SetPixels(uint8_t x, uint8_t y, uint8_t x2, uint8_t y2,
                            uint8_t color) {
    uint8_t mask, pageOffset, h, i, data;
    uint8_t height = y2 - y + 1;
    uint8_t width = x2 - x + 1;

    pageOffset = y % 8;

    y -= pageOffset;
    mask = 0xFF;
    if (height < 8 - pageOffset) {
        mask >>= (8 - height);
        h = height;
    } else {
        h = 8 - pageOffset;
    }
    mask <<= pageOffset;

    debug::log("GLCD_Device: SetPixels");
    if (LockFrameBuffer()) {
        this->GotoXY(x, y);
        for (i = 0; i < width; i++) {
            data = _do_ReadData();

            if (color == BLACK) {
                data |= mask;
            } else {
                data &= ~mask;
            }

            _do_WriteData(data);
        }

        while (h + 8 <= height) {
            h += 8;
            y += 8;
            this->GotoXY(x, y);

            for (i = 0; i < width; i++) {
                _do_WriteData(color);
            }
        }

        if (h < height) {
            mask = ~(0xFF << (height - h));
            this->GotoXY(x, y + 8);

            for (i = 0; i < width; i++) {
                data = _do_ReadData();

                if (color == BLACK) {
                    data |= mask;
                } else {
                    data &= ~mask;
                }

                _do_WriteData(data);
            }
        }

        UnlockFrameBuffer();
    }
    _updateDisplay();
}

/*
* needed to resolve virtual print functions
*/
#if ARDUINO < 100
void glcd_Device::write(uint8_t) // for Print base class
{}
#else
size_t glcd_Device::write(uint8_t) // for Print base class
{
    return (0);
}
#endif
