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
#include "IncomingRadioMessage.h"

#include <FreeRTOS_ARM.h>

#define NO_CURRENT_MESSAGE 65535
#define NO_CHANNEL_DISCOVERED 255

RadioTask::RadioTask(MessageCheckTask& aMessageCheckTask, RTC_clock& aRealTimeClock)
	:mMessageCheckTask(aMessageCheckTask), mRealTimeClock(aRealTimeClock)
{
}

String RadioTask::getName() const {
	return "RadioTask";
}

void RadioTask::task() {

	// Temporary: Remove me soon!
	_outgoingPacketBuffer[0] = 0x90;
	_outgoingPacketBuffer[1] = 0x03; // ping service
	for (uint8_t i=2; i<RADIO_PACKET_LENGTH; i++) {
		_outgoingPacketBuffer[i] = i - 2;
	}
	_outgoingPacketAvailable = true;

	// Setup radio communitcation
	RADIO_SERIAL.begin(RADIO_SERIAL_BAUD);

	// Setup AT Mode pin
	pinMode(RADIO_AT_MODE_PIN, OUTPUT);

	_clearSerialBuffer();
	_initialiseDiscoveryState();

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

			packetBufferLength = _parsePacketBuffer(packetBuffer, packetBufferLength);

			_lastMessageReceived = xTaskGetTickCount();
		}

		_checkForStateChange();
	}
}

inline void RadioTask::_enterAtMode() {
	digitalWrite(RADIO_AT_MODE_PIN, LOW);
}

inline void RadioTask::_leaveAtMode() {
	vTaskDelay(10);
	digitalWrite(RADIO_AT_MODE_PIN, HIGH);
}

inline uint8_t RadioTask::_parsePacketBuffer(byte packetBuffer[], uint8_t packetBufferLength) {
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

		if (_radioState == RADIO_STATE_DISCOVERY) {
			_handleDiscoveryPacket(packetBuffer, packetBufferLength, rssi);
		} else if (_radioState == RADIO_STATE_RECEIVE) {
			_handleReceivePacket(packetBuffer, packetBufferLength);
		}

		packetBufferLength = 0;
	} else if (packetBufferLength == RADIO_PACKET_WITH_RSSI_LENGTH) {
		debug::log("RadioTask: Packet does not conform");
		//debug::logByteArray(packetBuffer, 58);
		// Something's wrong, we received enough bytes but it's not formated correctly.
		packetBufferLength = 0;
	}

	return packetBufferLength;
}

inline void RadioTask::_handleDiscoveryPacket(byte packetBuffer[], uint8_t packetBufferLength, uint8_t rssi) {
	uint8_t channel = packetBuffer[0];
	uint32_t timestamp = _bytesToInt(packetBuffer[1], packetBuffer[2], packetBuffer[3], packetBuffer[4]);
	if (!mRealTimeClock.has_been_set()) {
		debug::log("Setting time to " + String(timestamp));
		mRealTimeClock.set_unixtime(timestamp);
	}

	char identifier[3];
	identifier[0] = packetBuffer[5];
	identifier[1] = packetBuffer[6];
	identifier[2] = packetBuffer[7];
	if (rssi < _bestRssi) {
		_bestRssi = rssi;
		_bestChannel = channel;
	}
}

inline void RadioTask::_handleReceivePacket(byte packetBuffer[], uint8_t packetBufferLength) {
	// the first two bytes are always describing the receiver
	int packetReceiver = _bytesToInt(packetBuffer[0], packetBuffer[1]);

	/*if (_currentMessageReceiver != packetReceiver) {
		debug::log("RadioTask: Still waiting for packets, but got new receiver. Was waiting for " + String(_remainingMessageLength) + " bytes");
	}*/

	// parsing the packet - is it payload or header?
	bool couldBeMessageHeader =
		_currentMessageReceiver == NO_CURRENT_MESSAGE || // First message or first packet after successfully finished message
		_currentMessageReceiver != packetReceiver;  // or something has gone wrong (e.g. packet lost)

	if (couldBeMessageHeader) {

		// Set meta data variables
		_messageBufferPosition = 0;
		_currentMessageReceiver = packetReceiver;
		_remainingMessageLength = _bytesToInt(packetBuffer[2], packetBuffer[3], packetBuffer[4], packetBuffer[5]);
		memcpy(_currentMessageHash, packetBuffer + 6, 12);
		memcpy(_currentMessageSignature, packetBuffer + 18, 40);
	} else {
		if (_messageBufferPosition + packetBufferLength > RADIO_MAX_MESSAGE_BUFFER_LENGTH) {
			// buffer overflow protection. a message should never be this long.
			_messageBufferPosition = 0;
		}

		// assemble message
		int bytesToCopy = min(_remainingMessageLength, packetBufferLength - 2);
		memcpy(_messageBuffer + _messageBufferPosition, packetBuffer + 2, bytesToCopy);
		_messageBufferPosition += bytesToCopy;
		_remainingMessageLength -= bytesToCopy;
	}

	if (_remainingMessageLength == 0 && _currentMessageReceiver != NO_CURRENT_MESSAGE) {
		_verifyMessage();

		// Temporary: Just send a message back.
		//_sendOutgoingBuffer();

		// Reset for next message
		_messageBufferPosition = 0;
		_currentMessageReceiver = NO_CURRENT_MESSAGE;
	}
}

