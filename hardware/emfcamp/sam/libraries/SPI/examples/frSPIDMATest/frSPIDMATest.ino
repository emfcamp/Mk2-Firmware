/*
 TiLDA Mk2 SPI DMA transfer API test as a FreeRTOS task
 
 Read a buffer from the S25FLx flash chip using dma
 buypasing the flash library and just using commmands directly
 
 To triger a read press the LIGHT button
 
 
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
 
#include <Debounce.h>
#include <SPI.h>
#include <FreeRTOS_ARM.h>

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

void tildaButtonInterruptPriority() {
    // reset pin interrupt handler IRQn priority levels to allow use of FreeRTOS API calls
    NVIC_DisableIRQ(PIOA_IRQn);
    NVIC_ClearPendingIRQ(PIOA_IRQn);
    NVIC_SetPriority(PIOA_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOA_IRQn);
    
    NVIC_DisableIRQ(PIOB_IRQn);
    NVIC_ClearPendingIRQ(PIOB_IRQn);
    NVIC_SetPriority(PIOB_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOB_IRQn);
    
    NVIC_DisableIRQ(PIOC_IRQn);
    NVIC_ClearPendingIRQ(PIOC_IRQn);
    NVIC_SetPriority(PIOC_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOC_IRQn);
    
    NVIC_DisableIRQ(PIOD_IRQn);
    NVIC_ClearPendingIRQ(PIOD_IRQn);
    NVIC_SetPriority(PIOD_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOD_IRQn);
}


void setup() {
    BaseType_t t1,t2;
    // PMIC to CHARGE
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);
    // Wake SRF
    pinMode(SRF_SLEEP, OUTPUT);
    digitalWrite(SRF_SLEEP, LOW);
    
    pinMode(LCD_CS, OUTPUT);
    digitalWrite(LCD_CS, HIGH);
    
    pinMode(PIN_LED_RXL, OUTPUT);
    digitalWrite(PIN_LED_RXL, HIGH);
    
    tildaButtonSetup();
    tildaButtonAttachInterrupts();
    tildaButtonInterruptPriority();
    
    
    SerialUSB.begin(115200);
    while(!SerialUSB){};
    delay(500);
    SerialUSB.println("TiLDA SPI DMA tests");
    
    
  
    t1 = xTaskCreate(vSPITest,
                     NULL,
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     2,
                     NULL);
                
    t2 = xTaskCreate(vBlinkTask,
                     NULL,
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     1,
                     NULL);
    
    if ( t2 != pdPASS) {
        // tasked didn't get created
        SerialUSB.println("Failed to create task");
        while(1);
    }
      

    // Start scheduler
    SerialUSB.println("Start Scheduler");
    vTaskStartScheduler();

    SerialUSB.println("Insufficient RAM");
    while(1);
    
         
}

void loop() {
   
}

SemaphoreHandle_t xFlashSemaphore;
void vSPITest(void *pvParameters) {
    xFlashSemaphore = xSemaphoreCreateBinary();
//    xSemaphoreTake(xFlashSemaphore, ( TickType_t ) 10 );
  
    SerialUSB.println("Setup flash SPI");
    pinMode(FLASH_HOLD, OUTPUT);
    digitalWrite(FLASH_HOLD, HIGH);
    
    SPI.begin(FLASH_CS);
    SPI.setClockDivider(2); //42MHz
    SerialUSB.println("Configure DMA");
    SPI.configureDMA();
    
    SPI.registerDMACallback(spiDMADoneCallback);
    
    
    while(1) {
        if (xSemaphoreTake(xFlashSemaphore, ( TickType_t ) 10 ) == pdTRUE) {
            // setup buffer with tx data 
            // for a fast read we need to set the first 5 bytes with the command, 24bit start address and blank xfer before the device will start sending back data
            SerialUSB.println("Setup tx buffer");
            flashBuffer[0] = FAST_READ;
            
            flashBuffer[1] = startAddress >> 16;   
            flashBuffer[2] = startAddress >> 8;
            flashBuffer[3] = startAddress && 0xFF;
        
            flashBuffer[4] = 0;
  
            SerialUSB.println("Start DMA");
            SPI.transferDMA(FLASH_CS, flashBuffer, flashBuffer, 5+READ_SIZE, SPI_LAST);
            SerialUSB.println("Entering loop");
        } else if (flashXferDoneFlag) {
            flashXferDoneFlag = 0;
            SerialUSB.println("Flash DMA Xfer Done");
            for (int i=0; i < 5+READ_SIZE; i++) {
                SerialUSB.print(i);
                SerialUSB.print(":0x");
                SerialUSB.println(flashBuffer[i], HEX);
                vTaskDelay((10/portTICK_PERIOD_MS));
            }
        }
        vTaskDelay((1000/portTICK_PERIOD_MS));
    }
        

}

// stupid attache interupt hack

uint8_t firstLightFire = 1;
void buttonLightPress(){

    if (firstLightFire) {
      firstLightFire = 0;
      return;
    }
    
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(xFlashSemaphore, &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/******************************************************************************/


void vBlinkTask(void *pvParameters) {
    SerialUSB.println("Blink Task start");
    // int to hold led state
    uint8_t state = 0;
    // enabled pin 13 led
    pinMode(PIN_LED_TXL, OUTPUT);
    while(1) {
        digitalWrite(PIN_LED_TXL, state);
        state = !state;
        
        vTaskDelay((100/portTICK_PERIOD_MS));
    }
    
}
