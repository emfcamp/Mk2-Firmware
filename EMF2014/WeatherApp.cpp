/*
 TiLDA Mk2

 Task

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

#include <FreeRTOS_ARM.h>
#include <debug.h>
#include "WeatherApp.h"
#include "Tilda.h"
#include <glcd.h>
#include <M2tk.h>
#include "GUITask.h"
#include "Utils.h"
#include "DataStore.h"

char weather_string[200];
M2_LABEL(weather_label, "W64", weather_string);
M2_ALIGN(weather_align, "-1|2W64H64", &weather_label);

App* WeatherApp::New() {
    return new WeatherApp;
}

WeatherApp::WeatherApp() {
    mButtonSubscription = Tilda::createButtonSubscription(UP | DOWN | LEFT | RIGHT);
}

WeatherApp::~WeatherApp() {
    delete mButtonSubscription;
}

String WeatherApp::getName() const {
    return "WeatherApp";
}

String WeatherApp::getWeatherTypeString(WeatherType aWeatherType) {
    switch (aWeatherType) {
        case WEATHER_TYPE_NOT_AVAILABLE: return "Not available";
        case WEATHER_TYPE_CLEAR_NIGHT: return "Clear night";
        case WEATHER_TYPE_SUNNY_NIGHT: return "Sunny night";
        case WEATHER_TYPE_PARTLY_CLOUDY_NIGHT:
        case WEATHER_TYPE_PARTLY_CLOUDY_DAY: return "Partly cloudy";
        case WEATHER_TYPE_MIST : return "Mist";
        case WEATHER_TYPE_FOG : return "Fog";
        case WEATHER_TYPE_CLOUDY : return "Cloudy";
        case WEATHER_TYPE_OVERCAST : return "Overcast";
        case WEATHER_TYPE_LIGHT_RAIN_SHOWER_NIGHT:
        case WEATHER_TYPE_LIGHT_RAIN_SHOWER_DAY: return "Light rain shower";
        case WEATHER_TYPE_DRIZZLE: return "Drizzle";
        case WEATHER_TYPE_LIGHT_RAIN: return "Light rain";
        case WEATHER_TYPE_HEAVY_RAIN_SHOWER_NIGHT:
        case WEATHER_TYPE_HEAVY_RAIN_SHOWER_DAY: return "Heavy rain shower";
        case WEATHER_TYPE_HEAVY_RAIN: return "Heavy rain";
        case WEATHER_TYPE_SLEET_SHOWER_NIGHT:
        case WEATHER_TYPE_SLEET_SHOWRT_DAY: return "Sleet shower";
        case WEATHER_TYPE_SLEET: return "Sleet";
        case WEATHER_TYPE_HAIL_SHOWER_NIGHT:
        case WEATHER_TYPE_HAIL_SHOWER_DAY: return "Hail shower";
        case WEATHER_TYPE_HAIL: return "Hail";
        case WEATHER_TYPE_LIGHT_SNOW_SHOWER_NIGHT:
        case WEATHER_TYPE_LIGHT_SNOW_SHOWER_DAY: return "Light snow shower";
        case WEATHER_TYPE_LIGHT_SNOW: return "Light snow";
        case WEATHER_TYPE_HEAVY_SNOW_SHOWER_NIGHT:
        case WEATHER_TYPE_HEAVY_SNOW_SHOWER_DAY: return "Heavy snow shower";
        case WEATHER_TYPE_HEAVY_SNOW: return "Heavy snow";
        case WEATHER_TYPE_THUNDER_SHOWER_NIGHT:
        case WEATHER_TYPE_THUNDER_SHOWER_DAY: return "Thunder shower";
        case WEATHER_TYPE_THUNDER: return "Thunder!";
    }

    return "Unknown";
}

String WeatherApp::getDayOfWeekString(const RTC_date_time& timestamp) {
    switch (RTC_clock::calculate_day_of_week(timestamp.year, timestamp.month, timestamp.day)) {
        case 0: return "Sun";
        case 1: return "Mon";
        case 2: return "Tue";
        case 3: return "Wed";
        case 4: return "Thu";
        case 5: return "Fri";
        case 6: return "Sat";
    }

    return "Funday!";
}

bool WeatherApp::getWeatherString(String& forecastStr, const WeatherForecast& aWeatherForecast, uint8_t aWeatherPeriod) {
    if (aWeatherForecast.mValid) {
        const WeatherForecastPeriod& wfp = aWeatherForecast.mWeatherForecastPeriods[aWeatherPeriod];
        RTC_date_time timestamp = RTC_clock::from_unixtime(wfp.timestamp + TIMEZONE_OFFSET);
        forecastStr = "Weather for " + getDayOfWeekString(timestamp) + " " + String(timestamp.hour) + ":" + ((timestamp.minute<10)?"0":"") + String(timestamp.minute) + "\n"; 
        forecastStr += getWeatherTypeString(wfp.weatherType) + "\n";
        forecastStr += "Temp: " + String(wfp.temperature) + " deg C\n";
        forecastStr += "Feels: " + String(wfp.feelsLikeTemperature) + " deg C\n";
        forecastStr += "Windspeed: " + String(wfp.windSpeed) + "mph\n";
        forecastStr += String(wfp.screenRelativeHumidity) + "% humidity\n";
        forecastStr += String(wfp.precipitationProbability) + "% chance of rain\n";
    } else {
        forecastStr = "Sorry, no forecast recieved yet. Please try later (or wiggle the joystick to check for update).";
    }
    return aWeatherForecast.mValid;
}

void WeatherApp::task() {
    GLCD.SetRotation(ROTATION_0);

    uint period = WEATHER_CURRENT;
    String weatherString;

    while (true) {
        WeatherForecast* weatherForecast = Tilda::getDataStore().getWeatherForecast();
        bool valid = getWeatherString(weatherString, *weatherForecast, period);
        Utils::wordWrap(weather_string, weatherString.c_str(), 21, 7);
        Tilda::getGUITask().setM2Root(&weather_align);
        delete weatherForecast;

        Button button = mButtonSubscription->waitForPress();

        if (valid) {
            // only update the position once we've shown a forecast
            // the current forecast is then the first seen
            switch (button) {
                case UP:
                case RIGHT:
                    period = (WeatherPeriod)(period + 1) % WEATHER_PERIOD_COUNT;
                    break;
                case DOWN:
                case LEFT:
                    period = (period == 0) ?
                                WEATHER_PERIOD_COUNT - 1:
                                (period - 1) % WEATHER_PERIOD_COUNT;
                    break;
            }
        }
    }
}

