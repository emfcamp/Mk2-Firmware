/*
 TiLDA Mk2
 
 PMICTask
 
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

#include "PMICTask.h"
#include "Tilda.h"
#include "BadgeNotifications.h"
#include <debug.h>

// EventGroup bita
#define PMIC_CHARGE_STATE_BIT   (1 << 0)
#define PMIC_SAMPLE_RATE_BIT    (1 << 1)


/**
 * PMIC Task class
 */

// Callbacks
static void PMICChargeStateInterrupt(void)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        // set the Charge state bit to wake the PMIC Task
        xEventGroupSetBitsFromISR(PMIC.eventGroup,
                                  PMIC_CHARGE_STATE_BIT,
                                  &xHigherPriorityTaskWoken);
        
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}


// Public
String PMICTask::getName() const
{
    return "PMICTask";
}

uint32_t PMICTask::getBatteryReading()
{
    return batteryReading;
}

float PMICTask::getBatteryVoltage()
{
    return (batteryReading * (3.3 /512));
}

uint8_t PMICTask::getBatteryPercent()
{
    return (batteryReading-PMIC_BATTERY_FLAT)*PMIC_BATTERY_PERCENT_RATIO;
    
}

uint8_t PMICTask::getChargeState()
{
    return chargeState;
}

int8_t PMICTask::setSampleRate(TickType_t ms)
{
    sampleRate = ms;
    xEventGroupSetBits(eventGroup, PMIC_SAMPLE_RATE_BIT);
}


// protected
void PMICTask::task()
{
    pinMode(MCP_STAT, INPUT_PULLUP);
    pinMode(VBATT_MON, INPUT);
    sampleRate = PMIC_DEFAULT_RATE;
    
    eventGroup = xEventGroupCreate();
    if (eventGroup == NULL) {
        debug::log("PMICTask: failed to create event group");
    }
    
    attachInterrupt(MCP_STAT, PMICChargeStateInterrupt, CHANGE);
    EventBits_t uxBits;
    
    // initial reading
    batteryReading = analogRead(VBATT_MON);
    chargeState = digitalRead(MCP_STAT);
    debug::log("PMICTask: Battery: " + String(getBatteryVoltage()) + " Charging: " + chargeState);
    
    if (batteryReading <= PMIC_BATTERY_VERYLOW) {
        // TODO: Panic and Charge now notifications
        debug::log("PMICTask: Battery Very Low on startup");
    } else if (batteryReading <= PMIC_BATTERY_LOW) {
        // TODO: Low battery notification
        debug::log("PMICTask: Battery Low on startup");
        Tilda::getBadgeNotifications().pushNotification("Battery low, please charge", RGBColor(0,0,0), RGBColor(0,0,0), false, 1);

    }
       
    while(true) {
        /* Wait a maximum of 100ms for either bit 0 or bit 4 to be set within
         the event group.  Clear the bits before exiting. */
        uxBits = xEventGroupWaitBits(eventGroup,
                                     PMIC_CHARGE_STATE_BIT | PMIC_SAMPLE_RATE_BIT,
                                     pdFALSE,
                                     pdFALSE,
                                     (sampleRate/portTICK_PERIOD_MS) );
        
        if ((uxBits & PMIC_SAMPLE_RATE_BIT) != 0 ) {
            // new sample rate notting todo but clear the bit and re enter the wait
            xEventGroupClearBits(eventGroup,
                                 PMIC_SAMPLE_RATE_BIT);
        }

        if ((uxBits & PMIC_CHARGE_STATE_BIT) != 0 ) {
            chargeState = digitalRead(MCP_STAT);
            xEventGroupClearBits(eventGroup,
                                 PMIC_CHARGE_STATE_BIT);
            debug::log("PMICTask: New charge state: " + String(chargeState));
            // TODO: notify others that want to know about state change
        }

        if ((uxBits & (PMIC_CHARGE_STATE_BIT | PMIC_SAMPLE_RATE_BIT)) == 0 ) {
            // wait timed out, time to sample the battery voltage
            batteryReading = analogRead(VBATT_MON);
            chargeState = digitalRead(MCP_STAT);
            debug::log("PMICTask: Battery: " + String(getBatteryVoltage()) + " Charging: " + chargeState);
            // check battery state and notify others
            if (batteryReading <= PMIC_BATTERY_VERYLOW) {
                // TODO: Panic and Charge now notifications
                debug::log("PMICTask: Battery Very Low");
                Tilda::getBadgeNotifications().pushNotification("BATTERY VERY LOW, TURN OFF AND CHARGE NOW", RGBColor(255,0,0), RGBColor(255,0,0), true, 1);
            } else if (batteryReading <= PMIC_BATTERY_LOW) {
                // TODO: Low battery notification
                debug::log("PMICTask: Battery Low");
                Tilda::getBadgeNotifications().pushNotification("Battery low, please charge", RGBColor(0,0,0), RGBColor(0,0,0), false, 1);
            }
        }

    }
}


// only once instance
PMICTask PMIC;
