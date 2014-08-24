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

//Define S25FLx control bytes

#define WREN            0x06    /* Write Enable */
#define WRDI            0x04    /* Write Disable */
#define RDSR            0x05    /* Read Status Register */
#define WRSR            0x01    /* Write Status Register */
#define READ            0x03    /* Read Data Bytes  */
#define FAST_READ       0x0B    /* Read Data Bytes at Higher Speed */
#define FAST_READ_DO    0x3B    /* Fast read with dual output's (NOT IMPLMENTED) */
#define PP              0x02    /* Page Program  */
#define BE              0xD8    /* Block Erase (64k)  */
#define SE              0x20    /* Sector Erase (4k)  */
#define CE              0xC7    /* Erase entire chip  */
#define DP              0xB9    /* Deep Power-down  */
#define RES             0xAB    /* Release Power-down, return Device ID */
#define MID             0x90    /* Read Manufacture ID, Device ID */
#define RDID            0x9F    /* Read JEDEC: Manufacture ID, memory type ID, capacity ID */

//A great little tool for printing a byte as binary without it chopping off the leading zeros.
//from http://forum.arduino.cc/index.php/topic,46320.0.html

void printBits(byte myByte) {
    for(byte mask = 0x80; mask; mask >>= 1) {
        if(mask  & myByte)
            Serial.print('1');
        else
            Serial.print('0');
    }
}


Flash::Flash(int pin_flash_cs, int pin_flash_hold):flash_cs(pin_flash_cs),flash_hold(pin_flash_hold) {

}

void Flash::begin() {
    pinMode(this->flash_hold, OUTPUT);
    digitalWrite(this->flash_hold, HIGH);
    SPI.begin(this->flash_cs);
    SPI.setBitOrder(this->flash_cs, MSBFIRST);
    SPI.setClockDivider(this->flash_cs, 2);          // With 84MHz MCK this gives 42Mhz
    SPI.setDataMode(this->flash_cs, SPI_MODE0);
    
}

//read and return the status register.
byte Flash::stat() {                            //check status register
    SPI.transfer(this->flash_cs, RDSR, SPI_CONTINUE);
    byte s = SPI.transfer(this->flash_cs, 0);
    //  printBits(s);
    return s;
}

// use between each communication to make sure S25FLxx is ready to go.
void Flash::waitforit() {
    byte s = stat();
    while ((s & B0000001) == B00000001) {    //check if WIP bit is 1
                                          //  while (s==B00000011||s==B00000001) {
        if ((millis()-this->prev) > 1000) {
            this->prev = millis();
            Serial.print("S25FL Busy. Status register = B");
            printBits(s);
            Serial.println();
        }
        
        s = stat();
    }
    
}


// Must be done to allow erasing or writing
void Flash::write_enable() {
    SPI.transfer(this->flash_cs, WREN);
    waitforit();
    // Serial.println("write enabled");
    
}


// Erase an entire 4k sector the location is in.
// For example "erase_4k(300);" will erase everything from 0-3999.
//
// All erase commands take time. No other actions can be preformed
// while the chip is errasing except for reading the register
void Flash::erase_4k(unsigned long loc) {
    
    waitforit();
    write_enable();

    SPI.transfer(this->flash_cs, SE, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, loc >> 16, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, loc >> 8, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, loc & 0xFF);
    waitforit();
}

// Errase an entire 64_k sector the location is in.
// For example erase4k(530000) will erase everything from 524543 to 589823.

void Flash::erase_64k(unsigned long loc) {
    
    waitforit();
    write_enable();
    
    SPI.transfer(this->flash_cs, BE, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, loc >> 16, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, loc >> 8, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, loc & 0xFF);
    waitforit();
}

//errases all the memory. Can take several seconds.
void Flash::erase_all() {
    waitforit();
    write_enable();
    
    SPI.transfer(this->flash_cs, CE);
    waitforit();
    
}

