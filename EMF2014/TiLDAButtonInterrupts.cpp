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

#include "TiLDAButtonInterrupts.h"
#include <FreeRTOS_ARM.h>

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

void tildaButtonSetup() {
    // set button pins to input and enable pull up's
    pinMode(BUTTON_LIGHT, INPUT_PULLUP);
    pinMode(BUTTON_SCREEN_LEFT, INPUT_PULLUP);
    pinMode(BUTTON_SCREEN_RIGHT, INPUT_PULLUP);
    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_RIGHT, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
    pinMode(BUTTON_LEFT, INPUT_PULLUP);
    pinMode(BUTTON_CENTER, INPUT_PULLUP);

    // setup debounce
    setDebounce(BUTTON_LIGHT);
    setDebounce(BUTTON_SCREEN_LEFT);
    setDebounce(BUTTON_SCREEN_RIGHT);
    setDebounce(BUTTON_A);
    setDebounce(BUTTON_B);
    setDebounce(BUTTON_UP);
    setDebounce(BUTTON_RIGHT);
    setDebounce(BUTTON_DOWN);
    setDebounce(BUTTON_LEFT);
    setDebounce(BUTTON_CENTER);

}

void tildaButtonAttachInterrupts() {
    // attach button's to interrupt's
    attachInterrupt(BUTTON_LIGHT, buttonLightPress, FALLING);
    attachInterrupt(BUTTON_SCREEN_LEFT, buttonScreenLeftPress, FALLING);
    attachInterrupt(BUTTON_SCREEN_RIGHT, buttonScreenRightPress, FALLING);
    attachInterrupt(BUTTON_A, buttonAPress, FALLING);
    attachInterrupt(BUTTON_B, buttonBPress, FALLING);
    attachInterrupt(BUTTON_UP, buttonUpPress, FALLING);
    attachInterrupt(BUTTON_RIGHT, buttonRightPress, FALLING);
    attachInterrupt(BUTTON_DOWN, buttonDownPress, FALLING);
    attachInterrupt(BUTTON_LEFT, buttonLeftPress, FALLING);
    attachInterrupt(BUTTON_CENTER, buttonCenterPress, FALLING);

}

void tildaButtonDetachInterrupts() {
    // detach button's to interrupt's
    detachInterrupt(BUTTON_LIGHT);
    detachInterrupt(BUTTON_SCREEN_LEFT);
    detachInterrupt(BUTTON_SCREEN_RIGHT);
    detachInterrupt(BUTTON_A);
    detachInterrupt(BUTTON_B);
    detachInterrupt(BUTTON_UP);
    detachInterrupt(BUTTON_RIGHT);
    detachInterrupt(BUTTON_DOWN);
    detachInterrupt(BUTTON_LEFT);
    detachInterrupt(BUTTON_CENTER);

}


