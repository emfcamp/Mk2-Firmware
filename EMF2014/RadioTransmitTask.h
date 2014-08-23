/*
 TiLDA Mk2

 RadioTransmitTask
 This handles the periodic wake of the radio for all our need communication with the gateway's.
 Incoming request are passed back to the TiLDATask
 Outgoing request from TiLDATask are sent at the next opportunity.

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

#pragma once

#include <Arduino.h>
#include <FreeRTOS_ARM.h>

#include "EMF2014Config.h"
#include "Task.h"
#include "RadioReceiveTask.h"
#include "RadioMessageHandler.h"

class SettingsStore;
class MessageCheckTask;

class RadioTransmitTask: public Task, public RadioMessageHandler {
public:
	RadioTransmitTask(RadioReceiveTask& aRadioReceiveTask, const SettingsStore& aSettingsStore, MessageCheckTask& aMessageCheckTask);
    ~RadioTransmitTask();

	String getName() const;

private:
    RadioTransmitTask(const RadioTransmitTask&);

    void respond();

private: // from RadioMessageHandler
	void handleMessage(const IncomingRadioMessage& aIncomingRadioMessage);
    inline void _sleep();
    inline void _wakeUp();
protected:
	void task();

private:
	RadioReceiveTask& mRadioReceiveTask;
    const SettingsStore& mSettingsStore;
    MessageCheckTask& mMessageCheckTask;

	QueueHandle_t mQueue;
};
