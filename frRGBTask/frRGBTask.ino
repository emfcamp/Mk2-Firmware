/*
 TiLDA Mk2 RGB main task
 
 Description of task
 Setup and manage out put to the RGB LED's
 
 use a queue to pass in requests for an led change
 some struct with the following
 (RGB, LED, TYPE, TIME, PRIOITY)
 
 block on queue for most of the time
 
 install interrupt handler for the LIGHT button which should pass something on the queue
 
 if LIGHT and IMU orientation is up show only half brigthness
 
 
 
 Original based on information from
 http://forum.arduino.cc/index.php?topic=156474.0
 
 The MIT License (MIT)
 
 Copyright (c) 2014 Electromagnetic  Field LTD
 
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

#include <Debounce.h>
#include <FreeRTOS_ARM.h>
#include "RGBTask.h"

// timer handle for use by pin interrrupt
TimerHandle_t xLightFadeTimer;
QueueHandle_t xRGBRequestQueue;     // use by others and ButtonLigthPress to request a change in led state
QueueHandle_t xRGBPendQueue;        // needs to be global so RGBTimmerCallback can access it
RGBRequest_t currentRequest;        // current running request, needs to be global so RGBTimmerCallback can access it

// time to fade lights after in ms
#define LIGHT_FADE_AFTER   3000     // how long should the LED's be on for
#define LIGHT_FADE_STEP    5        // how much to decune the PWM by each time (starts at 255)
#define LIGHT_FADE_DELAY   10       // how much to delay at each level



void tildaButtonInterruptPriority() {
    // reset pin interrupt handler IRQn priority levels to allow use of FreeRTOS API calls
    NVIC_DisableIRQ(PIOA_IRQn);
    NVIC_ClearPendingIRQ(PIOA_IRQn);
    NVIC_SetPriority(PIOA_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOA_IRQn);
    
    NVIC_DisableIRQ(PIOB_IRQn);
    NVIC_ClearPendingIRQ(PIOB_IRQn);
    NVIC_SetPriority(PIOB_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOB_IRQn);
    
    NVIC_DisableIRQ(PIOC_IRQn);
    NVIC_ClearPendingIRQ(PIOC_IRQn);
    NVIC_SetPriority(PIOC_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOC_IRQn);
    
    NVIC_DisableIRQ(PIOD_IRQn);
    NVIC_ClearPendingIRQ(PIOD_IRQn);
    NVIC_SetPriority(PIOD_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOD_IRQn);
}



void setup() {
    // holder for task handle
    BaseType_t t1,t2;
      
    // PMIC to CHARGE
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);
    
    pinMode(SRF_SLEEP, OUTPUT);
    digitalWrite(SRF_SLEEP, LOW);
    
    // setup  Serial
    Serial.begin(115200);
    delay(250);
    Serial.println("TiLDA Mk2 RGB task tester tester");

    // set button pins and attach interrupts
    // dose not matter if we do not define all button interupt handlers
    tildaButtonSetup();
    tildaButtonAttachInterrupts();
    tildaButtonInterruptPriority();

    Serial.println("Create RGB task");
    // setup RGB task
    t1 = xTaskCreate(vRGBTask,
                     NULL,
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     2,
                     NULL);
                    
    t2 = xTaskCreate(vBlinkTask,
                     NULL,
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     1,
                     NULL);
    
    if (t1 != pdPASS || t2 != pdPASS) {
        // tasked didn't get created
        Serial.println("Failed to create task");
        while(1);
    }
    // start scheduler
    Serial.println("Start Scheduler");
    vTaskStartScheduler();
    Serial.println("Insufficient RAM");
    while(1);
}

// in FreeRTOS loop is use as the vApplicationIdleHook
void loop() {
}

/******************************************************************************/


/*
 * helper to set led's
 */
void RGBSetOutput(RGBRequest_t *request) {
    if (request->type == OFF) {
        if (request->led == LED1 || request->led == BOTH) {
            analogWrite(LED1_RED, 0);
            analogWrite(LED1_GREEN, 0);
            analogWrite(LED1_BLUE, 0);
        }
        if (request->led == LED2 || request->led == BOTH) {
            analogWrite(LED2_RED, 0);
            analogWrite(LED2_GREEN, 0);
            analogWrite(LED2_BLUE, 0);
        }
    } else {
        if (request->led == LED1 || request->led == BOTH) {
            analogWrite(LED1_RED, request->rgb[0]);
            analogWrite(LED1_GREEN, request->rgb[1]);
            analogWrite(LED1_BLUE, request->rgb[2]);
        }
        if (request->led == LED2 || request->led == BOTH) {
            analogWrite(LED2_RED, request->rgb[0]);
            analogWrite(LED2_GREEN, request->rgb[1]);
            analogWrite(LED2_BLUE, request->rgb[2]);
        }
    }
}