// Read data from the flash chip. There is no limit "length". The entire memory can be read with one command.
//read_S25(starting location, array, number of bytes);
void Flash::read(unsigned long loc, uint8_t* array, unsigned long length) {
#ifdef SLOW_READ
    SPI.setClockDivider(this->flash_cs, 3);                         // slow down SPI to 28MHz
#endif
    
    SPI.transfer(this->flash_cs, READ, SPI_CONTINUE);               //control byte follow by location bytes
    SPI.transfer(this->flash_cs, loc >> 16, SPI_CONTINUE);          // convert the location integer to 3 bytes
    SPI.transfer(this->flash_cs, loc >> 8, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, loc & 0xff, SPI_CONTINUE);
    
    int i;
    for (i=0; i < length-1; i++) {
        array[i] = SPI.transfer(this->flash_cs, 0, SPI_CONTINUE);   // send the data (all but the last)
    }
    array[i] = SPI.transfer(this->flash_cs, 0);                   // send last one with SPI_LAST
    
#ifdef SLOW_READ
    SPI.setClockDivider(this->flash_cs, 2);                         // set SPI to 44MHz
#endif
}

// Read data from the flash chip using Fast read command. There is no limit "length". The entire memory can be read with one command.
void Flash::fast_read(unsigned long loc, uint8_t* array, unsigned long length) {
    
    SPI.transfer(this->flash_cs, READ, SPI_CONTINUE);               //control byte follow by location bytes
    SPI.transfer(this->flash_cs, loc >> 16, SPI_CONTINUE);          // convert the location integer to 3 bytes
    SPI.transfer(this->flash_cs, loc >> 8, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, loc & 0xff, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, 0, SPI_CONTINUE);                  // dummp byte
    
    int i;
    for (i=0; i < length-1; i++) {
        array[i] = SPI.transfer(this->flash_cs, 0, SPI_CONTINUE);   // send the data (all but the last)
        
    }
    array[i] = SPI.transfer(this->flash_cs, 0);                   // send last one with SPI_LAST
    
}

// Programs up to 256 bytes of data to flash chip. Data must be erased first. You cannot overwrite.
// Only one continuous page (256 Bytes) can be programmed at once so there's some
// sorcery going on here to make it not wrap around.
// It's most efficent to only program one page so if you're going for speed make sure your
// location %=0 (for example location=256, length=255.) or your length is less that the bytes remain
// in the page (location =120 , length= 135)


