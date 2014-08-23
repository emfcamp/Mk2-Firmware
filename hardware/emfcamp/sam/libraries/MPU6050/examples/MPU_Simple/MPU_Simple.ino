/*
 TiLDA Mk2 MPU-6050 Motion Driver Test code
 
 Test sketch for the TiLDA Wrapper of InvenSense Embedded MotionDriver 5.1
 
 
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

#include <Wire.h>
#include <MPU6050.h>

struct int_param_s int_param;
uint8_t int_flag = false;

#define DEFAULT_MPU_HZ  (100)

unsigned char accel_fsr;
unsigned short gyro_rate, gyro_fsr;

short gyro[3];
long temp[1];

void setup() {
  // put your setup code here, to run once:
 
  
    // PMIC to CHARGE
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);
    
    pinMode(SRF_SLEEP, OUTPUT);
    digitalWrite(SRF_SLEEP, LOW);
    Wire.begin();
    
    delay(400);
    Serial.begin(115200);
    
    Serial.println("TiLDA MPU test");
    
    int_param.cb = gyro_data_ready_cb;
    int_param.pin = MPU_INT;
    int_param.arg = FALLING;
    
    Serial.print("MPU init : "); 
    Serial.println(mpu_init(&int_param));
    
    Serial.print("Setup Sensors: ");
    Serial.println(mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL));
    
    Serial.print("Setup fifo");
    Serial.println(mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL));
    
    Serial.print("Set sample rate: ");    
    Serial.println(mpu_set_sample_rate(DEFAULT_MPU_HZ));
    
    Serial.print("Get sample rate: ");
    Serial.println(mpu_get_sample_rate(&gyro_rate));
    Serial.println(gyro_rate);
    Serial.print("Get gryo fsr: ");
    Serial.println(mpu_get_gyro_fsr(&gyro_fsr));
    Serial.println(gyro_fsr);
    Serial.print("Get accell_fsr: ");
    Serial.println(mpu_get_accel_fsr(&accel_fsr));
    Serial.println(accel_fsr);
 
    Serial.print("Get Gyro reg: ");
    Serial.println(mpu_get_gyro_reg(gyro, NULL));
    Serial.println(gyro[0]);
    Serial.println(gyro[1]);
    Serial.println(gyro[2]);
    
    Serial.print("Get temp: ");
    Serial.println(mpu_get_temperature(temp, NULL));
    Serial.println(temp[0]);
    
}


void loop() {
  // put your main code here, to run repeatedly:
  if (int_flag) {
     Serial.print("Int flag set");
     int_flag = false;
  }
  

}

void gyro_data_ready_cb()
{
    int_flag = true; 
}
