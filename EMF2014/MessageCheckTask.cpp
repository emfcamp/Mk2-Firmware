/*
 TiLDA Mk2

 Message Check Task

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

#include "MessageCheckTask.h"

#include <uECC.h>
#include <Sha1.h>
#include <debug.h>
#include "DataStore.h"
#include "RadioMessageHandler.h"
#include "IncomingRadioMessage.h"

#define MAX_HANDLERS 20

MessageCheckTask::MessageCheckTask() {
	mHandlers = new HandlerItem*[MAX_HANDLERS];

	for (int i = 0 ; i < MAX_HANDLERS ; ++i) {
		mHandlers[i] = NULL;
	}

	mHandlersSemaphore = xSemaphoreCreateMutex();
}

MessageCheckTask::~MessageCheckTask() {
	delete[] mHandlers;
	vSemaphoreDelete(mHandlersSemaphore);
}

String MessageCheckTask::getName() const {
	return "MessageCheckTask";
}

void MessageCheckTask::subscribe(RadioMessageHandler* aHandler, uint16_t aRangeStart, uint16_t aRangeEnd) {
	if (xSemaphoreTake(mHandlersSemaphore, 100) == pdTRUE) {
		for (int i = 0 ; i < MAX_HANDLERS ; ++i) {
			if (mHandlers[i] == NULL) {
				mHandlers[i] = new HandlerItem;
				mHandlers[i]->mHandler = aHandler;
				mHandlers[i]->mRangeStart = aRangeStart;
				mHandlers[i]->mRangeEnd = aRangeEnd;
				break;
			}
		}

        xSemaphoreGive(mHandlersSemaphore);
    } else {
    	debug::log("MessageCheckTask: Failed to get Semaphore for subscribe");
    }
}

void MessageCheckTask::unsubscribe(RadioMessageHandler* aHandler) {
	if (xSemaphoreTake(mHandlersSemaphore, 100 ) == pdTRUE) {
		for (int i = 0 ; i < MAX_HANDLERS ; ++i) {
			if (mHandlers[i]->mHandler == aHandler) {
				delete mHandlers[i];
				mHandlers[i] = NULL;
			}
		}

		xSemaphoreGive(mHandlersSemaphore);
	} else {
    	debug::log("MessageCheckTask: Failed to get Semaphore for unsubscribe");
    }
}

void MessageCheckTask::addIncomingMessage(IncomingRadioMessage *message) {
	if( mIncomingMessages == 0 ) {
		debug::log("MessageCheckTask: incomingMessages queue has not been created");
	} else {
		if(xQueueSendToBack(mIncomingMessages, (void *) &message, (TickType_t) 0) != pdPASS) {
	        debug::log("MessageCheckTask: Could not queue incoming message");
	        delete message;
	    }
	}
}

void MessageCheckTask::task() {
	mIncomingMessages = xQueueCreate(10, sizeof(struct IncomingRadioMessage *));

	while(true) {
		IncomingRadioMessage *message;
		if(xQueueReceive(mIncomingMessages, &message, portMAX_DELAY) == pdTRUE) {
            // Create SHA1 digest
			byte* digest = message->Sha1Result();

			// Check our digest against the one send in the header
			if (memcmp(digest, message->hash(), 12) != 0) {
				debug::log("MessageCheckTask: Can't validate message, checksum doesn't match.");
			} else {
			    // Check ECC
				if (!uECC_verify(EMF_PUBLIC_KEY, digest, message->signature())) {
					debug::log("MessageCheckTask: Can't validate message, ecc doesn't check out.");
				} else {
					// we have a valid message so tell the handlers
					#ifdef RADIO_DEBUG_MODE
						debug::log("MessageCheckTask: valid message. RID: " + String(message->rid()));
					#endif

					bool dispatched = false;
					for (int i = 0 ; i < MAX_HANDLERS ; ++i) {
						if (mHandlers[i] != NULL
								&& message->rid() >= mHandlers[i]->mRangeStart
								&& message->rid() <= mHandlers[i]->mRangeEnd) {
							#ifdef RADIO_DEBUG_MODE
								debug::log("MessageCheckTask: dispatched!");
							#endif
							mHandlers[i]->mHandler->handleMessage(*message);
							dispatched = true;
						}
					}

					if (!dispatched) {
						debug::log("MessageCheckTask: Unhandled valid message for RID: " + String(message->rid()));
					}
			    }
			}

			delete message;
        }
	}
}
