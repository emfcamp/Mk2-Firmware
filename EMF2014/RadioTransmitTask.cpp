/*
 TiLDA Mk2

 RadioTransmitTask

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

#include <FreeRTOS_ARM.h>
#include <debug.h>

#include "RadioTransmitTask.h"
#include "Utils.h"
#include "Tilda.h"
#include "SettingsStore.h"
#include "MessageCheckTask.h"
#include "IncomingRadioMessage.h"

RadioTransmitTask::RadioTransmitTask(RadioReceiveTask& aRadioRecieveTask,
										const SettingsStore& aSettingsStore,
										MessageCheckTask& aMessageCheckTask)
	:mRadioReceiveTask(aRadioRecieveTask),
	mSettingsStore(aSettingsStore),
	mMessageCheckTask(aMessageCheckTask)
{
	   mMessageCheckTask.subscribe(this, RID_START_TRANSMIT_WINDOW, RID_START_TRANSMIT_WINDOW);
}

RadioTransmitTask::~RadioTransmitTask() {
	mMessageCheckTask.unsubscribe(this);
}

String RadioTransmitTask::getName() const {
	return "RadioTransmitTask";
}

void RadioTransmitTask::handleMessage(const IncomingRadioMessage& aIncomingRadioMessage) {
	uint32_t transmitDuration = Utils::bytesToInt(aIncomingRadioMessage.content()[0],
										aIncomingRadioMessage.content()[1],
										aIncomingRadioMessage.content()[2],
										aIncomingRadioMessage.content()[3]);

	if( mQueue == 0 ) {
		debug::log("RadioTransmitTask: incomingMessages queue has not been created");
	} else {
		if(xQueueSendToBack(mQueue, &transmitDuration, (TickType_t) 0) != pdPASS) {
	        debug::log("RadioTransmitTask: Could not queue incoming message");
	    }
	}
}

void RadioTransmitTask::task() {
	mQueue = xQueueCreate(10, sizeof(uint32_t));

	while(true) {
		uint32_t transmitDuration;
		if(xQueueReceive(mQueue, &transmitDuration, portMAX_DELAY) == pdTRUE) {
			mRadioReceiveTask.suspend();
			debug::log("RadioTransmitTask: transmit duration (ms): " + String(transmitDuration));

			uint32_t preDelay = random(0, transmitDuration);
			uint32_t postDelay = transmitDuration - preDelay;

			Tilda::delay(preDelay);
			respond();
			Tilda::delay(postDelay);

			mRadioReceiveTask.start();
        }
	}
}

void RadioTransmitTask::respond() {
	if (!mSettingsStore.hasBadgeId()) {
		debug::log("RadioTransmitTask: hasn't got a badge id");

		uint32_t uniqueId[4];
		if (mSettingsStore.getUniqueId(uniqueId)) {
			debug::log("RadioTransmitTask: send our uniqueId: " + String(uniqueId[0]) + ":" + String(uniqueId[1]) + ":" + String(uniqueId[2]) + ":" + String(uniqueId[3]));
			byte outgoingPacketBuffer[RADIO_PACKET_LENGTH];

			uint8_t index = 0;
			// RID
			outgoingPacketBuffer[index++] = 0x90;
			outgoingPacketBuffer[index++] = 0x02;
			// Badge id (we don't know our yet so send zero)
			outgoingPacketBuffer[index++] = 0x00;
			outgoingPacketBuffer[index++] = 0x00;
			// Our unique id
			for (int i = 0 ; i < 4 ; ++i) {
				outgoingPacketBuffer[index++] = static_cast<byte>(uniqueId[i]);
				outgoingPacketBuffer[index++] = static_cast<byte>(uniqueId[i] >> 8);
				outgoingPacketBuffer[index++] = static_cast<byte>(uniqueId[i] >> 16);
				outgoingPacketBuffer[index++] = static_cast<byte>(uniqueId[i] >> 24);
			}

			while (index < RADIO_PACKET_LENGTH) {
				outgoingPacketBuffer[index++] = 0;
			}

			RADIO_SERIAL.write(outgoingPacketBuffer, RADIO_PACKET_LENGTH);
			RADIO_SERIAL.flush();
		} else {
			// Oh noes! We should really never get here!
		}
	} else {
		debug::log("RadioTransmitTask: send hello/battery status");
		byte outgoingPacketBuffer[RADIO_PACKET_LENGTH];

		uint8_t index = 0;
		// RID
		outgoingPacketBuffer[index++] = 0x90;
		outgoingPacketBuffer[index++] = 0x04;
		// Badge id
		uint16_t badgeId = mSettingsStore.getBadgeId();
		outgoingPacketBuffer[index++] = static_cast<byte>(badgeId);
		outgoingPacketBuffer[index++] = static_cast<byte>(badgeId >> 8);
		// Battery status
		outgoingPacketBuffer[index++] = Tilda::getBatteryPercent();
    	outgoingPacketBuffer[index++] = Tilda::getChargeState();

		while (index < RADIO_PACKET_LENGTH) {
			outgoingPacketBuffer[index++] = 0;
		}

		RADIO_SERIAL.write(outgoingPacketBuffer, RADIO_PACKET_LENGTH);
		RADIO_SERIAL.flush();
	}
}
