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

  The glcd_Device class impliments the protocol for sending and receiving data and commands to a GLCD device.
  It uses glcd_io.h to for the io primitives and glcd_Config.h for user specific configuration.

*/

#include "glcd_Device.h"
#include "glcd_errno.h"


#include "ST7565.h"
#include "SPI.h"

#define ST7565_STARTBYTES 4

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// a handy reference to where the pages are on the screen
const uint8_t pagemap[] = { 7, 6, 5, 4, 3, 2, 1, 0 };

// a 5x7 font table
extern uint8_t PROGMEM font[];


/*
 * define the static variables declared in glcd_Device
 */

uint8_t	 glcd_Device::Inverted;
lcdCoord  glcd_Device::Coord;

uint8_t glcd_rdcache[DISPLAY_HEIGHT/8][DISPLAY_WIDTH];

glcd_Device::glcd_Device(){

}

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

void glcd_Device::SetDot(uint8_t x, uint8_t y, uint8_t color)
{
	uint8_t data;

	if((x >= DISPLAY_WIDTH) || (y >= DISPLAY_HEIGHT))
		return;

	this->GotoXY(x, y-y%8);					// read data from display memory

	data = this->ReadData();
	if(color == BLACK){
		data |= 0x01 << (y%8);				// set dot
	} else {
		data &= ~(0x01 << (y%8));			// clear dot
	}
	this->WriteData(data);					// write data back to display
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

// set pixels from upper left edge x,y to lower right edge x1,y1 to the given color
// the width of the region is x1-x + 1, height is y1-y+1

void glcd_Device::SetPixels(uint8_t x, uint8_t y,uint8_t x2, uint8_t y2, uint8_t color)
{
  uint8_t mask, pageOffset, h, i, data;
  uint8_t height = y2-y+1;
  uint8_t width = x2-x+1;

  SerialUSB.print("SetPixels: ");
	SerialUSB.print("height: ");SerialUSB.println(height);
	SerialUSB.print("width: ");SerialUSB.println(width);

	pageOffset = y%8;
  SerialUSB.print("pageOffset: ");SerialUSB.println(pageOffset);

	y -= pageOffset;
	SerialUSB.print("y: ");SerialUSB.println(y);
	mask = 0xFF;
	if(height < 8-pageOffset) {
		mask >>= (8-height);
		h = height;
	} else {
		h = 8-pageOffset;
	}
	mask <<= pageOffset;

	this->GotoXY(x, y);
	for(i=0; i < width; i++) {
		data = this->ReadData();

		if(color == BLACK) {
			data |= mask;
		} else {
			data &= ~mask;
		}

		this->WriteData(data);
	}

	while(h+8 <= height) {
		h += 8;
		y += 8;
		SerialUSB.print("x,y,color: ");SerialUSB.print(x);SerialUSB.print(",");SerialUSB.print(y);SerialUSB.print(",");SerialUSB.println(color);
		this->GotoXY(x, y);

		for(i=0; i <width; i++) {
			this->WriteData(color);
		}
	}

	if(h < height) {
		mask = ~(0xFF << (height-h));
		this->GotoXY(x, y+8);

		for(i=0; i < width; i++) {
			data = this->ReadData();

			if(color == BLACK) {
				data |= mask;
			} else {
				data &= ~mask;
			}

			this->WriteData(data);
		}
	}
}

/**
 * set current x,y coordinate on display device
 *
 * @param x X coordinate
 * @param y Y coordinate
 *
 * Sets the current pixel location to x,y.
 * x and y are relative to the 0,0 origin of the display which
 * is the upper left most pixel on the display.
 */

void glcd_Device::GotoXY(uint8_t x, uint8_t y)
{
  uint8_t chip, cmd;

  if( (x > DISPLAY_WIDTH-1) || (y > DISPLAY_HEIGHT-1) )	// exit if coordinates are not legal
  {
    return;
  }

  this->Coord.x = x;								// save new coordinates
  this->Coord.y = y;

  // As this is just working with a frame buffer that we perodicly write to the
  // LCD, we don't need to change anything

}


/**
 * Low level h/w initialization of display and AVR pins
 *
 * @param invert specifices whether display is in normal mode or inverted mode.
 *
 * This should only be called by other library code.
 *
 * It does all the low level hardware initalization of the display device.
 *
 * The optional invert parameter specifies if the display should be run in a normal
 * mode, dark pixels on light background or inverted, light pixels on a dark background.
 *
 * To specify dark pixels use the define @b NON-INVERTED and to use light pixels use
 * the define @b INVERTED
 *
 * @returns 0 when successful or non zero error code when unsucessful
 *
 * Upon successful completion of the initialization, the entire display will be cleared
 * and the x,y postion will be set to 0,0
 *
 * @note
 * This function can be called more than once
 * to re-initliaze the hardware.
 *
 */

void glcd_Device::st7565_command(uint8_t c) {
  digitalWrite(LCD_A0, LOW);
  SPI.transfer(LCD_CS,c);
}

void glcd_Device::st7565_data(uint8_t c) {
  digitalWrite(LCD_A0, HIGH);
  SPI.transfer(LCD_CS,c);
}

void glcd_Device::st7565_set_brightness(uint8_t val) {
    st7565_command(CMD_SET_VOLUME_FIRST);
    st7565_command(CMD_SET_VOLUME_SECOND | (val & 0x3f));
}


void glcd_spidma_cb(void){
    LCDDataDoneFlag = 1;
}


void glcd_Device::display(void) {
  uint8_t col, maxcol, p;

  for(p = 0; p < 8; p++) {
		st7565_command(CMD_SET_PAGE | pagemap[p]);
    // start at the beginning of the row
    col = 0;
    maxcol = DISPLAY_WIDTH-1;
    st7565_command(CMD_SET_COLUMN_LOWER | ((col+ST7565_STARTBYTES) & 0xf));
    st7565_command(CMD_SET_COLUMN_UPPER | (((col+ST7565_STARTBYTES) >> 4) & 0x0F));
    st7565_command(CMD_RMW);
/*
    for(; col <= maxcol; col++) {
      st7565_data(glcd_rdcache[pagemap[p]][col]);
    }
  */
      digitalWrite(LCD_A0, HIGH);
      //memcpy(xferBuffer, glcd_rdcache[pagemap[p]], 128);
      SPI.configureDMA();
      SPI.transferDMA(LCD_CS, glcd_rdcache[pagemap[p]], xferBuffer, 128, SPI_LAST);
      while (LCDDataDoneFlag == 0) {
              delay(10);
      }
      LCDDataDoneFlag = 0;
  
      
      
  }
}


int glcd_Device::Init(uint8_t invert)
{
  // set pin directions
  pinMode(LCD_A0, OUTPUT);
  pinMode(LCD_RESET, OUTPUT);
  pinMode(LCD_CS, OUTPUT);

  // Reset Sequence LCD must not be selected, but in command mode
  digitalWrite(LCD_A0, LOW ) ;
  digitalWrite(LCD_CS, HIGH);

  digitalWrite(LCD_RESET, LOW);
  delay(200);
  digitalWrite(LCD_RESET, HIGH);
  digitalWrite(LCD_CS, LOW);

  // Setup Hardware SPI
  SPI.begin(LCD_CS);
  SPI.setBitOrder(LCD_CS, MSBFIRST);
  SPI.registerDMACallback(glcd_spidma_cb);

  // LCD bias select
  st7565_command(CMD_SET_BIAS_9);
  // ADC select
  st7565_command(CMD_SET_ADC_REVERSE);
  // SHL select
  st7565_command(CMD_SET_COM_NORMAL);

  // Static Off
  st7565_command(CMD_SET_STATIC_OFF);
  // Initial display line
  st7565_command(CMD_SET_DISP_START_LINE);

  // turn on voltage converter (VC=1, VR=0, VF=0)
  st7565_command(CMD_SET_POWER_CONTROL | 0x4);
  // wait for 50% rising
  delay(50);

  // turn on voltage regulator (VC=1, VR=1, VF=0)
  st7565_command(CMD_SET_POWER_CONTROL | 0x6);
  // wait >=50ms
  delay(50);

  // turn on voltage follower (VC=1, VR=1, VF=1)
  st7565_command(CMD_SET_POWER_CONTROL | 0x7);
  // wait
  delay(10);

  // set lcd operating voltage (regulator resistor, ref voltage resistor)
  st7565_command(CMD_SET_RESISTOR_RATIO | 0x7);


  // Library Initialisation
	this->Coord.x = 0;
	this->Coord.y = 0;

	this->Inverted = invert;

  st7565_command(CMD_SET_ALLPTS_NORMAL);
  st7565_command(CMD_DISPLAY_ON);
  //st7565_set_brightness(contrast);
  st7565_set_brightness(0x08);

	/*
	 * All hardware initialization is complete.
	 *
	 * Now, clear the screen and home the cursor to ensure that the display always starts
	 * in an identical state after being initialized.
	 *
	 * Note: the reason that SetPixels() below always uses WHITE, is that once the
	 * the invert flag is in place, the lower level read/write code will invert data
	 * as needed.
	 * So clearing an areas to WHITE when the mode is INVERTED will set the area to BLACK
	 * as is required.
	 */

	this->SetPixels(0,0, DISPLAY_WIDTH-1,DISPLAY_HEIGHT-1, WHITE);
	this->GotoXY(0,0);
	//Test clear the buffer
	//for (int page = 0; page < 8; page++)
	//	for(int column = 0; column < 128; column++)
	//		glcd_rdcache[page][column] = 0;

  display();
	return(GLCD_ENOERR);
}

/**
 * read a data byte from display device memory
 *
 * @return the data byte at the current x,y position
 *
 * @note the current x,y location is not modified by the routine.
 *	This allows a read/modify/write operation.
 *	Code can call ReadData() modify the data then
 *  call WriteData() and update the same location.
 *
 * @see WriteData()
 */

uint8_t glcd_Device::ReadData()
{
uint8_t x, data;
	x = this->Coord.x;
	if(x >= DISPLAY_WIDTH)
	{
		return(0);
	}
  //Read from the frame buffer
	data = glcd_rdcache[this->Coord.y/8][x];

	if(this->Inverted)
	{
		data = ~data;
	}
	return(data);
}


/**
 * Write a byte to display device memory
 *
 * @param data date byte to write to memory
 *
 * The data specified is written to glcd memory at the current
 * x,y position. If the y location is not on a byte boundary, the write
 * is fragemented up into multiple writes.
 *
 * @note the full behavior of this during split byte writes
 * currently varies depending on a compile time define.
 * The code can be configured to either OR in 1 data bits or set all
 * the data bits.
 * @b TRUE_WRITE controls this behavior.
 *
 * @note the x,y address will not be the same as it was prior to this call.
 * 	The y address will remain the aame but the x address will advance by one.
 *	This allows back to writes to write sequentially through memory without having
 *	to do additional x,y positioning.
 *
 * @see ReadData()
 *
 */

void glcd_Device::WriteData(uint8_t data) {
  uint8_t displayData, yOffset, chip;
  //showHex("wrData",data);
  //showXY("wr", this->Coord.x,this->Coord.y);

  if(this->Coord.x >= DISPLAY_WIDTH) {
    return;
  }

  yOffset = this->Coord.y%8;

  if(yOffset != 0) {
    // first page
    displayData = this->ReadData();

    /*
    * Strip out bits we need to update.
    */
    //displayData &= (_BV(yOffset)-1);

    displayData |= data << yOffset;

    if(this->Inverted) {
      displayData = ~displayData;
    }
		SerialUSB.print("Page 1 ");
		SerialUSB.print("y: ");
		SerialUSB.print(this->Coord.y);
		SerialUSB.print(" x: ");
		SerialUSB.print(this->Coord.x);
		SerialUSB.print(" data: ");
		SerialUSB.println(displayData,HEX);

    glcd_rdcache[this->Coord.y/8][this->Coord.x] = displayData; // save to read cache

    // second page

    /*
    * Make sure to goto y address of start of next page
    * and ensure that we don't fall off the bottom of the display.
    */
    uint8_t ysave = this->Coord.y;
    if(((ysave+8) & ~7) >= DISPLAY_HEIGHT) {
      this->GotoXY(this->Coord.x+1, ysave);
      return;
    }

    this->GotoXY(this->Coord.x, ((ysave+8) & ~7));

    displayData = this->ReadData();

    /*
    * Strip out bits we need to update.
    */
    //displayData &= ~(_BV(yOffset)-1);

    displayData |= data >> (8-yOffset);
    if(this->Inverted){
      displayData = ~displayData;
    }
		SerialUSB.print("Page 2 ");
		SerialUSB.print("y: ");
		SerialUSB.print(this->Coord.y);
		SerialUSB.print(" x: ");
		SerialUSB.print(this->Coord.x);
		SerialUSB.print(" data: ");
		SerialUSB.println(displayData,HEX);

    glcd_rdcache[this->Coord.y/8][this->Coord.x] = displayData; // save to read cache
    this->GotoXY(this->Coord.x+1, ysave);
  } else {

    // just this code gets executed if the write is on a single page
    if(this->Inverted) {
      data = ~data;
    }
    glcd_rdcache[this->Coord.y/8][this->Coord.x] = data; // save to read cache

    /*
    * NOTE/WARNING:
    * This bump can cause the s/w X coordinate to bump beyond a legal value
    * for the display. This is allowed because after writing to the display
    * display, the column (x coordinate) is always bumped. However,
    * when writing to the the very last column, the resulting column location
    * inside the hardware is somewhat undefined.
    * Some chips roll it back to 0, some stop the maximu of the LCD, and others
    * advance further as the chip supports more pixels than the LCD shows.
    *
    * So to ensure that the s/w is never indicating a column (x value) that is
    * incorrect, we allow it bump beyond the end.
    *
    * Future read/writes will not attempt to talk to the chip until this
    * condition is remedied (by a GotoXY()) and by having this somewhat
    * "invalid" value, it also ensures that the next GotoXY() will always send
    * both a set column and set page address to reposition the glcd hardware.
    */

    this->Coord.x++;
  }
}

/*
 * needed to resolve virtual print functions
 */
#if ARDUINO < 100
void glcd_Device::write(uint8_t) // for Print base class
{}
#else
size_t glcd_Device::write(uint8_t) // for Print base class
{ return(0); }
#endif
