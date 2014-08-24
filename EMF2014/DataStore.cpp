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
#include "Utils.h"

#include <debug.h>

#define CONTENT_RID_WEATHER_FORECAST  40962
#define CONTENT_RID_SCHEDULE_FRIDAY   40963
#define CONTENT_RID_SCHEDULE_SATURDAY 40964
#define CONTENT_RID_SCHEDULE_SUNDAY   40965

DataStore::DataStore(MessageCheckTask& aMessageCheckTask)
	:mMessageCheckTask(aMessageCheckTask)
{
	mWeatherForecast = new WeatherForecast;
	mWeatherForecast->valid = false;
	mSchedule = new Schedule*[3];
	mSchedule[SCHEDULE_FRIDAY] = new Schedule(NULL, 0);
	mSchedule[SCHEDULE_SATURDAY] = new Schedule(NULL, 0);
	mSchedule[SCHEDULE_SUNDAY] = new Schedule(NULL, 0);

	mWeatherSemaphore = xSemaphoreCreateMutex();
	mScheduleSemaphore = xSemaphoreCreateMutex();

    mMessageCheckTask.subscribe(this, RID_RANGE_CONTENT_START, RID_RANGE_CONTENT_END);
}

DataStore::~DataStore() {
    mMessageCheckTask.unsubscribe(this);

    delete mWeatherForecast;
    delete[] mSchedule;

    vSemaphoreDelete(mWeatherSemaphore);
    vSemaphoreDelete(mScheduleSemaphore);
}

void DataStore::handleMessage(const IncomingRadioMessage& aIncomingRadioMessage) {
	if (aIncomingRadioMessage.rid() == CONTENT_RID_WEATHER_FORECAST) {
		_addWeatherForecastRaw(aIncomingRadioMessage);
	} else if (aIncomingRadioMessage.rid() == CONTENT_RID_SCHEDULE_FRIDAY) {
		_addScheduleRaw(aIncomingRadioMessage, SCHEDULE_FRIDAY);
	} else if (aIncomingRadioMessage.rid() == CONTENT_RID_SCHEDULE_SATURDAY) {
		_addScheduleRaw(aIncomingRadioMessage, SCHEDULE_SATURDAY);
	} else if (aIncomingRadioMessage.rid() == CONTENT_RID_SCHEDULE_SUNDAY) {
		_addScheduleRaw(aIncomingRadioMessage, SCHEDULE_SUNDAY);
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

Schedule* DataStore::getSchedule(ScheduleDay day) const {
	Schedule* schedule = NULL;
	if (xSemaphoreTake(mScheduleSemaphore, portMAX_DELAY) == pdTRUE) {
		schedule = new Schedule(*mSchedule[day]);
		xSemaphoreGive(mScheduleSemaphore);
	}
	return schedule;
}

void DataStore::_unpackWeatherForecastPeriod(WeatherForecastPeriod& period, PackReader& reader) {
	period.timestamp = (uint32_t)Utils::getInteger(reader);
	period.weatherType = (WeatherType)Utils::getInteger(reader);
	period.temperature = (int8_t)Utils::getInteger(reader);
	period.windSpeed = (uint8_t)Utils::getInteger(reader);
	period.screenRelativeHumidity = (uint8_t)Utils::getInteger(reader);
	period.precipitationProbability = (uint8_t)Utils::getInteger(reader);
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

void DataStore::_addScheduleRaw(const IncomingRadioMessage& aIncomingRadioMessage, ScheduleDay day) {
	if (xSemaphoreTake(mScheduleSemaphore, portMAX_DELAY) == pdTRUE) {
		mReader.setBuffer((unsigned char*)aIncomingRadioMessage.content(), aIncomingRadioMessage.length());

		// get the length and create a new array of events
		tp_integer_t eventCount = Utils::getInteger(mReader);
		Event* events = new Event[eventCount];

		for (int i = 0 ; i < eventCount ; ++i) {
			events[i].locationId = (LocationId)Utils::getInteger(mReader);
			events[i].typeId = (uint8_t)Utils::getInteger(mReader);
			events[i].startTimestamp = (uint32_t)Utils::getInteger(mReader);
			events[i].endTimestamp = (uint32_t)Utils::getInteger(mReader);
			events[i].speaker = Utils::getString(mReader);
			events[i].title = Utils::getString(mReader);
		}

		delete mSchedule[day];
		mSchedule[day] = new Schedule(events, eventCount);

		debug::log("DataStore: Got schedule: " + String(mSchedule[day]->getEventCount()) + " events");

		xSemaphoreGive(mScheduleSemaphore);
	}
}
