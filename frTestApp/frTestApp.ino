/*
 TiLDA Mk2 Sample blinky

 Description of task
 Setup and manage out put to the RGB LED's

 use a queue to pass in requests for an led change
 some struct with the following
 (RGB, LED, TYPE, TIME, PRIORITY)

 block on queue for most of the time

 install interrupt handler for the LIGHT button which should pass something on the queue

 if LIGHT and IMU orientation is up show only half brightness



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

#include <Debounce.h>
#include <FreeRTOS_ARM.h>
#include <stdlib.h>
#include "globals.h"



void setup() {
    // holder for task handle
    BaseType_t t1, t2;

    // PMIC to CHARGE
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);

    pinMode(SRF_SLEEP, OUTPUT);
    digitalWrite(SRF_SLEEP, LOW);

    // setup  Serial
    Serial.begin(115200);
    delay(250);

    // create a mutex for the serial port
    xSerialMutex = xSemaphoreCreateMutex();

    // set button pins and attach interrupts
    // dose not matter if we do not define all button interrupt handlers
    tildaButtonSetup();
    tildaButtonAttachInterrupts();
    tildaButtonInterruptPriority();

    // All Tasks in this project - each has its own file
    vButtonTaskCreate();
    vBlinkTaskCreate();
    vSerialReceiverTaskCreate();
    vSerialButtonUpdaterTaskCreate();

    vSafePrint("All Tasks created");

    // start scheduler
    Serial.println("Start Scheduler");
    vTaskStartScheduler();
    Serial.println("Insufficient RAM");
    while(1);
}

// in FreeRTOS loop is use as the vApplicationIdleHook
void loop() {
}








