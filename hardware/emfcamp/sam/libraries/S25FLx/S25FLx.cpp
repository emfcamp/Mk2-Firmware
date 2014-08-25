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
#include "S25FLx.h"

// uncomment to SLOW down SPI clock during .read cycle
//#defne SLOW_READ

// uncomment for debug logging
//#define FLASH_DEBUG

#define WREN            0x06    /* Write Enable */
#define WRDI            0x04    /* Write Disable */
#define RDSR            0x05    /* Read Status Register */
#define WRSR            0x01    /* Write Status Register */
#define READ            0x03    /* Read Data Bytes  */
#define FAST_READ       0x0B    /* Read Data Bytes at Higher Speed */
#define FAST_READ_DO    0x3B    /* Fast read with dual output's (NOT IMPLEMENTED) */
#define PP              0x02    /* Page Program  */
#define BE              0xD8    /* Block Erase (64k)  */
#define SE              0x20    /* Sector Erase (4k)  */
#define CE              0xC7    /* Erase entire chip  */
#define DP              0xB9    /* Deep Power-down  */
#define RES             0xAB    /* Release Power-down, return Device ID */
#define MID             0x90    /* Read Manufacture ID, Device ID */
#define RDID            0x9F    /* Read JEDEC: Manufacture ID, memory type ID, capacity ID */

#define WIP             0x1     /* Write In Progress */
#define WEL             0x2     /* Write Enable Latch */
#define BP0             0x4     /* Block Protect 0 */
#define BP1             0x8     /* Block Protect 1 */
#define BP2             0x10    /* Block Protect 2 */
#define BP3             0x20    /* Block Protect 3 */
#define REV             0x40    /* Reserved */
#define SRP             0x80    /* Status Register Protect */

FlashClass::FlashClass(int pin_flash_cs, int pin_flash_hold)
    : flash_cs(pin_flash_cs), flash_hold(pin_flash_hold) {

}

void FlashClass::begin() {
    pinMode(this->flash_hold, OUTPUT);
    digitalWrite(this->flash_hold, HIGH);
    SPI.begin(this->flash_cs);
    SPI.setClockDivider(this->flash_cs, 2);          // With 84MHz MCK this gives 42Mhz
    
    SPI.configureDMA();
}

void FlashClass::setCallback(void (*_cb)(void)) {
    SPI.registerDMACallback(_cb);
}

uint8_t* FlashClass::allocBuffer(uint32_t length) {
    length += 5;
    if (length > bufferLength) {
        if (commandBuffer) delete commandBuffer;
        commandBuffer = new uint8_t[length];
        bufferLength = length;
    }
    return getBuffer();
}

uint8_t* FlashClass::getBuffer() {
    return &commandBuffer[5];
}

void FlashClass::clearBuffer() {
    memset(commandBuffer, 0, bufferLength);
}

//read and return the status register.
byte FlashClass::stat() {
    SPI.transfer(this->flash_cs, RDSR, SPI_CONTINUE);
    byte s = SPI.transfer(this->flash_cs, 0, SPI_LAST);
    return s;
}

bool FlashClass::write_busy() {
    return stat() & WIP;
}

// use between each communication to make sure S25FLxx is ready to go.
// Don't use on FreeRTOS
void FlashClass::wait_write() {
    byte s = stat();
    while (s & WIP) {
#ifdef FLASH_DEBUG
        if ((millis() - this->prev) > 1000) {
            this->prev = millis();
            SerialUSB.print("S25FL Busy. Status register = 0x");
            SerialUSB.print(s, HEX);
            SerialUSB.println();
        }
#endif
        s = stat();
    }
    
}

// Must be done to allow erasing or writing
void FlashClass::write_enable() {
    SPI.transfer(this->flash_cs, WREN, SPI_LAST);
}

void FlashClass::write_disable() {
    SPI.transfer(this->flash_cs, WRDI, SPI_LAST);
}


// Erase an entire 4k sector the address is in.
// For example "erase_4k(300);" will erase everything from 0-3999.
//
// All erase commands take time. No other actions can be preformed
// while the chip is errasing except for reading the register
void FlashClass::erase_4k(uint32_t addr) {
    write_enable();

    SPI.transfer(this->flash_cs, SE, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, addr >> 16, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, addr >> 8, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, addr & 0xFF, SPI_LAST);
}

// Errase an entire 64_k sector the address is in.
// For example erase4k(530000) will erase everything from 524543 to 589823.

void FlashClass::erase_64k(uint32_t addr) {
    write_enable();
    
    SPI.transfer(this->flash_cs, BE, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, addr >> 16, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, addr >> 8, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, addr & 0xFF, SPI_LAST);
}

//erases all the memory. Can take several seconds.
void FlashClass::erase_all() {
    write_enable();
    
    SPI.transfer(this->flash_cs, CE, SPI_LAST);
}

