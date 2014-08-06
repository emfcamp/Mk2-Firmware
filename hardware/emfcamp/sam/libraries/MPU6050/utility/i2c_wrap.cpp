/*
 TiLDA Mk2 MPU-6050 driver
 
 Wrapper file for i2c interface
 
 
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

#include "i2c_wrap.h"

#include <Wire.h>

// Borrowed from I2Cdev lib http://www.i2cdevlib.com

extern "C" {

/** Read multiple bytes from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register regAddr to read from
 * @param length Number of bytes to read
 * @param data Buffer to store read data in
 * @return Number of bytes read (-1 indicates failure)
 */
int8_t i2c_read(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data) {
    int8_t count = 0;
    
    // Arduino v1.0.1+, Wire library
    // Adds official support for repeated start condition, yay!
    
    // I2C/TWI subsystem uses internal buffer that breaks with large data requests
    // so if user requests more than BUFFER_LENGTH bytes, we have to do it in
    // smaller chunks instead of all at once
    uint8_t k = 0;
    for (k; k < length; k += min(length, BUFFER_LENGTH)) {
        Wire.beginTransmission(devAddr);
        Wire.write(regAddr);
        Wire.endTransmission();
        Wire.beginTransmission(devAddr);
        Wire.requestFrom(devAddr, (uint8_t)min(length - k, BUFFER_LENGTH));
        
        for (; Wire.available(); count++) {
            data[count] = Wire.read();
        }
    }
    if (count != length)
        return -1;
    return 0;
}

/** Write multiple bytes to an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register address to write to
 * @param length Number of bytes to write
 * @param data Buffer to copy new data from
 * @return Status of operation (0:success, 1:data too long to fit in transmit buffer, 2:received NACK on transmit of address, 3:received NACK on transmit of data, 4:other error)
)
 */
bool i2c_write(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t* data) {
    uint8_t status = 0;
    
    Wire.beginTransmission(devAddr);
    Wire.write((uint8_t) regAddr); // send address
    uint8_t i = 0;
    for (i; i < length; i++) {
        Wire.write((uint8_t) data[i]);
    }
    
    Wire.endTransmission();
    status = Wire.endTransmission();
    
    return status;
}
    
}