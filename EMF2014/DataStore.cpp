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
#include <debug.h>

#define CONTENT_RID_WEATHER_FORECAST 40962
#define CONTENT_RID_SCHEDULE_FRIDAY 40963

#define MAX_TEXT_LENGTH 160

DataStore::DataStore() {
	mWeatherForecast.valid = false;
	mSchedule.events = new Event[0];
	mSchedule.numEvents = 0;
}
 
DataStore::~DataStore() {
}

void DataStore::addContent(uint16_t rid, const byte* content, uint16_t length) {
	if (rid == CONTENT_RID_WEATHER_FORECAST) {
		_addWeatherForecastRaw(content, length);
	} else if (rid == CONTENT_RID_SCHEDULE_FRIDAY) {
		_addScheduleFridayRaw(content, length);
	} else {
		debug::log("DataStore: Rid not supported: " + String(rid) + " " + String(length));
	}
}

const WeatherForecast& DataStore::getWeatherForecast() const {
	return mWeatherForecast;
}

const Schedule& DataStore::getSchedule() const {
	return mSchedule;
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

void DataStore::_addWeatherForecastRaw(const byte* content, uint16_t length) {
	mWeatherForecast.valid = true;

	mReader.setBuffer((unsigned char*)content, length);
	_unpackWeatherForecastPeriod(mWeatherForecast.current, mReader);
	_unpackWeatherForecastPeriod(mWeatherForecast.in3Hours, mReader);
	_unpackWeatherForecastPeriod(mWeatherForecast.in6Hours, mReader);
	_unpackWeatherForecastPeriod(mWeatherForecast.in12Hours, mReader);
	_unpackWeatherForecastPeriod(mWeatherForecast.in24Hours, mReader);
	_unpackWeatherForecastPeriod(mWeatherForecast.in48Hours, mReader);

	debug::log("DataStore: Stored weather forecast: " +
				String(mWeatherForecast.current.temperature) + "deg, Weather type: " + String((uint8_t) mWeatherForecast.current.weatherType));
}

void DataStore::_addScheduleFridayRaw(const byte* content, uint16_t length) {
	mReader.setBuffer((unsigned char*)content, length);

	// get the length and create a new array of events
	mSchedule.numEvents = _getInteger(mReader);
	delete[] mSchedule.events;
	mSchedule.events = new Event[mSchedule.numEvents];

	for (int i = 0 ; i < mSchedule.numEvents ; ++i) {
		mSchedule.events[i].stageId = (uint8_t)_getInteger(mReader);
		mSchedule.events[i].typeId = (uint8_t)_getInteger(mReader);
		mSchedule.events[i].startTimestamp = (uint32_t)_getInteger(mReader);
		mSchedule.events[i].endTimestamp = (uint32_t)_getInteger(mReader);
		mSchedule.events[i].speaker = _getString(mReader);
		mSchedule.events[i].title = _getString(mReader);
	}

	debug::log("DataStore: Got schedule: " + String(mSchedule.numEvents) + " events");
}