/*
 * RGBTimerCallback
 * This is called at the end of the current running timmer
 * it's job is to switch the light to the next tast in the pending que 
 * or off if there is nothing waiting
 *
 */
void vRGBTimerCallback(TimerHandle_t pxTimer) {
    // current timer has expired
    Serial.println("Timer expired");
    if (xTimerDelete(pxTimer, (2/portTICK_PERIOD_MS)) != pdPASS) {
           // TODO: failed to remove timmer
    } else {
            currentRequest.timer = NULL;
    }
    if (uxQueueMessagesWaiting(xRGBPendQueue) == 0) {
        // there is nothing in the pending queue to replace it with so turn the LED's OFF
        currentRequest.type = OFF;
        RGBSetOutput(&currentRequest);
        
    } else {
        // load request from the front of the pending queue
        if (xQueueReceive(xRGBPendQueue, &currentRequest, (2/portTICK_PERIOD_MS))) {
            // got request out of the queue
            RGBProcessRequest();
        } else {
            // failed to get request from queue
        }
        
    }
    
}

/*
 * RGBFlashCallback
 * called every request.period to flash change an led flash state
 */
void vRGBFlashCallback(TimerHandle_t xTimer){

}

/*
 * RGBFadeCallback
 * handle the next step in the current fade
 * called every facation of request.period
 */
void vRGBFadeCallback(TimerHandle_t xTimer){

}

/*
 * RGBProcessRequest
 * setup request, setup leds, create(?) start timers as needed
 */
void RGBProcessRequest() {
    if (currentRequest.type == OFF){
        // not much to do wigt an off request 
        RGBSetOutput(&currentRequest);
        currentRequest.timer = NULL;
    } else {
        // its not an OFF so do we need a new timer
        if (currentRequest.timer == NULL) {
            // need a new timmer        
             currentRequest.timer = xTimerCreate(NULL, (currentRequest.time/portTICK_PERIOD_MS), pdFALSE, NULL, vRGBTimerCallback);
        }
           
        if (currentRequest.type == TORCH) {
            // TODO: check IMU state before turning on torch mode
            #define IMU_UP false
            if (IMU_UP) {
                //reduce brightness
                currentRequest.rgb = {128,128,128};
                // TODO: IMU orientation HOOK
            }
            // set LED outputs
            RGBSetOutput(&currentRequest);
            // start the timer
            if (xTimerStart(currentRequest.timer, (5/portTICK_PERIOD_MS)) != pdPASS) {
                // TODO: timer could not start
            }
            
            
        } else if (currentRequest.type == STATIC) {
            
        } else if (currentRequest.type == FLASH) {
            
        } else if (currentRequest.type == FADE) {
        
        }
    }
}

/*
 * RGB task
 */
