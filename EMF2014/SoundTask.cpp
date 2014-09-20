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

SoundTask::SoundTask() {
}

void SoundTask::playMelody(int melody[], int tempo[], int length) {
    for (int index = 0; index < length; index++) {
        // calculate note duration
        int noteDuration = 1000 / tempo[index];
        // play tone
        tone(PIEZO, melody[index], noteDuration);
        // to distinguish the notes pause between
        int pauseBetweenNotes = noteDuration * 1.30;
        vTaskDelay(pauseBetweenNotes);
        // stop tone
        digitalWrite(PIEZO,LOW);
    }
}

String SoundTask::getName() const {
    return "SoundTask";
}

void SoundTask::task() {
    while(true) {
        // ToDo: Do something here that makes sound when inactivity occurs
        // ToDo: Add some logic for sound, mario, megaman, ...
        vTaskDelay((1000/portTICK_PERIOD_MS));
    }
}
