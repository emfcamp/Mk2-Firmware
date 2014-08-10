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

#ifndef _DATA_STORE_H_
#define _DATA_STORE_H_

#include <Arduino.h>
#include <FreeRTOS_ARM.h>
#include <TinyPacks.h>

#include "Weather.h"
#include "EMF2014Config.h"

uint16_t static const CONTENT_RID_WEATHER_FORECAST = 40962;
uint16_t static const CONTENT_RID_SCHEDULE_FRIDAY = 40963;

class DataStore {
public:
	DataStore();
	~DataStore();

	void addContent(uint16_t rid, byte* content, uint16_t length);
	WeatherForecast& getWeatherForecast();

private:
	void _addWeatherForecastRaw(const byte* content, uint16_t length);
	void _addScheduleFridayRaw(const byte* content, uint16_t length);

	static void _unpackWeatherForecastPeriod(WeatherForecastPeriod& period, PackReader& reader);
	static tp_integer_t _getInteger(PackReader& reader);

private:
	WeatherForecast mWeatherForecast;

	PackReader mReader;
};

#endif // _DATA_STORE_H_