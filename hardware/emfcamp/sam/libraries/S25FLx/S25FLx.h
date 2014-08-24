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

class FlashClass
{
public:
    FlashClass(int pin_flash_cs, int pin_flash_hold);
    void begin();
    byte stat();
    bool write_busy();
    void wait_write();
    void write_enable();
	void erase_4k(unsigned long loc);
	void erase_64k(unsigned long loc);
	void erase_all();
	void read(unsigned long loc, uint8_t* array, unsigned long length);
    void fast_read(unsigned long loc, uint8_t* array, unsigned long length);
    void fast_read_dma(unsigned long loc, uint8_t* array, unsigned long length);
    void page_program(unsigned long loc, uint8_t* array, unsigned long length);
	void write_reg(byte w);
	bool read_info();
    void sleep();
    void wake();
    
private:
    unsigned long prev;
    int flash_cs;
    int flash_hold;
};

extern FlashClass Flash;

#endif
