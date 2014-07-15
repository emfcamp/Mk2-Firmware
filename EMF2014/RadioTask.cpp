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
#include "sha1.h"

#define RADIO_ENTER_AT_MODE() vTaskDelay(10); digitalWrite(RADIO_AT_MODE_PIN, LOW); vTaskDelay(10);
#define RADIO_LEAVE_AT_MODE() vTaskDelay(10); digitalWrite(RADIO_AT_MODE_PIN, HIGH); vTaskDelay(10);

String RadioTask::getName() {
	return "RadioTask";
}

void RadioTask::task() {
	// Setup radio communitcation
	RADIO_SERIAL.begin(115200);

	// Setup AT Mode pin
	pinMode(RADIO_AT_MODE_PIN, OUTPUT);

	//Sha1.init();
	//Sha1.print("abc");
	//debug::logHash("Hash:", Sha1.result());

	//pinMode(RADIO_AT_MODE_PIN, OUTPUT);

	// Set general setting for radio
	RADIO_ENTER_AT_MODE();
	RADIO_SERIAL.println("ATZD3");  // output format <payload>|<rssi>
	RADIO_SERIAL.println("ATPK3A"); // 58byte
	RADIO_SERIAL.println("ATAC");   // apply
	RADIO_LEAVE_AT_MODE();

	// Clear buffer
	for (uint8_t i=0; i<32; i++) RADIO_SERIAL.read();

	// Buffers
	byte packetBuffer[RADIO_PACKET_WITH_RSSI_LENGTH + 1];
	uint8_t packetBufferLength = 0;

	while (true) {
		uint8_t availableBytes = RADIO_SERIAL.available();
		if (availableBytes > 0) {
			//debug::log("Bytes to read: " + String(availableBytes));
			while (availableBytes > 0) {
				packetBuffer[packetBufferLength] = RADIO_SERIAL.read();
				packetBufferLength++;
				availableBytes--;

				// Only handle one packet at a time
				if (packetBufferLength == RADIO_PACKET_WITH_RSSI_LENGTH) {
					debug::log("Break - we have enough for one packet");
					break;
				}
			}

			//debug::log("Out: " + String((char*)packetBuffer));

			// Have we received a whole packet yet?
			bool receivedWholePacket = 
				packetBufferLength >= 5 + 1 && // Has to have at least one byte payload
				packetBuffer[packetBufferLength - 1] >= 48 && packetBuffer[packetBufferLength - 1] <= 57 && // Digit
				packetBuffer[packetBufferLength - 2] >= 48 && packetBuffer[packetBufferLength - 2] <= 57 && // Digit
				packetBuffer[packetBufferLength - 3] >= 48 && packetBuffer[packetBufferLength - 3] <= 57 && // Digit
				packetBuffer[packetBufferLength - 4] == '-' &&
				packetBuffer[packetBufferLength - 5] == '|';

			// If the data looks like a packet we can start parsing it
			if (receivedWholePacket) {
				uint8_t rssi = (packetBuffer[packetBufferLength - 1] - 48) +
					(packetBuffer[packetBufferLength - 2] - 48) * 10 +
					(packetBuffer[packetBufferLength - 3] - 48) * 100;
				packetBufferLength -= 5;

				String test = "";
				for (uint8_t i=0; i<packetBufferLength; i++) {
					test += (char)packetBuffer[i];
				}
				debug::log("Packet: " + test + " RSSI: " + rssi);

				packetBufferLength = 0;

			} else if (packetBufferLength == RADIO_PACKET_WITH_RSSI_LENGTH) {
				debug::log("Packet does not conform");
				// Something's wrong, we received enough bytes but it's not formated correctly. 
				packetBufferLength = 0;
			}

		} 
	}
}