void vRGBTask(void *pvParameters) {
    Serial.println("RGBTask Start");
    // LED's off by defualt
    analogWrite(LED1_RED, 0);
    analogWrite(LED1_GREEN, 0);
    analogWrite(LED1_BLUE, 0);
    analogWrite(LED2_RED, 0);
    analogWrite(LED2_GREEN, 0);
    analogWrite(LED2_BLUE, 0);
    // create RGB request queue
    xRGBRequestQueue = xQueueCreate(3, sizeof(RGBRequest_t ));
    xRGBPendQueue = xQueueCreate(3, sizeof(RGBRequest_t ));
    
    if (xRGBRequestQueue == 0) {
      Serial.println("Failed to create rgbQueue");
    } else if (xRGBPendQueue == 0) {
       Serial.println("Failed to create pendQueue");
    }
    
    BaseType_t xStatus;
    
    RGBRequest_t newRequest;
    RGBRequest_t tempRequest;
    
    currentRequest.prioity = 255;
    currentRequest.type = OFF;
    currentRequest.led = BOTH;
    currentRequest.timer = NULL;
    
    for(;;) {
        /* block on queue forever */
        Serial.println("RGBTask: Block on Queue");
        xStatus = xQueueReceive( xRGBRequestQueue, &newRequest, portMAX_DELAY);
        
        if (xStatus == pdPASS) {
            // we have a new RGB Request to process
            Serial.println("Got a newRequest");
            newRequest.timer = NULL;
            // if we are off we dont need to stash the currentRequest
            if (currentRequest.type !=OFF) {
                // TODO: should compare which LED's have been requested and which are in use
                if (newRequest.prioity < currentRequest.prioity) {
                    // TODO: stop the current request timer
                    if (xTimerStop(currentRequest.timer, (2/portTICK_PERIOD_MS)) != pdPASS) {
                        // TODO: failed to stop the timmer
                    }
                    // move current to previous if we are interrupting
                    if (uxQueueMessagesWaiting(xRGBPendQueue) == 0) {
                        xQueueSendToBack(xRGBPendQueue, &currentRequest, portMAX_DELAY);
                    } else {
                        // peek to deiced if adding to from or back
                        xQueuePeek(xRGBPendQueue, &tempRequest, portMAX_DELAY);
                        if (currentRequest.prioity < tempRequest.prioity) {
                            if (uxQueueSpacesAvailable(xRGBPendQueue)) {
                                // add to front of queue if current is higher prioity that front of pend queue
                                xQueueSendToFront(xRGBPendQueue, &currentRequest, portMAX_DELAY);
                            } else {
                                // TODO: make room at the back of the que and add currentRequest one to the front
                            }
                        } else {
                            if (uxQueueSpacesAvailable(xRGBPendQueue)) {
                                xQueueSendToBack(xRGBPendQueue, &currentRequest, portMAX_DELAY);
                            } else {
                                // dont care
                            }
                        }
                    }
                    
                    // start setting up the newRequest
                    currentRequest = newRequest;
                    RGBProcessRequest();
                } else {
                    // Put newRequest in pending queue
                    if (uxQueueMessagesWaiting(xRGBPendQueue) == 0) {
                        xQueueSendToBack(xRGBPendQueue, &newRequest, portMAX_DELAY);
                    } else {
                        // peek to deiced if adding to from or back
                        xQueuePeek(xRGBPendQueue, &tempRequest, portMAX_DELAY);
                        if (newRequest.prioity < tempRequest.prioity) {
                            // newRequest has a higher prioity than whats at the front of the que
                            // so place it in front
                            if (uxQueueSpacesAvailable(xRGBPendQueue)) {
                                // add to front of queue if current is higher prioity that front of pend queue
                                xQueueSendToFront(xRGBPendQueue, &newRequest, portMAX_DELAY);
                            } else {
                                // TODO: make room at the back of the que and add newRequest to the front
                            }
                        } else {
                            // newRequest has same or lower prioity as head of que
                            // so it can go to the back if we have room
                            if (uxQueueSpacesAvailable(xRGBPendQueue)) {
                                xQueueSendToBack(xRGBPendQueue, &newRequest, portMAX_DELAY);
                            } else {
                                // TODO: no room, so drop the new reuest or we could over write the oldest
                            }
                        }
                        
                    }

                }
            } else {
                // currentRequest is of type OFF
                // so just start setting up the newRequest
                currentRequest = newRequest;
                RGBProcessRequest();
            }
        } else {
            // WE SHOULD NEVER GET HERE
        }
        
    }
    
}
uint8_t firstFire = 1;

/*
 * A BUTTON_LIGHT press will call the ISR and turn on the LED's to white
 * this will then start the timer to turn them back off after LIGHT_FADE_AFTER
 * Another button press befor the LIGHT_FADE_AFTER time has expired or during the LightFadeCallback
 * will set the RGB's back to White and reset the time out period
 */
void buttonLightPress(){
    if (firstFire) {
      firstFire = 0;
      return;
    }
    // called when Light button is pressed
    // put LIGHT onto RGB que and check for wake task
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    RGBRequest_t light;
    
    light.rgb = {255,255,255};
    light.led = BOTH;
    light.type = TORCH;
    light.time = LIGHT_FADE_AFTER;
    light.prioity = 0;
    xQueueSendToFrontFromISR(xRGBRequestQueue, &light, &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/******************************************************************************/


void vBlinkTask(void *pvParameters) {
    Serial.println("Blink Task start");
    // int to hold led state
    uint8_t state = 0;
    // enabled pin 13 led
    pinMode(PIN_LED_TXL, OUTPUT);
    while(1) {
        digitalWrite(PIN_LED_TXL, state);
        state = !state;
        
        vTaskDelay((100/portTICK_PERIOD_MS));
    }
    
}

