/*
 TiLDA Mk2 button debounce helper
 
 Original based on information from
 http://forum.arduino.cc/index.php?topic=156474.0
 
 
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

#if !defined Debounce_H
#define Debounce_H

#include <Arduino.h>

// These are in usec
// 10ms
#define DEBOUNCE_DEFAULT 1000

// 500ms or 0.5sec
#define DEBOUNCE_MAX 500000

void setDebounce(int, int);

void setDebounce(int);

void tildaButtonSetup();

void tildaButtonAttachInterrupts();

void tildaButtonDetachInterrupts();

// Define a bunch of interrupt handlers as weak so can overirde in main program
extern void buttonLightPress(void) __attribute__ ((weak));
extern void buttonScreenLeftPress(void) __attribute__ ((weak));
extern void buttonScreenRightPress(void) __attribute__ ((weak));
extern void buttonAPress(void) __attribute__ ((weak));
extern void buttonBPress(void) __attribute__ ((weak));
extern void buttonUpPress(void) __attribute__ ((weak));
extern void buttonRightPress(void) __attribute__ ((weak));
extern void buttonDownPress(void) __attribute__ ((weak));
extern void buttonLeftPress(void) __attribute__ ((weak));
extern void buttonCenterPress(void) __attribute__ ((weak));


#endif // Debounce_H