//write_S25(starting location, array, number of bytes);
void Flash::write(unsigned long loc, uint8_t* array, unsigned long length) {
    
    if (length>256) {
        // ***LWK*** this assumes to much, mostly that loc is page aligned or that a >255 write ends on a page boundry will fix this another day
        /*
        unsigned long reps = length >> 8;
        unsigned long length1;
        unsigned long array_count;
        unsigned long first_length;
        unsigned remainer0 = length - (256 * reps);
        unsigned long locb = loc;
        
        Serial.print("reps ");
        Serial.println(reps);
        Serial.print("remainer0 ");
        Serial.println(remainer0);
        
        
        for (int i=0; i < (reps+2); i++) {
            
            if (i == 0) {
                length1 = 256 - (locb & 0xff);
                first_length = length1;
                if (length1 == 0) { i++; }
                array_count = 0;
            }
            
            if (i > 0 && i < (reps+1)) {
                locb = first_length + loc + (256 * (i - 1));;
                
                array_count = first_length + (256 * (i - 1));
                length1 = 255;
                
            }
            
            if (i == (reps+1)) {
                locb += (256);
                array_count += 256;
                length1 = remainer0;
                if ( remainer0 == 0) {break;}
                
            }
            //Serial.print("i ");Serial.println(i);
            //Serial.print("locb ");Serial.println(locb);
            //Serial.print("length1 ");Serial.println(length1);
            //Serial.print("array_count ");Serial.println(array_count );
            
            
            write_enable();
            waitforit();
            SPI.transfer(this->flash_cs, PP, SPI_CONTINUE);
            SPI.transfer(this->flash_cs, locb>>16, SPI_CONTINUE);
            SPI.transfer(this->flash_cs, locb>>8, SPI_CONTINUE);
            SPI.transfer(this->flash_cs, locb & 0xff, SPI_CONTINUE);
            unsigned long i;
            for (i = array_count; i < (length1 + array_count - 1); i++) {
                SPI.transfer(this->flash_cs, array[i], SPI_CONTINUE); // all but the last
            }
            SPI.transfer(this->flash_cs, array[++i]); // last one
            
            waitforit();
            
            
            //Serial.println("//////////");
            
            
        }
        */
    }
    // one page or less
    if (length<=256) {

        if (256 - (loc % 0x100) < length) {             // if over flow Page end {
            byte remainer = loc & 0xff;                 // space remaing in first page before loc
            byte length1 = 256 - remainer;              // length we can write to first page
            byte length2 = length - length1;            // length to write to second page
            unsigned long page1_loc = loc;              // location in first page
            unsigned long page2_loc = loc + length1;    // location of second page (should be page aligned)
            
            write_enable();
            waitforit();
            SPI.transfer(this->flash_cs, PP, SPI_CONTINUE);
            SPI.transfer(this->flash_cs, page1_loc>>16, SPI_CONTINUE);
            SPI.transfer(this->flash_cs, page1_loc>>8, SPI_CONTINUE);
            SPI.transfer(this->flash_cs, page1_loc & 0xff, SPI_CONTINUE);
            
            int i;
            for (i=0; i < length1-1; i++) {
                SPI.transfer(this->flash_cs, array[i], SPI_CONTINUE);
            }
            SPI.transfer(this->flash_cs, array[++i]);
            
            waitforit();
            write_enable();
            
            waitforit();
            
            SPI.transfer(this->flash_cs, PP, SPI_CONTINUE);
            SPI.transfer(this->flash_cs, page2_loc>>16, SPI_CONTINUE);
            SPI.transfer(this->flash_cs, page2_loc>>8, SPI_CONTINUE);
            SPI.transfer(this->flash_cs, page2_loc & 0xff, SPI_CONTINUE);
            
            for (i = length1; i < length-1; i++) {
                SPI.transfer(this->flash_cs, array[i], SPI_CONTINUE);
            }
            SPI.transfer(this->flash_cs, array[i]);
            
            waitforit();
            //Serial.println("//////////");
            //Serial.print("remainer ");Serial.println(remainer);
            
            //Serial.print("length1 ");Serial.println(length1);
            //Serial.print("length2 ");Serial.println(length2);
            //Serial.print("page1_loc ");Serial.println(page1_loc);
            //Serial.print("page2_loc ");Serial.println(page2_loc);
            //Serial.println("//////////");
            
            
        } else {                                        // write one page (aligned) or less
            write_enable(); // Must be done before writing can commence. Erase clears it.
            waitforit();
        
            SPI.transfer(this->flash_cs, PP, SPI_CONTINUE);
            SPI.transfer(this->flash_cs, loc>>16, SPI_CONTINUE);
            SPI.transfer(this->flash_cs, loc>>8, SPI_CONTINUE);
            SPI.transfer(this->flash_cs, loc & 0xff, SPI_CONTINUE);
            
            int i;
            for (i=0; i < length-1; i++) {
                SPI.transfer(this->flash_cs, array[i], SPI_CONTINUE);
            }
            SPI.transfer(this->flash_cs, array[i]);
            
            waitforit();
        }
    }
}

//Used in conjuture with the write protect pin to protect blocks. 
//For example on the S25FL216K sending "write_reg(B00001000);" will protect 2 blocks, 30 and 31.
//See the datasheet for more. http://www.mouser.com/ds/2/380/S25FL216K_00-6756.pdf
void Flash::write_reg(byte w) {
    SPI.transfer(this->flash_cs, WRSR, SPI_CONTINUE);
    SPI.transfer(this->flash_cs, w);
}

byte Flash::read_info() {
    SPI.transfer(this->flash_cs, RDID, SPI_CONTINUE);
    //  SPI.transfer(0);
    byte m = SPI.transfer(this->flash_cs, 0, SPI_CONTINUE);
    byte t = SPI.transfer(this->flash_cs, 0, SPI_CONTINUE);
    byte c = SPI.transfer(this->flash_cs, 0);
    
    
    if (c==0) {
        Serial.println("Cannot read S25FL. Check wiring");
        return false;
    }
    
    Serial.print("Manufacturer ID: 0x");
    Serial.print(m, HEX);
    Serial.print("     Memory type: 0x");
    Serial.print(t, HEX);
    Serial.print("     Capacity: 0x");
    Serial.println(c, HEX);
    Serial.println();
    waitforit();
    return true;
}

void Flash::sleep() {
    waitforit();
    SPI.transfer(this->flash_cs, DP);
    
}

void Flash::wake() {
    SPI.transfer(this->flash_cs, RES);
}

