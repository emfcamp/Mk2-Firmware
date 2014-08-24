/*
 TiLDA Mk2
 
 IMUTask
 This looks after the MPU-6050 and its DMP.
 This handles booting the DMP after power cycle
 Fetching and analysing data on IMU interrupt
 Passing IMU and orientation data onto any interested party
 
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

#pragma once

#include <Arduino.h>

#include <FreeRTOS_ARM.h>
#include "EMF2014Config.h"
#include "Task.h"
#include "AppManager.h"



// callbacks
static void IMU_interrupt(void);
static void IMU_tap_cb(unsigned char direction, unsigned char count);
static void IMU_android_orient_cb(unsigned char orientation);

class IMUTask: public Task {
public:
    String getName() const;
    EventGroupHandle_t eventGroup;
    void setOrientation(uint8_t);
    uint8_t getOrientation();
    
protected:
    void task();
private:
    // variables
    uint8_t dmp_state;
    uint8_t _orientation;

    // functions
    void setup();
    int8_t MPUSetup();
    static inline unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx);
    static unsigned short inv_row_2_scale(const signed char *row);
    int8_t enable_dmp();
    int8_t disable_dmp();
    
};

extern IMUTask imuTask;

