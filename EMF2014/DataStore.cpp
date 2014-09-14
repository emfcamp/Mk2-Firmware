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
#define CONTENT_RID_SCHEDULE_FRIDAY_START 0xA010
#define CONTENT_RID_SCHEDULE_SATURDAY_START 0xA020
#define CONTENT_RID_SCHEDULE_SUNDAY_START 0xA030

DataStore::DataStore(MessageCheckTask& aMessageCheckTask)
	:mMessageCheckTask(aMessageCheckTask)
{
	mWeatherForecast = new WeatherForecast;
	mWeatherForecast->mValid = false;
	mSchedule = new Schedule**[SCHEDULE_NUM_DAYS];
	for (int day = 0 ; day < SCHEDULE_NUM_DAYS ; ++day) {
		mSchedule[day] = new Schedule*[LOCATION_COUNT];
	}

	for (int day = 0 ; day < SCHEDULE_NUM_DAYS ; ++day) {
		for (int location = 0 ; location < LOCATION_COUNT ; ++location) {
			mSchedule[day][location] = new Schedule(NULL, 0);
		}
	}

	mWeatherSemaphore = xSemaphoreCreateMutex();
	mScheduleSemaphore = xSemaphoreCreateMutex();

    mMessageCheckTask.subscribe(this, RID_RANGE_CONTENT_START, RID_RANGE_CONTENT_END);
}

DataStore::~DataStore() {
    mMessageCheckTask.unsubscribe(this);

    delete mWeatherForecast;

	for (int day = 0 ; day < SCHEDULE_NUM_DAYS ; ++day) {
		for (int location = 0 ; location < LOCATION_COUNT ; ++location) {
			delete mSchedule[day][location];
		}
		delete[] mSchedule[day];
	}
    delete[] mSchedule;

    vSemaphoreDelete(mWeatherSemaphore);
    vSemaphoreDelete(mScheduleSemaphore);
}

void DataStore::handleMessage(const IncomingRadioMessage& aIncomingRadioMessage) {
	if (aIncomingRadioMessage.rid() == CONTENT_RID_WEATHER_FORECAST) {
		_addWeatherForecastRaw(aIncomingRadioMessage);
	} else if (aIncomingRadioMessage.rid() >= CONTENT_RID_SCHEDULE_FRIDAY_START
				&& aIncomingRadioMessage.rid() < CONTENT_RID_SCHEDULE_FRIDAY_START + LOCATION_COUNT) {
		_addScheduleRaw(aIncomingRadioMessage, SCHEDULE_FRIDAY, aIncomingRadioMessage.rid() - CONTENT_RID_SCHEDULE_FRIDAY_START);
	} else if (aIncomingRadioMessage.rid() >= CONTENT_RID_SCHEDULE_SATURDAY_START
				&& aIncomingRadioMessage.rid() < CONTENT_RID_SCHEDULE_SATURDAY_START + LOCATION_COUNT) {
		_addScheduleRaw(aIncomingRadioMessage, SCHEDULE_SATURDAY, aIncomingRadioMessage.rid() - CONTENT_RID_SCHEDULE_SATURDAY_START);
	} else if (aIncomingRadioMessage.rid() >= CONTENT_RID_SCHEDULE_SUNDAY_START
				&& aIncomingRadioMessage.rid() < CONTENT_RID_SCHEDULE_SUNDAY_START + LOCATION_COUNT) {
		_addScheduleRaw(aIncomingRadioMessage, SCHEDULE_SUNDAY, aIncomingRadioMessage.rid() - CONTENT_RID_SCHEDULE_SUNDAY_START);
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

Schedule* DataStore::getSchedule(uint8_t aDay, uint8_t aLocationId) const {
	Schedule* schedule = NULL;
	if (xSemaphoreTake(mScheduleSemaphore, portMAX_DELAY) == pdTRUE) {
		schedule = new Schedule(*mSchedule[aDay][aLocationId]);
		xSemaphoreGive(mScheduleSemaphore);
	}
	return schedule;
}

void DataStore::_unpackWeatherForecastPeriod(WeatherForecastPeriod& period, PackReader& reader) {
	period.timestamp = (uint32_t)Utils::getInteger(reader);
	period.weatherType = (WeatherType)Utils::getInteger(reader);
	period.temperature = (int8_t)Utils::getInteger(reader);
	period.feelsLikeTemperature = (int8_t)Utils::getInteger(reader);
	period.windSpeed = (uint8_t)Utils::getInteger(reader);
	period.screenRelativeHumidity = (uint8_t)Utils::getInteger(reader);
	period.precipitationProbability = (uint8_t)Utils::getInteger(reader);
}

void DataStore::_addWeatherForecastRaw(const IncomingRadioMessage& aIncomingRadioMessage) {
	if (xSemaphoreTake(mWeatherSemaphore, portMAX_DELAY) == pdTRUE) {
		mWeatherForecast->mValid = true;
		mReader.setBuffer((unsigned char*)aIncomingRadioMessage.content(), aIncomingRadioMessage.length());
		_unpackWeatherForecastPeriod(mWeatherForecast->mWeatherForecastPeriods[WEATHER_CURRENT], mReader);
		_unpackWeatherForecastPeriod(mWeatherForecast->mWeatherForecastPeriods[WEATHER_3_HOURS], mReader);
		_unpackWeatherForecastPeriod(mWeatherForecast->mWeatherForecastPeriods[WEATHER_6_HOURS], mReader);
		_unpackWeatherForecastPeriod(mWeatherForecast->mWeatherForecastPeriods[WEATHER_12_HOURS], mReader);
		_unpackWeatherForecastPeriod(mWeatherForecast->mWeatherForecastPeriods[WEATHER_24_HOURS], mReader);
		_unpackWeatherForecastPeriod(mWeatherForecast->mWeatherForecastPeriods[WEATHER_48_HOURS], mReader);

		debug::log("DataStore: Stored weather forecast: " +
				String(mWeatherForecast->mWeatherForecastPeriods[WEATHER_CURRENT].temperature) + "deg, Weather type: " + String((uint8_t) mWeatherForecast->mWeatherForecastPeriods[WEATHER_CURRENT].weatherType));

		xSemaphoreGive(mWeatherSemaphore);
	}
}

void DataStore::_addScheduleRaw(const IncomingRadioMessage& aIncomingRadioMessage, uint8_t aDay, uint8_t aLocation) {
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

		delete mSchedule[aDay][aLocation];
		mSchedule[aDay][aLocation] = new Schedule(events, eventCount);

		debug::log("DataStore: Got schedule: day: " +  String(aDay) + " location: " + String(aLocation) + " events: " + String(mSchedule[aDay][aLocation]->getEventCount()));

		// FIXME: delete events
		xSemaphoreGive(mScheduleSemaphore);
	}
}
