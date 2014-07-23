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
#include "DebugTask.h"

QueueHandle_t MessageCheckTask::incomingMessages;

String MessageCheckTask::getName() {
	return "MessageCheckTask";
}

void MessageCheckTask::addIncomingMessage(IncomingRadioMessage *message) {
	if( incomingMessages == 0 ) {
		debug::log("incomingMessages queue has not been created");
	} else {
		if(xQueueSendToBack(incomingMessages, (void *) &message, (TickType_t) 0) != pdPASS) {
	        debug::log("Could not queue incoming message");
	        free(message->content);
	        free(message);
	    }
	}
}

void MessageCheckTask::task() {
	incomingMessages = xQueueCreate(10, sizeof(struct IncomingRadioMessage *));

	while(true) {
		IncomingRadioMessage *message;
		if(xQueueReceive(incomingMessages, &(message), portMAX_DELAY) == pdTRUE) {
            // Create SHA1 digest
			Sha1.init();
			char receiverHi = message->receiver >> 8;
			char receiverLo = message->receiver & 0xFF;
		 	Sha1.print(receiverHi);
			Sha1.print(receiverLo);
			for (uint32_t i=0; i<message->length; i++) {
				Sha1.print((char)message->content[i]);
			}
			byte* digest = Sha1.result();	
			
			// Check our digest against the one send in the header
			if (memcmp(digest, message->hash, 12) != 0) {
				debug::log("Can't validate message, checksum doesn't match.");
			} else {

			    // Check ECC
			    if (!uECC_verify(EMF_PUBLIC_KEY, digest, message->signature)) {
			        debug::log("Can't validate message, ecc doesn't check out.");
			    } else {
			    	debug::log("Received message: " + String((char*)message->content));
			    }
			}
			free(message->content);
		    free(message);
        }
	}
}