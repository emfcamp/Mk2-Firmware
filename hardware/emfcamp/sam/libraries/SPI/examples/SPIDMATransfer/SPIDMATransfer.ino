/*
 TiLDA Mk2 SPI DMA transfer API test
 
 Read a buffer from the S25FLx flash chip using dma
 bupasing the flash library and just using commmands directly
 
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

#include <SPI.h>

uint32_t startAddress = 0x0;

#define READ_SIZE 256

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


uint32_t flashXferDoneFlag;

uint8_t flashBuffer[5+READ_SIZE]; // size of a read or write command setup followed by READ_SIZE

void spiDMADoneCallback() {
    flashXferDoneFlag = 1;
}


void setup() {
    // PMIC to CHARGE
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);
    // Wake SRF
    pinMode(SRF_SLEEP, OUTPUT);
    digitalWrite(SRF_SLEEP, LOW);
    pinMode(PIN_LED_RXL, OUTPUT);
    digitalWrite(PIN_LED_RXL, HIGH);
    
    Serial.begin(115200);
    delay(500);
    Serial.println("TiLDA SPI DMA tests");
    
    Serial.println("Setup flash SPI");
    pinMode(FLASH_HOLD, OUTPUT);
    digitalWrite(FLASH_HOLD, HIGH);
    
    SPI.begin(FLASH_CS);
    SPI.setClockDivider(2); //42MHz
    Serial.println("Configure DMA");
    SPI.configureDMA();
    
    SPI.registerDMACallback(spiDMADoneCallback);
    
    // setup buffer with tx data 
    // for a fast read we need to set the first 5 bytes with the command, 24bit start address and blank xfer before the device will start sending back data
    Serial.println("Setup tx buffer");
    flashBuffer[0] = FAST_READ;
    
    flashBuffer[1] = startAddress >> 16;   
    flashBuffer[2] = startAddress >> 8;
    flashBuffer[3] = startAddress && 0xFF;

    flashBuffer[4] = 0;
    Serial.println("Start DMA");
    SPI.transferDMA(FLASH_CS, flashBuffer, flashBuffer, 5+READ_SIZE, SPI_LAST);
    Serial.println("Entering loop");
        
}

void loop() {
    if (flashXferDoneFlag) {
        flashXferDoneFlag = 0;
        Serial.println("Flash DMA Xfer Done");
        for (int i=0; i < 5+READ_SIZE; i++) {
            Serial.print(i);
            Serial.print(":0x");
            Serial.println(flashBuffer[i], HEX);
            delay(5);
        }
    }
    
}