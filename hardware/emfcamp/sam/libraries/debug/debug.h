/*
 TiLDA Mk2

 DebugTask
 If enable via the compile config this task will take debug messages from any
 source and pass them onto the enabled debug outputs
 This could be via the USB Serial, a Log file to the Flash
 or morse code to the LEDs, If we have the time ;)
 Also provide a way to use SerialUSB debug out side of the RTOS
 Note only DEBUG_USB will work if the RTOS is not running and the FlashTask is yet to be started

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

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <Arduino.h>
#include <FreeRTOS_ARM.h>

// Enable debug task and output
#define DEBUG 1
// send the debug output out over the USB Serial line
#define DEBUG_SERIAL SerialUSB
// define a pin that will be set to high if stopWithMessage is called
#define DEBUG_LED 10
// send the debug output to a log file on the flash
#define DEBUG_USE_FLASH 0

namespace debug {
	void setup();
	void logByteArray(const byte hash[], int len);
	void logHWM();
    void log(String text);
    void logFromISR(String text);
    void stopWithMessage(String text);
    void waitForKey();
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName );

#endif // _DEBUG_H_
