/*
 TiLDA Mk2
 
 IMUTask
 
 TODO: look at mpu_lp_accel_mode() and mpu_lp_motion_interrupt() for low power sleep modes
 TODO: look at pedemoter
    int dmp_get_pedometer_step_count(unsigned long *count);
    int dmp_set_pedometer_step_count(unsigned long count);
    int dmp_get_pedometer_walk_time(unsigned long *time);
    int dmp_set_pedometer_walk_time(unsigned long time);
 TODO: look at using this to lower int rate, as we may have little need from anything else
    dmp_set_interrupt_mode(DMP_INT_GESTURE);
 
 
 
 Parts of this code are borrowed from the Nav6 project by Kauai Labs which is under the MIT license
 <link>
 
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

#include "IMUTask.h"
#include <debug.h>
#include <Wire.h>
#include <MPU6050.h>
#include "Tilda.h"

// eventGroup Bits
#define IMU_INT_BIT (1 << 0)

#define IMU_DMP_ON  1
#define IMU_DMP_OFF 0

/* Starting sampling rate. */
#define IMU_DEFAULT_MPU_HZ    (4)


/* 
 * This is not documented well in the invenses stuff but managed to work it out.
 * Matrix is aranged in a grid with real XYZ in the coll's and mapped XYZ in the rows
 * Positve 1 means in the same direction from the chip
 * Negative -1 means in the oppsite direction
 *
 * Chip default
 *   X  Y  Z
 * { 1, 0, 0,   X
 *   0, 1, 0,   Y
 *   0, 0, 1}   Z
 *
 */
static signed char gyro_orientation[9] = { 1, 0, 0,             // X axis mapping
                                           0, -1, 0,            // Y axis mapping
                                           0, 0, -1};           // Z axis mapping


// Callbacks
static void IMU_interrupt(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        // set the int state bit to wake the IMU Task
        xEventGroupSetBitsFromISR(imuTask.eventGroup,
                                  IMU_INT_BIT,
                                  &xHigherPriorityTaskWoken);
        
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

static void IMU_tap_cb(unsigned char direction, unsigned char count)
{
    debug::log("IMUTask: tap callback(" + String(direction) + ", " +String(count) + ")");
}

static void IMU_android_orient_cb(unsigned char orientation)
{
    debug::log("IMUTask: orient callback(" + String(orientation) + ")");
    imuTask.setOrientation(orientation);
}

// Public
String IMUTask::getName() const
{
    return "IMUTask";
}

void IMUTask::setup()
{
    // reset I2C bus
    // This ensures that if there was a reset, but the devices
    // on the I2C bus was not, that any transfer in progress do
    // not hang the device/bus.  Since the MPU-6050 has a 1024-byte
    // fifo, and there are 8 bits/byte, 10,000 clock edges
    // are generated to ensure the fifo is completely cleared
    // in the worst case.
    
    pinMode(PIN_WIRE_SDA, INPUT);
    pinMode(PIN_WIRE_SCL, OUTPUT);
    
    // Clock through up to 1000 bits
    int x = 0;
    for ( int i = 0; i < 10000; i++ ) {
        digitalWrite(PIN_WIRE_SCL, HIGH);
        digitalWrite(PIN_WIRE_SCL, LOW);
        digitalWrite(PIN_WIRE_SCL, HIGH);
        
        x++;
        if ( x == 8 ) {
            x = 0;
            // send a I2C stop signal
            digitalWrite(PIN_WIRE_SDA, HIGH);
            digitalWrite(PIN_WIRE_SDA, LOW);
        }
    }
    
    // send a I2C stop signal
    digitalWrite(PIN_WIRE_SDA, HIGH);
    digitalWrite(PIN_WIRE_SDA, LOW);
    Wire.begin();
    
    // MPU setup
    int count = 0;
    int result = 0;
    do {
        debug::log("IMUTask: Initialising MPU attempt " + String(count));
        mpu_force_reset();
        Tilda::delay(100);
        count++;
        result = MPUSetup();
    } while (result != 0);

    
}

void IMUTask::setOrientation(uint8_t newOrientation) {
    // TODO: wrap this in mutex?
    _orientation = newOrientation;
    
    Tilda::_appManager->orientationCallback(_orientation);
    
}

uint8_t IMUTask::getOrientation() {
    //TODO: mutex?
    return _orientation;
}



// Protected
void IMUTask::task()
{
    int8_t result;
    
    // eventGroup for notifications
    eventGroup = xEventGroupCreate();
    if (eventGroup == NULL) {
        debug::log("IMUTask: Failed to create eventGroup");
    }
    
    EventBits_t uxBits;
   
    /* Push both gyro and accel data into the FIFO. */
    // this starts the interrupts firing
    //result = mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    //debug::log("IMUTask: mpu_configure_fifo returned " + String(result));
    
    setup();
    
    // everthing setup start the dmp generating interrupts
    enable_dmp();
        
    while(true) {

        uxBits = xEventGroupWaitBits(eventGroup,
                                     IMU_INT_BIT,
                                     pdFALSE,
                                     pdFALSE,
                                     portMAX_DELAY);
        
        if( ( uxBits & IMU_INT_BIT ) != 0 ) {
            // interrupt has fired
            if (dmp_state == IMU_DMP_ON) {
                // interrupt as a result of DMP
                unsigned long sensor_timestamp;
                short gyro[3], accel[3], sensors;
                unsigned char more = 0;
                long quat[4];
                
                /* This function gets new data from the FIFO when the DMP is in
                 * use. The FIFO can contain any combination of gyro, accel,
                 * quaternion, and gesture data. The sensors parameter tells the
                 * caller which data fields were actually populated with new data.
                 * For example, if sensors == (INV_XYZ_GYRO | INV_WXYZ_QUAT), then
                 * the FIFO isn't being filled with accel data.
                 * The driver parses the gesture data to determine if a gesture
                 * event has occurred; on an event, the application will be notified
                 * via a callback (assuming that a callback function was properly
                 * registered). The more parameter is non-zero if there are
                 * leftover packets in the FIFO.
                 */
                int success = dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors,
                                            &more);
                if (!more) {
                    //hal.new_gyro = 0;
                    // TODO: more data to pull need a way to fetch it
                }
                
                // lets try printing the
             
            } else {
                // TODO: interrupt not from DMP
                
            }
            xEventGroupClearBits(eventGroup,
                                 IMU_INT_BIT);
        } else {
            // wait timed out
            // SerialUSB.println("IMUTask: Wait time out");
        }
        
    }
}