// Read data from the flash chip. There is no limit "length". The entire memory can be read with one command.
//read_S25(starting address, buf, number of bytes);
void FlashClass::read(uint32_t addr, uint8_t* buf, uint32_t length) {
#ifdef SLOW_READ
    SPI.setClockDivider(this->flash_cs, 3);                         // slow down SPI to 28MHz
#endif
    
    SPI.transfer(this->flash_cs, READ, SPI_CONTINUE);               //control byte follow by address bytes
    SPI.transfer(this->flash_cs, addr >> 16, SPI_CONTINUE);          // convert the address integer to 3 bytes
    SPI.transfer(this->flash_cs, addr >> 8, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, addr & 0xff, SPI_CONTINUE);
    
    int i;
    for (i = 0; i < length - 1; i++) {
        buf[i] = SPI.transfer(this->flash_cs, 0, SPI_CONTINUE);
    }
    buf[i] = SPI.transfer(this->flash_cs, 0, SPI_LAST);
    
#ifdef SLOW_READ
    SPI.setClockDivider(this->flash_cs, 2);                         // set SPI to 42MHz
#endif
}

// Read data from the flash chip using Fast read command. There is no limit "length". The entire memory can be read with one command.
void FlashClass::fast_read(uint32_t addr, uint8_t* buf, uint32_t length) {
    
    SPI.transfer(this->flash_cs, FAST_READ, SPI_CONTINUE);               //control byte follow by address bytes
    SPI.transfer(this->flash_cs, addr >> 16, SPI_CONTINUE);          // convert the address integer to 3 bytes
    SPI.transfer(this->flash_cs, addr >> 8, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, addr & 0xff, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, 0, SPI_CONTINUE);                  // dummp byte
    
    int i;
    for (i = 0; i < length - 1; i++) {
        buf[i] = SPI.transfer(this->flash_cs, 0, SPI_CONTINUE);
        
    }
    buf[i] = SPI.transfer(this->flash_cs, 0, SPI_LAST);
    
}

void FlashClass::fast_read_dma(uint32_t addr, uint16_t length) {

    allocBuffer(length);

    uint8_t* buf = commandBuffer;

    buf[0] = FAST_READ;

    addr &= 0x1fffff;
    buf[1] = addr >> 16;
    buf[2] = addr >> 8;
    buf[3] = addr & 0xff;

    buf[4] = 0;

    SPI.transferDMA(this->flash_cs, buf, buf, 5 + length, SPI_LAST);
}

// Programs up to 256 bytes of data to flash chip. Data must be erased first. You cannot overwrite.
// Only one continuous page (256 Bytes) can be programmed at once
// It's most efficent to only program one page so if you're going for speed make sure your
// address %=0 (for example address=256, length=255.) or your length is less that the bytes remain
// in the page (address =120 , length= 135)

void FlashClass::page_program(uint32_t addr, uint8_t* buf, uint32_t length) {
    write_enable(); // Must be done before writing can commence. Erase clears it.

    SPI.transfer(this->flash_cs, PP, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, addr>>16, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, addr>>8, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, addr & 0xff, SPI_CONTINUE);
    
    int i;
    for (i = 0; i < length - 1; i++) {
        SPI.transfer(this->flash_cs, buf[i], SPI_CONTINUE);
    }
    SPI.transfer(this->flash_cs, buf[i], SPI_LAST);
}

void FlashClass::page_program_dma(uint32_t addr, uint16_t length) {
    write_enable();

    uint8_t* buf = &commandBuffer[1];

    buf[0] = PP;

    addr &= 0x1fffff;
    buf[1] = addr >> 16;
    buf[2] = addr >> 8;
    buf[3] = addr & 0xff;

    SPI.transferDMA(this->flash_cs, buf, buf, 4 + length, SPI_LAST);
}

//Used in conjuture with the write protect pin to protect blocks. 
//For example on the S25FL216K sending "write_reg(B00001000);" will protect 2 blocks, 30 and 31.
//See the datasheet for more. http://www.mouser.com/ds/2/380/S25FL216K_00-6756.pdf
void FlashClass::write_reg(byte w) {
    SPI.transfer(this->flash_cs, WRSR, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, w, SPI_LAST);
}

bool FlashClass::read_info() {
    SPI.transfer(this->flash_cs, RDID, SPI_CONTINUE);
    byte manufacturer = SPI.transfer(this->flash_cs, 0, SPI_CONTINUE);
    byte type = SPI.transfer(this->flash_cs, 0, SPI_CONTINUE);
    byte capacity = SPI.transfer(this->flash_cs, 0, SPI_LAST);

    if (capacity == 0) {
        return false;
    }
    
#ifdef FLASH_DEBUG
    SerialUSB.print("Manufacturer ID: 0x");
    SerialUSB.print(manufacturer, HEX);
    SerialUSB.print("     Memory type: 0x");
    SerialUSB.print(type, HEX);
    SerialUSB.print("     Capacity: 0x");
    SerialUSB.println(capacity, HEX);
    SerialUSB.println();
#endif
    return true;
}

void FlashClass::sleep() {
    SPI.transfer(this->flash_cs, DP, SPI_LAST);
}

void FlashClass::wake() {
    SPI.transfer(this->flash_cs, RES, SPI_LAST);
}

FlashClass Flash(FLASH_CS, FLASH_HOLD);  //starts Flash class and initilzes SPI

