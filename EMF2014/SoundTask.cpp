/*
 TiLDA Mk2
 
 SoundTask
 
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

#include "SoundTask.h"
#include "Tilda.h"

Tone::Tone() {
    _note = 0;
    _duration = 0;
    _pauseAfterwards = 0;
};

Tone::Tone(uint16_t note, uint16_t duration, uint16_t pauseAfterwards) {
    _note = note;
    _duration = duration;
    _pauseAfterwards = pauseAfterwards;
};

void Tone::play() {
    tone(PIEZO, _note, _duration);
    vTaskDelay(_pauseAfterwards);
    digitalWrite(PIEZO, LOW);
}

SoundTask::SoundTask() {
    _tones = xQueueCreate(1024, sizeof(Tone));
}

// ToDo: Add some type of event handler that gets called when the melody/tone has finished
void SoundTask::playMelody(const uint16_t melody[], const uint16_t tempo[], const uint16_t length) {
    clear();
    for (uint16_t index = 0; index < length; index++) {
        // calculate note duration
        const uint16_t noteDuration = 1000 / tempo[index];
        const uint16_t pauseAfterwards = noteDuration * 1.30;

        // queue up for future use
        playTone(melody[index], noteDuration, pauseAfterwards);
    }
}

void SoundTask::playTone(const uint16_t note, const uint16_t duration, const uint16_t pauseAfterwards) {
    Tone tone(note, duration, pauseAfterwards);
    playTone(tone);
}

void SoundTask::playTone(const Tone& tone) {
    xQueueSendToBack(_tones, (void *) &tone, (TickType_t) 0);
}

void SoundTask::clear() {
    xQueueReset(_tones);
}

String SoundTask::getName() const {
    return "SoundTask";
}

void SoundTask::task() {
    while(true) {
        Tone tone;
        if(xQueueReceive(_tones, &tone, portMAX_DELAY) == pdTRUE) {
            tone.play();
        }
    }
}
