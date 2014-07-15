/*
 TiLDA Mk2
 
 EMF2014Config
 This files contains all the initial configuration details for the badge firmware and any compli time defines that might be used by any of the task
 
 
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

#ifndef _EMF2014_CONFIG_H_
#define _EMF2014_CONFIG_H_

// Enable debug task and output
#define DEBUG 1
// send the debug output out over the USB Serial line
#define DEBUG_SERIAL SerialUSB
// define a pin that will be set to high if stopWithMessage is called
#define DEBUG_LED 10
// send the debug output to a log file on the flash
#define DEBUG_USE_FLASH 0
// This defines how many subscription each button can handle
#define MAX_BUTTON_SUBSCRIPTIONS 10
// maximum amount of apps the AppManager can handle
#define MAX_APPS 10
// Radio serial port
#define RADIO_SERIAL Serial

enum Button {
    NONE         = 0,
    LIGHT        = 1,
    A            = 2,
    B            = 4,
    UP           = 8,
    DOWN         = 16,
    LEFT         = 32,
    RIGHT        = 64,
    CENTER       = 128
};

#endif // _EMF2014_CONFIG_H_