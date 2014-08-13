/*
glcd_Device.h - support for specific graphical LCDs
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

the glcd_Device class impliments the protocol for sending and receiving data and commands to a GLCD device.


*/

#ifndef	GLCD_DEVICE_H
#define GLCD_DEVICE_H


#if defined WIRING
#include <WPrint.h> // used when deriving this class in Wiring
#else
#include "Print.h" // used when deriving this class in Arduino
#endif

#include <FreeRTOS_ARM.h>

#define GLCD_Device 1 // software version of this class

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// useful user constants
#define NON_INVERTED false
#define INVERTED     true

// Colors
#define BLACK				0xFF
#define WHITE				0x00


#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

#define ST7565_STARTBYTES 4

const uint8_t pagemap[] = { 7, 6, 5, 4, 3, 2, 1, 0 };

/// @cond hide_from_doxygen
typedef struct {
	uint8_t x;
	uint8_t y;
} lcdCoord;
/// @endcond

/*
* Note that all data in glcd_Device is static so that all derived instances
* (gText instances for example) share the same device state.
* Any added data fields should also be static unless there is explicit reason
* to not share the field among the instances
*/

/**
* @class glcd_Device
* @brief Low level device functions
*
*/
class glcd_Device : public Print
{
private:
	// Control functions
	#if ARDUINO < 100
	void write(uint8_t); // for Print base class
	#else
	size_t write(uint8_t); // for Print base class
	#endif
    void _command(uint8_t);
    void _data(uint8_t);
    void _set_brightness(uint8_t);
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
public:
	glcd_Device();
    void WaitForUpdate(void);
    uint8_t ReadData(void);
    void WriteData(uint8_t);
    void Init();
    void Display();

protected:
	void SetDot(uint8_t x, uint8_t y, uint8_t color);
	void SetPixels(uint8_t x, uint8_t y,uint8_t x1, uint8_t y1, uint8_t color);

	void GotoXY(uint8_t x, uint8_t y);
};

#endif
