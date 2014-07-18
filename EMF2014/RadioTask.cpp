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
#include <Sha1.h>

String RadioTask::getName() {
	return "RadioTask";
}

void RadioTask::task() {
	// Setup radio communitcation
	RADIO_SERIAL.begin(RADIO_SERIAL_BAUD);

	// Setup AT Mode pin
	pinMode(RADIO_AT_MODE_PIN, OUTPUT);

	// Set general setting for radio
	_enterAtMode();
	RADIO_SERIAL.println("ATZD3");  // output format <payload>|<rssi>
	RADIO_SERIAL.println("ATPK3A"); // 58byte
	RADIO_SERIAL.println("ATAC");   // apply
	RADIO_SERIAL.flush(); 
	_leaveAtMode();

	// Clear serial buffer
	for (uint8_t i=0; i<32; i++) RADIO_SERIAL.read();

	// Packet
	byte packetBuffer[RADIO_PACKET_WITH_RSSI_LENGTH];
	uint8_t packetBufferLength = 0;

	_currentMessageReceiver = NO_CURRENT_MESSAGE;
	while (true) {
		uint8_t availableBytes = RADIO_SERIAL.available();
		if (availableBytes > 0) {
			while (availableBytes > 0) {
				packetBuffer[packetBufferLength] = RADIO_SERIAL.read();
				packetBufferLength++;
				availableBytes--;

				// Only handle one packet at a time
				if (packetBufferLength == RADIO_PACKET_WITH_RSSI_LENGTH) {
					break; // we have enough for one packet
				}
			}

			_parsePacketBuffer(packetBuffer, packetBufferLength);
		} 
	}
}

inline void RadioTask::_enterAtMode() {
	digitalWrite(RADIO_AT_MODE_PIN, LOW); 
}

inline void RadioTask::_leaveAtMode() {
	vTaskDelay(10);
	digitalWrite(RADIO_AT_MODE_PIN, HIGH); 
} 

inline void RadioTask::_parsePacketBuffer(byte packetBuffer[], uint8_t & packetBufferLength) {
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
		// read rssi and then ignore the packet footer
		uint8_t rssi = (packetBuffer[packetBufferLength - 1] - 48) +
			(packetBuffer[packetBufferLength - 2] - 48) * 10 +
			(packetBuffer[packetBufferLength - 3] - 48) * 100;
		packetBufferLength -= 5;

		//debug::log("Received packet with RSSI " + String(rssi));
		//debug::logByteArray(packetBuffer, 58);
		if (packetBufferLength!=58) {
			debug::log("Found packet that's too short: " + String(packetBufferLength));
		}

		_handlePacket(packetBuffer, packetBufferLength);

		packetBufferLength = 0;
	} else if (packetBufferLength == RADIO_PACKET_WITH_RSSI_LENGTH) {
		debug::log("Packet does not conform");
		//debug::logByteArray(packetBuffer, 58);
		// Something's wrong, we received enough bytes but it's not formated correctly. 
		packetBufferLength = 0;
	}
}

inline void RadioTask::_handlePacket(byte packetBuffer[], uint8_t packetBufferLength) {
	// the first two bytes are always describing the receiver
	int packetReceiver = _bytesToInt(packetBuffer[0], packetBuffer[1]);

	if (_currentMessageReceiver != packetReceiver) {
		debug::log("Still waiting for packets, but got new receiver. Was waiting for " + String(_remainingMessageLength) + " bytes");
	}

	// parsing the packet - is it payload or header?
	bool couldBeMessageHeader = 
		_currentMessageReceiver == NO_CURRENT_MESSAGE || // First message or first packet after successfully finished message
		_currentMessageReceiver != packetReceiver;  // or something has gone wrong (e.g. packet lost)

	if (couldBeMessageHeader) {

		// Set meta data variables
		_messageBufferPosition = 0;
		_currentMessageReceiver = packetReceiver;
		_remainingMessageLength = _bytesToInt(packetBuffer[2], packetBuffer[3], packetBuffer[4], packetBuffer[5]);
		memcpy(_currentMessageHash, packetBuffer + 6, sizeof(_currentMessageHash));
		memcpy(_currentMessageSignature, packetBuffer + 26, sizeof(_currentMessageSignature));
		debug::log("Received message header to " + String(_currentMessageReceiver) + " with length " + String(_remainingMessageLength));
	} else {
		if (_messageBufferPosition + packetBufferLength > RADIO_MAX_MESSAGE_BUFFER_LENGTH) {
			// buffer overflow protection. a message should never be this long. 
			_messageBufferPosition = 0;
		}

		// assemble message
		int bytesToCopy = min(_remainingMessageLength, packetBufferLength - 2);
		if (bytesToCopy!=56) {
			debug::log("Less bytes to copy " + String(bytesToCopy));
		}
		memcpy(_messageBuffer + _messageBufferPosition, packetBuffer + 2, bytesToCopy);
		_messageBufferPosition += bytesToCopy;
		_remainingMessageLength -= bytesToCopy;
	}

	if (_remainingMessageLength == 0 && _currentMessageReceiver != NO_CURRENT_MESSAGE) {
		_verifyMessage();

		// Reset for next message
		_messageBufferPosition = 0;
		_currentMessageReceiver = NO_CURRENT_MESSAGE;
	}	
}

inline void RadioTask::_verifyMessage() {
	uint32_t messageLength = _messageBufferPosition;

	// Create SHA1 digest
	Sha1.init();
	char receiverHi = _currentMessageReceiver >> 8;
	char receiverLo = _currentMessageReceiver & 0xFF;
 	Sha1.print(receiverHi);
	Sha1.print(receiverLo);
	for (uint32_t i=0; i<messageLength; i++) {
		Sha1.print((char)_messageBuffer[i]);
	}
	byte* digest = Sha1.result();	
	
	// Check our digest against the one send in the header
	if (memcmp(digest, _currentMessageHash, 20) != 0) {
		debug::log("Can't validate message, checksum doesn't match.");
		debug::logByteArray(digest, 20);
		debug::logByteArray(_currentMessageHash, 20);
		return;
	}

	debug::log("Received message (checksum ok): " + String((char*)_messageBuffer));
}

inline uint16_t RadioTask::_bytesToInt(byte b1, byte b2) {
	int result = 0;
	result = (result << 8) + b1;
	result = (result << 8) + b2;
	return result;
}
inline uint32_t RadioTask::_bytesToInt(byte b1, byte b2, byte b3, byte b4) {
	int result = 0;
	result = (result << 8) + b1;
	result = (result << 8) + b2;
	result = (result << 8) + b3;
	result = (result << 8) + b4;
	return result;
}