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

class FlashClass
{
public:
    FlashClass(int pin_flash_cs, int pin_flash_hold);
    void begin();
    void setCallback(void (*_cb)(void));
    uint8_t* allocBuffer(uint32_t length);
    uint8_t* getBuffer();
    void clearBuffer();
    byte stat();
    bool write_busy();
    void wait_write();
    void write_enable();
    void write_disable();
    void erase_4k(uint32_t loc);
    void erase_64k(uint32_t loc);
    void erase_all();
    void read(uint32_t loc, uint8_t* array, uint32_t length);
    void fast_read(uint32_t loc, uint8_t* array, uint32_t length);
    void fast_read_dma(uint32_t loc, uint16_t length);
    void page_program(uint32_t loc, uint8_t* array, uint32_t length);
    void page_program_dma(uint32_t addr, uint16_t length);
    void write_reg(byte w);
    bool read_info();
    void sleep();
    void wake();
    
private:
    uint32_t prev;
    int flash_cs;
    int flash_hold;
    uint8_t* commandBuffer;
    uint32_t bufferLength;
};

extern FlashClass Flash;

#endif