inline void RadioTask::_verifyMessage() {
	// We're not actually verifying messages in this task, they're passed
	// on to the MessageCheckTask
	uint32_t messageLength = _messageBufferPosition;

	// Create a message object.
	IncomingRadioMessage *message = new IncomingRadioMessage(messageLength,
																_messageBuffer,
																_currentMessageHash,
																_currentMessageSignature,
																_currentMessageReceiver);

	mMessageCheckTask.addIncomingMessage(message);
}

inline void RadioTask::_checkForStateChange() {
	if (_radioState == RADIO_STATE_DISCOVERY) {
		if (_discoveryFinishingTime <= xTaskGetTickCount()) {
			if (_bestChannel == NO_CHANNEL_DISCOVERED) {
				debug::log("RadioTask: No channel discovered during this discovery period. Will sleep for certain time and try again");
				vTaskDelay(RADIO_UNSUCCESSFUL_DISCOVERY_SLEEP);
				_initialiseDiscoveryState();
			} else {
				_initialiseReceiveState();
			}
		}
	} else if (_radioState == RADIO_STATE_RECEIVE) {
		if (_lastMessageReceived + RADIO_RECEIVE_TIMEOUT <= xTaskGetTickCount()) {
			debug::log("RadioTask: Main channel timeout - Going back to disovery");
			_initialiseDiscoveryState();
		}
	}
}

inline void RadioTask::_initialiseDiscoveryState() {
	debug::log("RadioTask: Starting discovery state");
	_bestRssi = 255;
	_bestChannel = NO_CHANNEL_DISCOVERED;
	_radioState = RADIO_STATE_DISCOVERY;
	_discoveryFinishingTime = xTaskGetTickCount() + RADIO_DISCOVERY_TIME;

	_enterAtMode();
	RADIO_SERIAL.println("ATZD3");  // output format <payload>|<rssi>
	RADIO_SERIAL.println("ATPK08"); // 8byte packet length
	RADIO_SERIAL.println(String("ATCN") + String(RADIO_DISCOVERY_CHANNEL)); // Discovery Channel
	RADIO_SERIAL.println("ATAC");   // apply
	RADIO_SERIAL.flush();
	_leaveAtMode();

	_clearSerialBuffer();
}

inline void RadioTask::_initialiseReceiveState() {
	debug::log("RadioTask: Starting to receive on channel " + String(_bestChannel));

	_enterAtMode();
	RADIO_SERIAL.println("ATZD3");  // output format <payload>|<rssi>
	RADIO_SERIAL.println("ATPK3A"); // 58byte packet length
	RADIO_SERIAL.println("ATCN" + _intToHex(_bestChannel)); // Channel
	RADIO_SERIAL.println("ATAC");   // apply
	RADIO_SERIAL.flush();
	_leaveAtMode();

	_radioState = RADIO_STATE_RECEIVE;

	_clearSerialBuffer();
}

inline void RadioTask::_clearSerialBuffer() {
	while (RADIO_SERIAL.available()) RADIO_SERIAL.read();
}

inline void RadioTask::_sendOutgoingBuffer() {
	RADIO_SERIAL.write(_outgoingPacketBuffer, RADIO_PACKET_LENGTH);
	RADIO_SERIAL.flush();
	debug::log("RadioTask: Outgoing message sent");
	//_outgoingPacketAvailable = false;
}

// ToDo: These could probably live somewhere else
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
inline String RadioTask::_intToHex(uint8_t input) {
	return String("0123456789abcdef"[input>>4]) + String("0123456789abcdef"[input&0xf]);
}

