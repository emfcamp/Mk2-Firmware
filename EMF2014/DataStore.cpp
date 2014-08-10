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
#include "DebugTask.h"

DataStore::DataStore() {
	mWeatherForecast.valid = false;
}
 
DataStore::~DataStore() {
}

void DataStore::addContent(uint16_t rid, byte* content, uint16_t length) {
	if (rid == CONTENT_RID_WEATHER_FORECAST) {
		_addWeatherForecastRaw(content, length);
	} else if (rid == CONTENT_RID_SCHEDULE_FRIDAY) {
		//_addScheduleFridayRaw(content, length);
		debug::log("friday");
	} else {
		debug::log("Rid not supported: " + String(rid) + " " + String(length));
	}
}

WeatherForecast& DataStore::getWeatherForecast() {
	return mWeatherForecast;
}

tp_integer_t DataStore::_getInteger(PackReader& reader) {
	reader.next();
	return reader.getInteger();
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

	debug::log("Stored weather forecast: " +
				String(mWeatherForecast.current.temperature) + "deg, Weather type: " + String((uint8_t) mWeatherForecast.current.weatherType));
}

void DataStore::_addScheduleFridayRaw(const byte* content, uint16_t length) {
	/*
	stageId,
	typeId,
	startTimestamp,
	endTimestamp,
	speaker,
	event['title'],
	*/
}