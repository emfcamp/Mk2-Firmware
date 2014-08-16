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
#include "DebugTask.h"
#include "DataStore.h"
#include "RadioMessageHandler.h"

#define RID_RANGE_UNINDENTIFIED_BADGE 0x0000 //Unidentified Badges
#define RID_RANGE_BADGE_ID_START 0x0001
#define RID_RANGE_BADGE_ID_END 0x8FFF // Badge IDs (16384 badges max)
#define RID_RANGE_SPECIAL_START 0x9000
#define RID_RANGE_SPECIAL_END 0x9FFF //Special backend service (e.g. badge id negotiation)
#define RID_RANGE_CONTENT_START 0xA000
#define RID_RANGE_CONTENT_END 0xAFFF //Content (e.g. Schedule Saturday, Weather forecast)
#define RID_RANGE_NON_CONTENT_START 0xB000
#define RID_RANGE_NON_CONTENT_END 0xBFFF //Special non-content broadcasts (e.g. start reply-window, reply with badge id)
#define RID_RANGE_RESERVED_START 0xC000
#define RID_RANGE_RESERVED_RESERVED_END 0xFFFF //Reserved

MessageCheckTask::MessageCheckTask() {
}

MessageCheckTask::~MessageCheckTask() {
}

String MessageCheckTask::getName() const {
	return "MessageCheckTask";
}

void MessageCheckTask::setContentHandler(RadioMessageHandler& aHandler) {
	mContentHandler = &aHandler;
}

void MessageCheckTask::setNonContentHandler(RadioMessageHandler& aHandler) {
	mNonContentHandler = &aHandler;
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
					if (message->rid() <= RID_RANGE_CONTENT_START &&
			    		 message->rid() >= RID_RANGE_CONTENT_END) {
						mContentHandler->handleMessage(*message);
			    	} else if (message->rid() <= RID_RANGE_NON_CONTENT_START &&
			    		 message->rid() >= RID_RANGE_NON_CONTENT_END) {

			    	}
			    }
			}

			delete message;
        }
	}
}