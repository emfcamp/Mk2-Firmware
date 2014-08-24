/*
 Arduino S25FLx Serial Flash library
 By John-Mike Reed (Dr. Bleep) of Bleep labs
 bleeplabs.com
 
 Usage guide at github.com/BleepLabs/S25FLx/
 Datasheet for S25FL216K www.mouser.com/ds/2/380/S25FL216K_00-6756.pdf
 This library can interface with most of the S25FL family with no modifications.
 
 This free library is realeased under Creative comoms license CC BY-SA 3.0
 http://creativecommons.org/licenses/by-sa/3.0/deed.en_US
 
 Modified by LWK for TiLDA Mk2 (Due advance SPI)
 */

#include "arduino.h"
#include <SPI.h>

#ifndef S25FLx_h
#define S25FLx_h

class Flash
{
public:
    Flash(int pin_flash_cs, int pin_flash_hold);
    void begin();
    byte stat();
    void waitforit();
    void write_enable();
	void erase_4k(unsigned long loc);
	void erase_64k(unsigned long loc);
	void erase_all();
	void read(unsigned long loc, uint8_t* array, unsigned long length);
    void fast_read(unsigned long loc, uint8_t* array, unsigned long length);
    void write(unsigned long loc, uint8_t* array, unsigned long length);
	void write_reg(byte w);
	byte read_info();
    void sleep();
    void wake();
    
private:
    unsigned long prev;
    int flash_cs;
    int flash_hold;
};

#endif
