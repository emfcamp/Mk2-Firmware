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

#include <msgpack.h>

#include "DataStore.h"
#include "DebugTask.h"

WeatherForecast DataStore::_weatherForecast;

void DataStore::addContent(uint16_t rid, byte* content, uint16_t length) {
	if (rid == CONTENT_RID_WEATHER_FORECAST && length == 6 * 6) {
		_addWeatherForecastRaw(content, length);
	} else {
		debug::log("Rid or length not supported: " + String(rid) + " " + String(length));
	}
}

WeatherForecast DataStore::getWeatherForecast() {
	return _weatherForecast;
}

void unpack(WeatherForecastPeriod& period, const msgpack_object& object) {
	period.timestamp = object.via.array.ptr[0].via.u64;
	period.temperature = object.via.array.ptr[1].via.u64;
	period.feelsLikeTemperature = object.via.array.ptr[2].via.u64;
	period.windSpeed = object.via.array.ptr[3].via.u64;
	period.screenRelativeHumidity = object.via.array.ptr[4].via.u64;
	period.precipitationProbability = object.via.array.ptr[5].via.u64;
}

void DataStore::_addWeatherForecastRaw(const byte* content, uint16_t length) {
	_weatherForecast.valid = true;

	msgpack_zone mempool;
	msgpack_zone_init(&mempool, 2048);

	msgpack_object deserialized;
	msgpack_unpack((char*)content, length, NULL, &mempool, &deserialized);

	unpack(_weatherForecast.current, deserialized.via.array.ptr[0]);
	unpack(_weatherForecast.in3Hours, deserialized.via.array.ptr[1]);
	unpack(_weatherForecast.in6Hours, deserialized.via.array.ptr[2]);
	unpack(_weatherForecast.in12Hours, deserialized.via.array.ptr[3]);
	unpack(_weatherForecast.in24Hours, deserialized.via.array.ptr[4]);
	unpack(_weatherForecast.in48Hours, deserialized.via.array.ptr[5]);

	msgpack_zone_destroy(&mempool);

	debug::log("Stored weather forecast: " + String(_weatherForecast.current.temperature) + "deg, Weather type: " + String((uint8_t) _weatherForecast.current.weatherType));
}

