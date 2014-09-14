/*
 TiLDA Mk2

 EMF2014
 This is the entry point for the badge firmware.
 Here we deal with the setup of all the badge features
 This includes:
 Initial Pin States
 Detecting the reset condition (power cycle, button wake from deep sleep, IMU wake from deep sleep)
 Recovering of the config or hibernation file from the Flash
 Restoring context as needed
 Starting the main TiLDATask and the FreeRTOS scheduler


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

// Reference all libraries that are used here, otherwise Arduino won't include them :(
#include <FreeRTOS_ARM.h>
#include <debug.h>
#include <Wire.h>
#include <MPU6050.h>
#include <Sha1.h>
#include <DueFlashStorage.h>
#include <TinyPacks.h>
#include <rtc_clock.h>
#include <uECC.h>
#include <SPI.h>
#include <glcd.h>
#include <M2tk.h>
#include <Arduino.h>

#include "TiLDATask.h"
#include "TiLDAButtonInterrupts.h"

// All setup of tasks is now done within the TilDATask
TiLDATask tiLDATask;

void setup() {
    randomSeed(analogRead(RANDOM_SEED_PIN));

    // Setup radio communitcation
    RADIO_SERIAL.begin(RADIO_SERIAL_BAUD);
    // Setup radio pins
    pinMode(SRF_AT_COMMAND, OUTPUT);
    pinMode(SRF_SLEEP, OUTPUT);

    // Uncomment this to wait for a serial connection before continuing startup
    // while (!SerialUSB);

    debug::setup();

    tildaButtonSetup();
    tildaButtonAttachInterrupts();
    tildaButtonInterruptPriority();

    // Uncomment to wait for a keypress before continuing startup
    // debug::waitForKey();

    GLCD.Init();
    tiLDATask.start();

    debug::log("Start Scheduler");
    // Start scheduler
    vTaskStartScheduler();

    debug::log("Insufficient RAM");
    while(1);
}

/*
 * In this Arduino implmentation of FreeRTOS the loop is used as the IDLE_HOOK
 * This runs on each iteration of the idle task, this will only run if all other task are blocked.
 */
void loop() {

}
