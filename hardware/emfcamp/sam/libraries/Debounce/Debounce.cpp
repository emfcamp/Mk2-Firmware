/*
 TiLDA Mk2 button debounce helper
 
 Original based on information from
 http://forum.arduino.cc/index.php?topic=156474.0
 
 The MIT License (MIT)
 
 Copyright (c) 2014 Electromagnetic  Field LTD
 
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

#include "Debounce.h"

// reject spikes shorter than usecs on pin
void setDebounce(int pin, int usecs) {
    if(usecs){
        g_APinDescription[pin].pPort -> PIO_IFER = g_APinDescription[pin].ulPin;
        g_APinDescription[pin].pPort -> PIO_DIFSR |= g_APinDescription[pin].ulPin;
    }
    else {
        g_APinDescription[pin].pPort -> PIO_IFDR = g_APinDescription[pin].ulPin;
        g_APinDescription[pin].pPort -> PIO_DIFSR &=~ g_APinDescription[pin].ulPin;
        return;
    }
    
    
    int div=(usecs/31)-1; if(div<0)div=0; if(div > 16383) div=16383;
    g_APinDescription[pin].pPort -> PIO_SCDR = div;
    
}

// use default debounce if not explicitly passed
void setDebounce(int pin) {
    return setDebounce(pin, DEBOUNCE_DEFAULT);
}