// Private
int8_t IMUTask::MPUSetup()
{
    // TODO: should check here to see if IMU is already setup and just init the lib
  
    int8_t result = 0;
    struct int_param_s int_param;
    int_param.cb = IMU_interrupt;
    int_param.pin = MPU_INT;
    int_param.arg = FALLING;
    
    unsigned char accel_fsr;
    unsigned short gyro_rate, gyro_fsr;
    unsigned long timestamp;
    
    result = mpu_init(&int_param);
    debug::log("IMUTask: mpu_init returned " + String(result));
    if (result != 0) {
        return -1;
    }
    
    /* Wake up all sensors. */
    result = mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    debug::log("IMUTask: mpu_set_sensors returned " + String(result));

    result = mpu_set_sample_rate(IMU_DEFAULT_MPU_HZ);
    debug::log("IMUTask: mpu_set_sample_rate returned " + String(result));
    
    debug::log("Pre laod");
    
    result = dmp_load_motion_driver_firmware();
    debug::log("IMUTask: dmp_load_motion_driver_firmware returned " + String(result));
    if (result != 0) {
       return -2;
    }
    result = dmp_set_orientation( inv_orientation_matrix_to_scalar(gyro_orientation) );
    debug::log("IMUTask: dmp_set_orientation returned " + String(result));
    if (result != 0) {
        return -3;
    }
    
    /*
     * Known Bug -
     * DMP when enabled will sample sensor data at 200Hz and output to FIFO at the rate
     * specified in the dmp_set_fifo_rate API. The DMP will then sent an interrupt once
     * a sample has been put into the FIFO. Therefore if the dmp_set_fifo_rate is at 25Hz
     * there will be a 25Hz interrupt from the MPU device.
     *
     * There is a known issue in which if you do not enable DMP_FEATURE_TAP
     * then the interrupts will be at 200Hz even if fifo rate
     * is set at a different rate. To avoid this issue include the DMP_FEATURE_TAP
     */
    unsigned short dmp_features = DMP_FEATURE_TAP |                // Don't remove this, see above
                                  DMP_FEATURE_ANDROID_ORIENT;      // we use this for the screen
                                                                   // Disableing the rest for now as we dont yet have a use for them
                                                                   //      DMP_FEATURE_SEND_RAW_ACCEL |
                                                                   //      DMP_FEATURE_SEND_RAW_GYRO;
    
    result = dmp_enable_feature(dmp_features);
    if (result != 0) {
        debug::log("IMUTask: mp_enable_feature returned " + String(result));
        return -4;
    }
    
    
    result = dmp_set_fifo_rate(IMU_DEFAULT_MPU_HZ);
    if (result != 0) {
        debug::log("IMUTask: dmp_set_fifo_rate returned " + String(result));
        return -5;
    }
    
    dmp_register_tap_cb(IMU_tap_cb);
    dmp_register_android_orient_cb(IMU_android_orient_cb);
    dmp_set_interrupt_mode(DMP_INT_GESTURE);
    
    return 0;
}
/* These next two functions converts the orientation matrix (see
 * gyro_orientation) to a scalar representation for use by the DMP.
 * NOTE: These functions are borrowed from Invensense's MPL.
 */
unsigned short IMUTask::inv_row_2_scale(const signed char *row) {
    
    unsigned short b;
    
    if (row[0] > 0)
    b = 0;
    else if (row[0] < 0)
    b = 4;
    else if (row[1] > 0)
    b = 1;
    else if (row[1] < 0)
    b = 5;
    else if (row[2] > 0)
    b = 2;
    else if (row[2] < 0)
    b = 6;
    else
    b = 7;      // error
    return b;
}

inline unsigned short IMUTask::inv_orientation_matrix_to_scalar(const signed char *mtx)
{
    unsigned short scalar;
    
    /*
     XYZ  010_001_000 Identity Matrix
     XZY  001_010_000
     YXZ  010_000_001
     YZX  000_010_001
     ZXY  001_000_010
     ZYX  000_001_010
     */
    
    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;
    
    return scalar;
}


int8_t IMUTask::disable_dmp()
{
    int8_t result = 0;
    result = mpu_set_dmp_state(0);
    debug::log("IMUTask: mpu_set_dmp_state(0) returned " + String(result));
    if (result != 0) {
        return -1;
    }
    dmp_state = IMU_DMP_OFF;
    return 0;
}

int8_t IMUTask::enable_dmp()
{
    int8_t result = 0;
    result = mpu_set_dmp_state(1);
    debug::log("IMUTask: mpu_set_dmp_state(1) returned " + String(result));
    if (result != 0) {
        return -1;
    }
    dmp_state = IMU_DMP_ON;
    return 0;
}

// only once instance
IMUTask imuTask;
