/*
 TiLDA Mk2
 
 Data Store

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

#include "DataStore.h"
#include "IncomingRadioMessage.h"
#include "MessageCheckTask.h"
#include "Weather.h"
#include "Schedule.h"

#include <debug.h>

#define CONTENT_RID_WEATHER_FORECAST 40962
#define CONTENT_RID_SCHEDULE_FRIDAY 40963

#define MAX_TEXT_LENGTH 160

DataStore::DataStore(MessageCheckTask& aMessageCheckTask)
	:mMessageCheckTask(aMessageCheckTask)
{
	mWeatherForecast = new WeatherForecast;
	mWeatherForecast->valid = false;
	mSchedule = new Schedule(NULL, 0);

	mWeatherSemaphore = xSemaphoreCreateBinary();
	mScheduleSemaphore = xSemaphoreCreateBinary();

    mMessageCheckTask.subscribe(this, RID_RANGE_CONTENT_START, RID_RANGE_CONTENT_END);
}
 
DataStore::~DataStore() {
    mMessageCheckTask.unsubscribe(this);

    delete mWeatherForecast;
    delete mSchedule;

    vSemaphoreDelete(mWeatherSemaphore);
    vSemaphoreDelete(mScheduleSemaphore);
}

void DataStore::handleMessage(const IncomingRadioMessage& aIncomingRadioMessage) {
	if (aIncomingRadioMessage.rid() == CONTENT_RID_WEATHER_FORECAST) {
		_addWeatherForecastRaw(aIncomingRadioMessage);
	} else if (aIncomingRadioMessage.rid() == CONTENT_RID_SCHEDULE_FRIDAY) {
		_addScheduleFridayRaw(aIncomingRadioMessage);
	} else {
		debug::log("DataStore: Rid not supported: " + String(aIncomingRadioMessage.rid()) + " " + String(aIncomingRadioMessage.length()));
	}
}

WeatherForecast* DataStore::getWeatherForecast() const {
	WeatherForecast* weather = NULL;
	if (xSemaphoreTake(mWeatherSemaphore, portMAX_DELAY) == pdTRUE) {
		weather = new WeatherForecast;
		*weather = *mWeatherForecast;
		xSemaphoreGive(mWeatherSemaphore);
	}
	return weather;
}

Schedule* DataStore::getSchedule() const {
	Schedule* schedule = NULL;
	if (xSemaphoreTake(mScheduleSemaphore, portMAX_DELAY) == pdTRUE) {
		Schedule* schedule = new Schedule(*mSchedule);
		xSemaphoreGive(mScheduleSemaphore);
	}
	return schedule;
}

tp_integer_t DataStore::_getInteger(PackReader& reader) {
	reader.next();
	return reader.getInteger();
}

String DataStore::_getString(PackReader& reader) {
	reader.next();
	char string[MAX_TEXT_LENGTH];
	tp_length_t legnth = reader.getString(string, MAX_TEXT_LENGTH);
	return String(string);
}

void DataStore::_unpackWeatherForecastPeriod(WeatherForecastPeriod& period, PackReader& reader) {
	period.timestamp = (uint32_t)_getInteger(reader);
	period.weatherType = (WeatherType)_getInteger(reader);
	period.temperature = (int8_t)_getInteger(reader);
	period.windSpeed = (uint8_t)_getInteger(reader);
	period.screenRelativeHumidity = (uint8_t)_getInteger(reader);
	period.precipitationProbability = (uint8_t)_getInteger(reader);
}

void DataStore::_addWeatherForecastRaw(const IncomingRadioMessage& aIncomingRadioMessage) {
	if (xSemaphoreTake(mWeatherSemaphore, portMAX_DELAY) == pdTRUE) {
		mWeatherForecast->valid = true;
		mReader.setBuffer((unsigned char*)aIncomingRadioMessage.content(), aIncomingRadioMessage.length());
		_unpackWeatherForecastPeriod(mWeatherForecast->current, mReader);
		_unpackWeatherForecastPeriod(mWeatherForecast->in3Hours, mReader);
		_unpackWeatherForecastPeriod(mWeatherForecast->in6Hours, mReader);
		_unpackWeatherForecastPeriod(mWeatherForecast->in12Hours, mReader);
		_unpackWeatherForecastPeriod(mWeatherForecast->in24Hours, mReader);
		_unpackWeatherForecastPeriod(mWeatherForecast->in48Hours, mReader);

		debug::log("DataStore: Stored weather forecast: " +
				String(mWeatherForecast->current.temperature) + "deg, Weather type: " + String((uint8_t) mWeatherForecast->current.weatherType));

		xSemaphoreGive(mWeatherSemaphore);
	}
}

void DataStore::_addScheduleFridayRaw(const IncomingRadioMessage& aIncomingRadioMessage) {
	if (xSemaphoreTake(mScheduleSemaphore, portMAX_DELAY) == pdTRUE) {
		mReader.setBuffer((unsigned char*)aIncomingRadioMessage.content(), aIncomingRadioMessage.length());

		// get the length and create a new array of events
		tp_integer_t eventCount = _getInteger(mReader);
		Event* events = new Event[eventCount];

		for (int i = 0 ; i < eventCount ; ++i) {
			events[i].stageId = (uint8_t)_getInteger(mReader);
			events[i].typeId = (uint8_t)_getInteger(mReader);
			events[i].startTimestamp = (uint32_t)_getInteger(mReader);
			events[i].endTimestamp = (uint32_t)_getInteger(mReader);
			events[i].speaker = _getString(mReader);
			events[i].title = _getString(mReader);
		}

		delete mSchedule;
		mSchedule = new Schedule(events, eventCount);

		debug::log("DataStore: Got schedule: " + String(mSchedule->getEventCount()) + " events");

		xSemaphoreGive(mScheduleSemaphore);
	}
}
