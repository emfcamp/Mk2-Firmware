/*
 TiLDA Mk2

 DataStore

 This is a helper class that knows how to de-serialise and store data send via
 radio. Other apps/tasks can look up data

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
#include <TinyPacks.h>

#include "EMF2014Config.h"
#include "RadioMessageHandler.h"
#include "Schedule.h"

class IncomingRadioMessage;
class MessageCheckTask;
class WeatherForecast;
class WeatherForecastPeriod;

class DataStore: public RadioMessageHandler {
public:
	DataStore(MessageCheckTask& aMessageCheckTask);
	~DataStore();

	WeatherForecast* getWeatherForecast() const;
	Schedule* getSchedule(uint8_t aDay, uint8_t aLocationId) const;

private: // from RadioMessageHandler
	void handleMessage(const IncomingRadioMessage& aIncomingRadioMessage);

private:
	void _addWeatherForecastRaw(const IncomingRadioMessage& aIncomingRadioMessage);
	void _addScheduleRaw(const IncomingRadioMessage& aIncomingRadioMessage, uint8_t day, uint8_t aLocationId);

	static void _unpackWeatherForecastPeriod(WeatherForecastPeriod& period, PackReader& reader);

private:
	MessageCheckTask& mMessageCheckTask;

	PackReader mReader;

	// data
	WeatherForecast* mWeatherForecast;
	Schedule*** mSchedule;

	SemaphoreHandle_t mWeatherSemaphore;
	SemaphoreHandle_t mScheduleSemaphore;
};
