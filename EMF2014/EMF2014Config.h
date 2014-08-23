/*
 TiLDA Mk2

 EMF2014Config
 This files contains all the initial configuration details for the badge firmware and any compli time defines that might be used by any of the task


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

// This defines how many subscription each button can handle
#define MAX_BUTTON_SUBSCRIPTIONS 10
// maximum amount of apps the AppManager can handle
#define MAX_APPS 10
// pin used for seeding the random number generator
#define RANDOM_SEED_PIN 5

// duration that center has to be pressed to go back to the homescreen
#define CENTER_BUTTON_PRESS_DURATION_FOR_HOME_SCREEN 500

// Radio serial port
#define RADIO_SERIAL Serial
#define RADIO_SERIAL_BAUD 115200
// Packet length
#define RADIO_PACKET_LENGTH 58
#define RADIO_PACKET_WITH_RSSI_LENGTH 58 + 1 + 4
// Radio message buffer length
#define RADIO_MAX_MESSAGE_BUFFER_LENGTH 40960
// Radio discovery channel (in hex)
#define RADIO_DISCOVERY_CHANNEL "02"
// Discovery time in ticks
#define RADIO_DISCOVERY_TIME 1000
// Sleep between unsuccessful discoverys
#define RADIO_UNSUCCESSFUL_DISCOVERY_SLEEP 10000
// Time spend without incoming messages before badge goes back into
// discovery mode (in ticks)
#define RADIO_RECEIVE_TIMEOUT 5000
// Sleep time for the receiver task when no serial data is available
#define RADIO_NO_DATA_SLEEP_DURATION 5

//#define RADIO_DEBUG_MODE
//#define RADIO_DEBUG_MODE_EXTENDED


// EMF2014 Public Key
const uint8_t EMF_PUBLIC_KEY[40] = {0x8a, 0x5a, 0x14, 0xcc, 0xf8, 0x45, 0x21, 0x59, 0x4c, 0xe1,
	                                0xf8, 0x82, 0x61, 0xfd, 0xa1, 0x87, 0xb5, 0x41, 0x6d, 0xb3,
                                    0xf6, 0xd2, 0x4b, 0xd7, 0x50, 0xc1, 0x76, 0x5c, 0xc2, 0x58,
                                    0x8f, 0x1d, 0x82, 0x68, 0xec, 0x37, 0x1f, 0xcd, 0xe7, 0x24};


enum Button {
    NONE         = 0,
    LIGHT        = 1,
    A            = 2,
    B            = 4,
    UP           = 8,
    DOWN         = 16,
    LEFT         = 32,
    RIGHT        = 64,
    CENTER       = 128
};
