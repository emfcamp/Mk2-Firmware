/*
 TiLDA Mk2

 RadioReceiveTask

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

#include "RadioReceiveTask.h"
#include "IncomingRadioMessage.h"
#include "Utils.h"

#include <FreeRTOS_ARM.h>
#include <debug.h>

#include "IncomingRadioMessage.h"
#include "MessageCheckTask.h"

#define NO_CURRENT_MESSAGE 65535
#define NO_CHANNEL_DISCOVERED 255

RadioReceiveTask::RadioReceiveTask(MessageCheckTask& aMessageCheckTask, RTC_clock& aRealTimeClock)
	:mMessageCheckTask(aMessageCheckTask), mRealTimeClock(aRealTimeClock)
{
	_bestChannelIdentifier[0] = '?';
	_bestChannelIdentifier[1] = '?';
	_bestChannelIdentifier[2] = '?';
	_bestChannelIdentifier[3] = 0;
}

String RadioReceiveTask::getName() const {
	return "RadioReceiveTask";
}

void RadioReceiveTask::beforeResume() {
	// Don't count suspended time towards timeout!
	_lastMessageReceived = xTaskGetTickCount();

	// Get rid of garbage collected during transmit period (like talk of other radios)
	_clearSerialBuffer();
}

void RadioReceiveTask::task() {
	_clearSerialBuffer();
	_initialiseDiscoveryState();

	// Packet
	byte packetBuffer[RADIO_PACKET_WITH_RSSI_LENGTH];
	uint8_t packetBufferLength = 0;

	#ifdef RADIO_DEBUG_MODE
		uint8_t serialRingBufferHWM = 0;
	#endif

	_wakeUp();

	_currentMessageReceiver = NO_CURRENT_MESSAGE;
	while (true) {
		uint8_t availableBytes = RADIO_SERIAL.available();
		if (availableBytes > 0) {
			pinMode(PIN_LED_RXL, OUTPUT);
			digitalWrite(PIN_LED_RXL, 1);

			#ifdef RADIO_DEBUG_MODE
				if (serialRingBufferHWM < availableBytes) {
					serialRingBufferHWM = availableBytes;
					debug::log("RadioReceiveTask: serialRingBufferHWM=" + String(serialRingBufferHWM));
				}
			#endif

			while (availableBytes > 0) {
				packetBuffer[packetBufferLength] = RADIO_SERIAL.read();
				packetBufferLength++;
				availableBytes--;

				// Only handle one packet at a time
				if (packetBufferLength == RADIO_PACKET_WITH_RSSI_LENGTH) {
					break; // we have enough for one packet
				}
			}

			#ifdef RADIO_DEBUG_MODE_EXTENDED
				debug::log("RadioReceiveTask: Current buffer:");
				debug::logByteArray(packetBuffer, packetBufferLength);
			#endif

			packetBufferLength = _parsePacketBuffer(packetBuffer, packetBufferLength);

			digitalWrite(PIN_LED_RXL, 0);

			_lastMessageReceived = xTaskGetTickCount();
		} else {
			vTaskDelay(RADIO_NO_DATA_SLEEP_DURATION);
		}

		_checkForStateChange();
	}
}

inline void RadioReceiveTask::_enterAtMode() {
	digitalWrite(SRF_AT_COMMAND, LOW);
}

inline void RadioReceiveTask::_leaveAtMode() {
	vTaskDelay(10);
	digitalWrite(SRF_AT_COMMAND, HIGH);
}

inline void RadioReceiveTask::_sleep() {
	digitalWrite(SRF_SLEEP, HIGH);
}

inline void RadioReceiveTask::_wakeUp() {
	digitalWrite(SRF_SLEEP, LOW);
}

inline uint8_t RadioReceiveTask::_parsePacketBuffer(byte packetBuffer[], uint8_t packetBufferLength) {
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
		_rssi = (packetBuffer[packetBufferLength - 1] - 48) +
			(packetBuffer[packetBufferLength - 2] - 48) * 10 +
			(packetBuffer[packetBufferLength - 3] - 48) * 100;
		packetBufferLength -= 5;

		#ifdef RADIO_DEBUG_MODE
			debug::log("Packet!");
		#endif

		if (_radioState == RADIO_STATE_DISCOVERY) {
			_handleDiscoveryPacket(packetBuffer, packetBufferLength, _rssi);
		} else if (_radioState == RADIO_STATE_RECEIVE) {
			_handleReceivePacket(packetBuffer, packetBufferLength);
		}

		packetBufferLength = 0;
	} else if (packetBufferLength == RADIO_PACKET_WITH_RSSI_LENGTH) {
		#ifdef RADIO_DEBUG_MODE
			debug::log("RadioReceiveTask: Packet does not conform");
			debug::logByteArray(packetBuffer, 58);
		#endif
		// Something's wrong, we received enough bytes but it's not formated correctly.
		// Let's sleep a while and clear the buffer. Hopefully that'll fix it.
		vTaskDelay(7);
		_clearSerialBuffer();
		packetBufferLength = 0;
	}

	return packetBufferLength;
}

inline void RadioReceiveTask::_handleDiscoveryPacket(byte packetBuffer[], uint8_t packetBufferLength, uint8_t rssi) {
	uint8_t channel = packetBuffer[0];
	uint32_t timestamp = Utils::bytesToInt(packetBuffer[1], packetBuffer[2], packetBuffer[3], packetBuffer[4]);
	if (timestamp < RADIO_MINIMUM_CURRENT_TIME || timestamp > RADIO_MAXIMUM_CURRENT_TIME) {
		#ifdef RADIO_DEBUG_MODE
			debug::log("RadioReceiveTask: timestamp sanity check failed");
		#endif
		return;
	}
	if (!mRealTimeClock.has_been_set()) {
		debug::log("Setting time to " + String(timestamp));
		mRealTimeClock.set_unixtime(timestamp);
	}

	_bestChannelIdentifier[0] = packetBuffer[5];
	_bestChannelIdentifier[1] = packetBuffer[6];
	_bestChannelIdentifier[2] = packetBuffer[7];
	_bestChannelIdentifier[3] = 0;

	if (rssi < _bestRssi) {
		_bestRssi = rssi;
		_bestChannel = channel;
	}

	if (_bestRssi == rssi) {
		_bestChannelRemainingTransmitWindow = Utils::bytesToInt(packetBuffer[8], packetBuffer[9], packetBuffer[10], packetBuffer[11]);

		// Sanity check. We don't want to have rouge packets send the task to sleep forever
		if (_bestChannelRemainingTransmitWindow > RADIO_MAX_TRANSMIT_WINDOW_LENGTH) {
			_bestChannelRemainingTransmitWindow = 0;
			#ifdef RADIO_DEBUG_MODE
				debug::log("RadioReceiveTask: RADIO_MAX_TRANSMIT_WINDOW_LENGTH sanity check failed");
			#endif
		}
	}
}

inline void RadioReceiveTask::_handleReceivePacket(byte packetBuffer[], uint8_t packetBufferLength) {
	// the first two bytes are always describing the receiver
	int packetReceiver = Utils::bytesToInt(packetBuffer[0], packetBuffer[1]);

	/*if (_currentMessageReceiver != packetReceiver) {
		debug::log("RadioReceiveTask: Still waiting for packets, but got new receiver. Was waiting for " + String(_remainingMessageLength) + " bytes");
	}*/

	// parsing the packet - is it payload or header?
	bool couldBeMessageHeader =
		_currentMessageReceiver == NO_CURRENT_MESSAGE || // First message or first packet after successfully finished message
		_currentMessageReceiver != packetReceiver;  // or something has gone wrong (e.g. packet lost)

	if (couldBeMessageHeader) {

		// Set meta data variables
		_messageBufferPosition = 0;
		_currentMessageReceiver = packetReceiver;
		_remainingMessageLength = Utils::bytesToInt(packetBuffer[2], packetBuffer[3], packetBuffer[4], packetBuffer[5]);
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

		// Reset for next message
		_messageBufferPosition = 0;
		_currentMessageReceiver = NO_CURRENT_MESSAGE;
	}
}

