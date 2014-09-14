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

#include "Task.h"

Task::Task()
    :mTaskHandle(NULL)
{}

Task::~Task() {
    vTaskDelete(mTaskHandle);
}

void Task::start() {
    if (mTaskHandle == NULL) {
        debug::log("Creating task: " + getName());
        
        // get task name
        String taskName = getName();

        // start the task
        if (xTaskCreate(_task, taskName.c_str(), ( uint8_t ) 255, static_cast<void*>(this), 2, &mTaskHandle) != pdPASS) {
            debug::stopWithMessage("Failed to create " + getName() + " task");
        }
    } else if (eTaskGetState(mTaskHandle) == eSuspended) {
        debug::log("Resume task: " + getName());
        beforeResume();
        vTaskResume(mTaskHandle);
    }
}

void Task::suspend() {
    if (mTaskHandle && eTaskGetState(mTaskHandle) != eSuspended) {
        debug::log("Suspend task: " + getName());
        vTaskSuspend(mTaskHandle);
        afterSuspension();
    } 
}

void Task::afterSuspension() {
    // do nothing
}

void Task::beforeResume() {
    // do nothing
}

// ToDo: Remove this function to save some stack 
void Task::taskCaller() {
    debug::log("Starting " + getName());
    task();

    while(true) {
        debug::log("Please make sure that no task ever returns. Task: " + getName());
        vTaskDelay((1000/portTICK_PERIOD_MS));
    }
}

void Task::_task(void* self) {
    static_cast<Task*>(self)->taskCaller();
}

