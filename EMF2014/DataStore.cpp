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

WeatherForecast DataStore::_weatherForecast;

void DataStore::addContent(uint16_t rid, byte* content, uint16_t length) {
	if (rid == CONTENT_RID_WEATHER_FORECAST && length == 6 * 6) {
		_addWeatherForecastRaw(content);
	} else {
		debug::log("Rid or length not supported: " + String(rid) + " " + String(length));
	}
}

WeatherForecast DataStore::getWeatherForecast() {
	return _weatherForecast;
}

void DataStore::_addWeatherForecastRaw(byte* content) {
	_weatherForecast.valid = true;

	_weatherForecast.current.weatherType = (WeatherType)content[0];
	_weatherForecast.current.temperature = content[1];
	_weatherForecast.current.feelsLikeTemperature = content[2];
	_weatherForecast.current.windSpeed = content[3];
	_weatherForecast.current.screenRelativeHumidity = content[4];
	_weatherForecast.current.precipitationProbability = content[5];

	_weatherForecast.in3Hours.weatherType = (WeatherType)content[6];
	_weatherForecast.in3Hours.temperature = content[7];
	_weatherForecast.in3Hours.feelsLikeTemperature = content[8];
	_weatherForecast.in3Hours.windSpeed = content[9];
	_weatherForecast.in3Hours.screenRelativeHumidity = content[10];
	_weatherForecast.in3Hours.precipitationProbability = content[11];

	_weatherForecast.in6Hours.weatherType = (WeatherType)content[12];
	_weatherForecast.in6Hours.temperature = content[13];
	_weatherForecast.in6Hours.feelsLikeTemperature = content[14];
	_weatherForecast.in6Hours.windSpeed = content[15];
	_weatherForecast.in6Hours.screenRelativeHumidity = content[16];
	_weatherForecast.in6Hours.precipitationProbability = content[17];

	_weatherForecast.in12Hours.weatherType = (WeatherType)content[18];
	_weatherForecast.in12Hours.temperature = content[19];
	_weatherForecast.in12Hours.feelsLikeTemperature = content[20];
	_weatherForecast.in12Hours.windSpeed = content[21];
	_weatherForecast.in12Hours.screenRelativeHumidity = content[22];
	_weatherForecast.in12Hours.precipitationProbability = content[23];

	_weatherForecast.in24Hours.weatherType = (WeatherType)content[24];
	_weatherForecast.in24Hours.temperature = content[25];
	_weatherForecast.in24Hours.feelsLikeTemperature = content[26];
	_weatherForecast.in24Hours.windSpeed = content[27];
	_weatherForecast.in24Hours.screenRelativeHumidity = content[28];
	_weatherForecast.in24Hours.precipitationProbability = content[29];

	_weatherForecast.in48Hours.weatherType = (WeatherType)content[30];
	_weatherForecast.in48Hours.temperature = content[31];
	_weatherForecast.in48Hours.feelsLikeTemperature = content[32];
	_weatherForecast.in48Hours.windSpeed = content[33];
	_weatherForecast.in48Hours.screenRelativeHumidity = content[34];
	_weatherForecast.in48Hours.precipitationProbability = content[35];

	debug::log("Stored weather forecast: " + String(_weatherForecast.current.temperature) + "deg, Weather type: " + String((uint8_t) _weatherForecast.current.weatherType));
}