inline void RadioReceiveTask::_verifyMessage() {
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

inline void RadioReceiveTask::_checkForStateChange() {
	if (_radioState == RADIO_STATE_DISCOVERY) {
		if (_discoveryFinishingTime <= xTaskGetTickCount()) {
			if (_bestChannel == NO_CHANNEL_DISCOVERED) {
				debug::log("RadioReceiveTask: No channel discovered during this discovery period. Will sleep for certain time and try again");
				_sleep();
				vTaskDelay(RADIO_UNSUCCESSFUL_DISCOVERY_SLEEP);
				_wakeUp();
				vTaskDelay(RADIO_WAKEUP_TIME);
				_initialiseDiscoveryState();
			} else {
				_initialiseReceiveState();
			}
		}
	} else if (_radioState == RADIO_STATE_RECEIVE) {
		if (_lastMessageReceived + RADIO_RECEIVE_TIMEOUT <= xTaskGetTickCount()) {
			debug::log("RadioReceiveTask: Main channel timeout - Going back to disovery");
			_initialiseDiscoveryState();
		}
	}
}

inline void RadioReceiveTask::_initialiseDiscoveryState() {
	debug::log("RadioReceiveTask: Starting discovery state");
	_bestRssi = 255;
	_bestChannel = NO_CHANNEL_DISCOVERED;
	_radioState = RADIO_STATE_DISCOVERY;
	_bestChannelRemainingTransmitWindow = 0;
	_discoveryFinishingTime = xTaskGetTickCount() + RADIO_DISCOVERY_TIME;
	_bestChannelIdentifier[0] = '?';
	_bestChannelIdentifier[1] = '?';
	_bestChannelIdentifier[2] = '?';
	_bestChannelIdentifier[3] = 0;

	_enterAtMode();
	RADIO_SERIAL.println("ATZD3");  // output format <payload>|<rssi>
	RADIO_SERIAL.println("ATPK0C"); // 12byte packet length
	RADIO_SERIAL.println(String("ATCN") + String(RADIO_DISCOVERY_CHANNEL)); // Discovery Channel
	RADIO_SERIAL.println("ATAC");   // apply
	RADIO_SERIAL.flush();
	_leaveAtMode();

	_clearSerialBuffer();
}

inline void RadioReceiveTask::_initialiseReceiveState() {
	debug::log("RadioReceiveTask: Starting to receive on channel " + String(_bestChannel) + " with RSSI " + String(_bestRssi));

	_enterAtMode();
	RADIO_SERIAL.println("ATZD3");  // output format <payload>|<rssi>
	RADIO_SERIAL.println("ATPK3A"); // 58byte packet length
	RADIO_SERIAL.println("ATCN" + Utils::intToHex(_bestChannel)); // Channel
	RADIO_SERIAL.println("ATAC");   // apply
	RADIO_SERIAL.flush();
	_leaveAtMode();

	// This channel might be in transmit phase. Sleep it out.
	if (_bestChannelRemainingTransmitWindow > 0) {
		vTaskDelay(_bestChannelRemainingTransmitWindow);
		_lastMessageReceived = xTaskGetTickCount();
	}

	_radioState = RADIO_STATE_RECEIVE;

	_clearSerialBuffer();
}

inline void RadioReceiveTask::_clearSerialBuffer() {
	while (RADIO_SERIAL.available()) RADIO_SERIAL.read();
}

char* RadioReceiveTask::channelIdentifier() {
	return _bestChannelIdentifier;
}

uint8_t RadioReceiveTask::rssi() {
	return _rssi;
}
