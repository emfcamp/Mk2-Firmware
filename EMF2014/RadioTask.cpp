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
#include "Tilda.h"
#include "Utils.h"
#include "Deserializer.h"
#include <debug.h>

RadioTask::RadioTask() {
    _messages = xQueueCreate(RADIO_MAX_INCOMING_MESSAGES_BUFFER, sizeof(Serializable *));
    _wakeUpSignal = xQueueCreate(1, sizeof(byte));
    pinMode(PIN_LED_RXL, OUTPUT);
    _listening = false;
}

String RadioTask::getName() const {
    return "RadioTask";
}

void RadioTask::listen(uint8_t channel, uint16_t panId) {
    _channel = channel;
    _panId = panId;
    _changed = true;
    _listening = true;
    xQueueReset(_messages);
    byte meaningless = 1;
    xQueueOverwrite(_wakeUpSignal, &meaningless);
}

void RadioTask::close() {
    _listening = false;
    xQueueReset(_wakeUpSignal);
}

void RadioTask::sendMessage(Serializable& message) {
    byte data[58];

    // 2 Byte MessageTypeId
    uint16_t messageTypeId = message.getMessageTypeId();
    data[0] = static_cast<byte>(messageTypeId);
    data[1] = static_cast<byte>(messageTypeId >> 8);

    // Null the rest
    for (uint8_t i=2; i<58; i++) data[i] = 0;
    
    // Serialize
    message.serialize(data+2);
    
    // Send
    RADIO_SERIAL.write(data, 58);
    RADIO_SERIAL.flush();
}

void RadioTask::task() {
    while (true) {
        if (_listening) {
            byte packetBuffer[RADIO_PACKET_WITH_RSSI_LENGTH];
            uint8_t packetBufferLength = 0;
        
            while(_listening) {

                // Configuration
                if (_changed) {
                    Tilda::log("RadioTask: Setting up radio");
                    _wakeUp();
                    _enterAtMode();
                    RADIO_SERIAL.println("ATZD3");  // output format <payload>|<rssi>
                    RADIO_SERIAL.println("ATPK3A"); // 58byte packet length
                    RADIO_SERIAL.println(String("ATCN") + Utils::intToHex(_channel)); 
                    RADIO_SERIAL.println(String("ATID") + Utils::intToHex(_panId)); 
                    RADIO_SERIAL.println("ATAC");   // apply
                    RADIO_SERIAL.flush();
                    _leaveAtMode();
                    _clearSerialBuffer();
                    _changed = false;
                }

                // Read data
                uint8_t availableBytes = RADIO_SERIAL.available();
                if (availableBytes > 0) {
                    digitalWrite(PIN_LED_RXL, 1);

                    while (_listening && availableBytes > 0) {
                        packetBuffer[packetBufferLength] = RADIO_SERIAL.read();
                        packetBufferLength++;
                        availableBytes--;

                        // Only handle one packet at a time
                        if (packetBufferLength == RADIO_PACKET_WITH_RSSI_LENGTH) {
                            break; // we have enough for one packet
                        }
                    }

                    if (_listening) {
                        packetBufferLength = _parsePacketBuffer(packetBuffer, packetBufferLength);
                    }

                    digitalWrite(PIN_LED_RXL, 0);
                }

            }
        }

        Tilda::log("RadioTask: Going to sleep");
        _sleep();

        // Wait forever until woken up again
        byte meaningless;
        xQueueReceive(_wakeUpSignal, &meaningless, portMAX_DELAY);
    }
}

uint8_t RadioTask::_parsePacketBuffer(byte packetBuffer[], uint8_t packetBufferLength) {
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
            debug::logByteArray(packetBuffer, packetBufferLength);
        #endif

        Serializable* message = Deserializer::deserialize(packetBuffer);
        if (message != NULL) {
            if (xQueueSendToBack(_messages, ( void * ) &message, 0) == errQUEUE_FULL) {
                // ToDo: do something smart here
                debug::log("Incoming messages buffer is full. Clearing it.");
                xQueueReset(_messages); 
            }
        }

        packetBufferLength = 0;
    } else if (packetBufferLength == RADIO_PACKET_WITH_RSSI_LENGTH) {
        #ifdef RADIO_DEBUG_MODE
            debug::log("RadioTask: Packet does not conform");
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

void RadioTask::_enterAtMode() {
    digitalWrite(SRF_AT_COMMAND, LOW);
}

void RadioTask::_leaveAtMode() {
    vTaskDelay(10);
    digitalWrite(SRF_AT_COMMAND, HIGH);
}

void RadioTask::_sleep() {
    digitalWrite(SRF_SLEEP, HIGH);
}

void RadioTask::_wakeUp() {
    digitalWrite(SRF_SLEEP, LOW);
    vTaskDelay(RADIO_WAKE_UP_DURATION);
}

void RadioTask::_clearSerialBuffer() {
    while (RADIO_SERIAL.available()) RADIO_SERIAL.read();
}

Serializable* RadioTask::waitForMessage(TickType_t ticksToWait) {
    Serializable* message;
    if (xQueueReceive(_messages, &(message), ticksToWait) == pdTRUE) {
        return message;
    } 
    return NULL;
}

Serializable* RadioTask::waitForMessage() {
    return waitForMessage(portMAX_DELAY);
}