/*
 TiLDA Mk2
 
 SoundTask
 Handle sound playback via the Piezo buzzer
 Play back request are sent via the ...
 
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

#include <Arduino.h>
#include <FreeRTOS_ARM.h>
#include "EMF2014Config.h"
#include "Task.h"

class Tone {
public:
	Tone();
	Tone(uint16_t note, uint16_t duration, uint16_t pauseAfterwards);
	void play();
private:
	uint16_t _note;
	uint16_t _duration;
	uint16_t _pauseAfterwards;
};

class SoundTask: public Task {
public:
    SoundTask();

    String getName() const;
    void playMelody(const uint16_t melody[], const uint16_t tempo[], const uint16_t length);
    void playTone(const uint16_t note, const uint16_t duration, const uint16_t pauseAfterwards);
    void playTone(const Tone& tone);
    void clear();
private:
    SoundTask(const SoundTask&) {}

    QueueHandle_t _tones;
protected:
    void task();
};
