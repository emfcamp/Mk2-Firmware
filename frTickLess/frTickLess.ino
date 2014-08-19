/*
 TiLDA Mk2

 Tickless Idle test sketch

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
#include <FreeRTOS_ARM.h>

uint8_t firstLightFire = 1;

void setup() {
    // holder for task handle
    BaseType_t t1,t2;
    pinMode(SRF_SLEEP, OUTPUT);
    digitalWrite(SRF_SLEEP, LOW);
    delay(200);
    
    Serial.begin(115200);
    Serial.println("frTickLess");
    
    pinMode(PIN_LED_RXL, OUTPUT);
    
    // set button pins and attach interrupts
    // dose not matter if we do not define all button interrupt handlers
    tildaButtonSetup();
    tildaButtonAttachInterrupts();
    tildaButtonInterruptPriority();    
    
    t1 = xTaskCreate(vBlinkTask,
                     NULL,
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     1,
                     NULL);
    
    if (t1 != pdPASS) {
        // tasked didn't get created
        Serial.println("Failed to create task");
        while(1);
    }
    // start scheduler
    Serial.println("Start Scheduler");
    vTaskStartScheduler();
    
    Serial.println("Insufficient RAM");
    while(1);
}


// Idle Hook
void loop() {
}

/******************************************************************************/

void buttonLightPress(){
    if (firstLightFire) {
      firstLightFire = 0;
      return;
    }
   
}

void vPreSleepProcessing( unsigned long ulExpectedIdleTime )
{
	/* Called by the kernel before it places the MCU into a sleep mode because
	configPRE_SLEEP_PROCESSING() is #defined to vPreSleepProcessing().
	NOTE:  Additional actions can be taken here to get the power consumption
	even lower.  For example, peripherals can be turned	off here, and then back
	on again in the post sleep processing function.  For maximum power saving
	ensure all unused pins are in their lowest power state. */

	/* Avoid compiler warnings about the unused parameter. */
	( void ) ulExpectedIdleTime;
        digitalWrite(PIN_LED_RXL, LOW);
}

void vPostSleepProcessing( unsigned long ulExpectedIdleTime )
{
	/* Called by the kernel when the MCU exits a sleep mode because
	configPOST_SLEEP_PROCESSING is #defined to vPostSleepProcessing(). */

	/* Avoid compiler warnings about the unused parameter. */
	( void ) ulExpectedIdleTime;
        digitalWrite(PIN_LED_RXL, HIGH);
}


/******************************************************************************/

void vBlinkTask(void *pvParameters) {
    Serial.println("Blink Task start");
    // int to hold led state
    uint8_t state = 0;
    // enabled pin 13 led
    pinMode(PIN_LED_TXL, OUTPUT);
    while(1) {
        digitalWrite(PIN_LED_TXL, state);
        state = !state;
        delay(100);
        vTaskDelay((100/portTICK_PERIOD_MS));
    }
    
}

/******************************************************************************/

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
