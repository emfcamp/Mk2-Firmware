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
#include "EMF2014Config.h"

uint16_t static const CONTENT_RID_WEATHER_FORECAST = 40962;

// This maps to http://www.metoffice.gov.uk/datapoint/support/documentation/code-definitions
enum class WeatherType : uint8_t {
	WEATHER_TYPE_NOT_AVAILABLE = 31,
	WEATHER_TYPE_CLEAR_NIGHT = 0,
	WEATHER_TYPE_SUNNY_NIGHT = 1,
	WEATHER_TYPE_PARTLY_CLOUDY_NIGHT = 2,
	WEATHER_TYPE_PARTLY_CLOUDY_DAY = 3,
	WEATHER_TYPE_MIST = 5,
	WEATHER_TYPE_FOG = 6,
	WEATHER_TYPE_CLOUDY = 7,
	WEATHER_TYPE_OVERCAST = 8,
	WEATHER_TYPE_LIGHT_RAIN_SHOWER_NIGHT = 9,
	WEATHER_TYPE_LIGHT_RAIN_SHOWER_DAY = 10,
	WEATHER_TYPE_DRIZZLE = 11,
	WEATHER_TYPE_LIGHT_RAIN = 12,
	WEATHER_TYPE_HEAVY_RAIN_SHOWER_NIGHT = 13,
	WEATHER_TYPE_HEAVY_RAIN_SHOWER_DAY = 14,
	WEATHER_TYPE_HEAVY_RAIN = 15,
	WEATHER_TYPE_SLEET_SHOWER_NIGHT = 16,
	WEATHER_TYPE_SLEET_SHOWRT_DAY = 17,
	WEATHER_TYPE_SLEET = 18,
	WEATHER_TYPE_HAIL_SHOWER_NIGHT = 19,
	WEATHER_TYPE_HAIL_SHOWER_DAY = 20,
	WEATHER_TYPE_HAIL = 21,
	WEATHER_TYPE_LIGHT_SNOW_SHOWER_NIGHT = 22,
	WEATHER_TYPE_LIGHT_SNOW_SHOWER_DAY = 23,
	WEATHER_TYPE_LIGHT_SNOW = 24,
	WEATHER_TYPE_HEAVY_SNOW_SHOWER_NIGHT = 25,
	WEATHER_TYPE_HEAVY_SNOW_SHOWER_DAY = 26,
	WEATHER_TYPE_HEAVY_SNOW = 27,
	WEATHER_TYPE_THUNDER_SHOWER_NIGHT = 28,
	WEATHER_TYPE_THUNDER_SHOWER_DAY = 29 ,
	WEATHER_TYPE_THUNDER = 30
};

struct WeatherForecastPeriod {
	WeatherType weatherType;
	int8_t temperature;
	int8_t feelsLikeTemperature;
	uint8_t windSpeed;
	uint8_t screenRelativeHumidity;
	uint8_t precipitationProbability;
};

struct WeatherForecast {
	bool valid;
	WeatherForecastPeriod current;
	WeatherForecastPeriod in3Hours;
	WeatherForecastPeriod in6Hours;
	WeatherForecastPeriod in12Hours;
	WeatherForecastPeriod in24Hours;
	WeatherForecastPeriod in48Hours;
};


class DataStore {
public:
	static void addContent(uint16_t rid, byte* content, uint16_t length);
	static WeatherForecast getWeatherForecast();
private:
	static WeatherForecast _weatherForecast;

	static void _addWeatherForecastRaw(byte* content);
};

#endif // _DATA_STORE_H_