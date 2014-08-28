/*
 TiLDA Mk2
 
 Weather 

 This handles the periodic wake of the radio for all our need communication with the gateway's.
 Incoming request are passed back to the TiLDATask
 Outgoing request from TiLDATask are sent at the next opportunity.
 
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

// This maps to http://www.metoffice.gov.uk/datapoint/support/documentation/code-definitions
enum WeatherType : uint8_t {
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
    uint32_t timestamp;
    int8_t temperature;
    int8_t feelsLikeTemperature;
    uint8_t windSpeed;
    uint8_t screenRelativeHumidity;
    uint8_t precipitationProbability;
};

enum WeatherPeriod : uint8_t  {
    WEATHER_CURRENT,
    WEATHER_3_HOURS,
    WEATHER_6_HOURS,
    WEATHER_12_HOURS,
    WEATHER_24_HOURS,
    WEATHER_48_HOURS,
    WEATHER_PERIOD_COUNT
};

struct WeatherForecast {
    bool mValid;
    WeatherForecastPeriod mWeatherForecastPeriods[WEATHER_PERIOD_COUNT];
};
