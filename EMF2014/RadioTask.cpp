/*
 TiLDA Mk2
 
 RadioTask

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

#include "RadioTask.h"
#include "DebugTask.h"
#include <FreeRTOS_ARM.h>

String RadioTask::getName() {
	return "RadioTask";
}

void RadioTask::task() {
	// Setup radio communitcation
	pinMode(8, OUTPUT);
	digitalWrite(8, HIGH);
	RADIO_SERIAL.begin(115200);

	// Initial setup
	Serial.println("+++");
	delay(1200);
	Serial.println("ATID");
	Serial.println("ATDN");

	// Buffers
	byte packetBuffer[RADIO_PACKET_WITH_RSSI_LENGTH];
	uint8_t packetBufferPosition = 0;

	while (true) {
		uint8_t availableBytes = RADIO_SERIAL.available();
		debug::log("Radio loop" + String(availableBytes));
		if (availableBytes > 0) {
			debug::log("Available Bytes: " + String(availableBytes));
			while (availableBytes > 0) {
				packetBuffer[packetBufferPosition] = RADIO_SERIAL.read();
				packetBufferPosition++;
				availableBytes--;

				// Only handle one packet at a time
				if (packetBufferPosition == RADIO_PACKET_WITH_RSSI_LENGTH) {
					break;
				}
			}

			debug::log("Received " + String((char*)packetBuffer));

			// Have we received a whole packet yet?
			if (packetBufferPosition == RADIO_PACKET_WITH_RSSI_LENGTH) {
				packetBufferPosition = 0;


			}

		} else {
			// Avoid busy waiting
			vTaskDelay(1200);
		}
	}
}