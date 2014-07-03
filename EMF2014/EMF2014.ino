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

#include <FreeRTOS_ARM.h>
#include "EMF2014Config.h"
#include "DebugTask.h"
#include "RGBTask.h"
#include "ButtonTask.h"
#include "FlashLightTask.h"
#include "TiLDAButtonInterrupts.h"


/*
 * Setup is the main entry point for an Arduino sketch.
 * Here is where we will do a lot of work in getting the system running
 * and in FreeRTOS we will start the scheduler
 */
void setup() {

    tildaButtonSetup();
    tildaButtonAttachInterrupts();
    tildaButtonInterruptPriority();

    // Background tasks
    debug::initializeTask();
    rgb::initializeTask();
    buttons::initializeTask();

    // Applications
    flashLight::initializeTask();

    // Start scheduler
    debug::log("Start Scheduler");
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

void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName ) {
    debug::log("Stack overflow detected");
}