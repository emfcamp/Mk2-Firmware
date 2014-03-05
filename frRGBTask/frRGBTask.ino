/*
 TiLDA Mk2 RGB main task
 
 Description of task
 Setup and manage out put to the RGB LED's
 
 use a queue to pass in requests for an led change
 some struct with the following
 (RGB, LED, TYPE, TIME, PRIORITY)
 
 block on queue for most of the time
 
 install interrupt handler for the LIGHT button which should pass something on the queue
 
 if LIGHT and IMU orientation is up show only half brightness
 
 
 
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

TimerHandle_t xRGBFlashTimer;
TimerHandle_t xRGBFadeTimer;
QueueHandle_t xRGBRequestQueue;     // use by others and ButtonLigthPress to request a change in led state
QueueHandle_t xRGBPendQueue;        // needs to be global so RGBtimerCallback can access it
RGBRequest_t currentRequest;        // current running request, needs to be global so RGBtimerCallback can access it
uint8_t RGB1[3];
uint8_t RGB2[3];
uint8_t flashState = 0;
// time to fade torch lights after in ms
#define LIGHT_FADE_AFTER   3000     // how long should the LED's be on for

/*
 * FadeState 
 * used to hold information during a FADE
 */
struct FadeState {
    uint16_t period;        // original request period
    int32_t rgb1Step[3];     // period match count for LED1
    int32_t rgb2Step[3];     // period match count for LED2
} RGBFadeState;


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
    // dose not matter if we do not define all button interrupt handlers
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
void RGBSetOutput(RGBRequest_t *request, uint8_t offOverride = 0) {
    if (request->type == OFF || offOverride == 1) {
        if (request->led == LED1 || request->led == BOTH) {
            analogWrite(LED1_RED, 0);
            analogWrite(LED1_GREEN, 0);
            analogWrite(LED1_BLUE, 0);
            RGB1 = {0, 0, 0};
        }
        if (request->led == LED2 || request->led == BOTH) {
            analogWrite(LED2_RED, 0);
            analogWrite(LED2_GREEN, 0);
            analogWrite(LED2_BLUE, 0);
            RGB2 = {0, 0, 0};
        }
    } else {
        if (request->led == LED1 || request->led == BOTH) {
            analogWrite(LED1_RED, request->rgb[0]);
            analogWrite(LED1_GREEN, request->rgb[1]);
            analogWrite(LED1_BLUE, request->rgb[2]);
            RGB1 = {request->rgb[0], request->rgb[1], request->rgb[2]};
        }
        if (request->led == LED2 || request->led == BOTH) {
            analogWrite(LED2_RED, request->rgb[0]);
            analogWrite(LED2_GREEN, request->rgb[1]);
            analogWrite(LED2_BLUE, request->rgb[2]);
            RGB2 = {request->rgb[0], request->rgb[1], request->rgb[2]};
        }
    }
}

/*
 * RGBTimerCallback
 * This is called at the end of the current running timer
 * it's job is to switch the light to the next task in the pending queue
 * or off if there is nothing waiting
 *
 */
void vRGBTimerCallback(TimerHandle_t pxTimer) {
    // current timer has expired
    Serial.println("Timer expired");
    if (xTimerDelete(pxTimer, (2/portTICK_PERIOD_MS)) != pdPASS) {
           // TODO: failed to remove timer
    } else {
            currentRequest.timer = NULL;
    }
    // Stop the flash timer if needed
    if (currentRequest.type == FLASH || currentRequest.type == FLASH_ALT) {
        if (xTimerStop(xRGBFlashTimer, (1/portTICK_PERIOD_MS)) != pdPASS) {
            // TODO: failed to stop flash timer
        }
    }
    
    if (uxQueueMessagesWaiting(xRGBPendQueue) == 0) {
        // there is nothing in the pending queue to replace it with so turn the LED's OFF
        currentRequest.type = OFF;
        RGBSetOutput(&currentRequest);
    } else {
        // TODO: if current is TORCH and poped request is for only one LED turn the other one off
        
        // load request from the front of the pending queue
        if (xQueueReceive(xRGBPendQueue, &currentRequest, (2/portTICK_PERIOD_MS))) {
            // got request out of the queue
            // if request is a FADE we need to restore a bit more context
            if (currentRequest.type == FADE && currentRequest.timer != NULL) {
                // restore FADE context
                if (currentRequest.led == LED1 || currentRequest.led == BOTH) {
                    RGB1[0] = currentRequest.stateRGB1[0];
                    RGB1[1] = currentRequest.stateRGB1[1];
                    RGB1[2] = currentRequest.stateRGB1[2];
                }
                if (currentRequest.led == LED2 || currentRequest.led == BOTH) {
                    RGB2[0] = currentRequest.stateRGB2[0];
                    RGB2[1] = currentRequest.stateRGB2[1];
                    RGB2[2] = currentRequest.stateRGB2[2];
                }
            }
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
    // switch leds between off and currentrequest.rgb
    if (currentRequest.type == FLASH) {
       if (flashState) {
            // need to turn the leds off 
            RGBSetOutput(&currentRequest, 1);
            flashState = 0;
        } else {
            RGBSetOutput(&currentRequest);
            flashState = 1;
        }
     
        // restart the flash timer
        if (xTimerStart(xTimer, (2/portTICK_PERIOD_MS)) != pdPASS) {
            // TODO: failed to restart flash timer
        }
    } else if (currentRequest.type == FLASH_ALT) {
        // need to handle Flashing alternate leds a little differently
        if (flashState) {
            // need to switch LED 2 off LED1 on
            currentRequest.led = LED2;
            RGBSetOutput(&currentRequest, 1);
            currentRequest.led = LED1;
            RGBSetOutput(&currentRequest);
            flashState = 0;
        } else {
            // need to switch LED1 off LED2 on
            currentRequest.led = LED1;
            RGBSetOutput(&currentRequest, 1);
            currentRequest.led = LED2;
            RGBSetOutput(&currentRequest);
            flashState = 1;
        }
        
        // restart the flash timer
        if (xTimerStart(xTimer, (2/portTICK_PERIOD_MS)) != pdPASS) {
            // TODO: failed to restart flash timer
        }
    } else {
        // this is not a flash we should not have got here
        
    }
}

/*
 * RGBFadeCallback
 * handle the next step in the current fade
 * called every fraction of request.period
 */
void vRGBFadeCallback(TimerHandle_t xTimer){
    if (currentRequest.period == 1) {
        // last pass
        currentRequest.period = 0;
        currentRequest.type = STATIC;
        
    }
    uint32_t i = RGBFadeState.period - currentRequest.period;
    // match period and step count then change led
    if (currentRequest.led == LED1 || currentRequest.led == BOTH) {
        RGB1[0] = RGBCalculateFadeVal(RGBFadeState.rgb1Step[0], RGB1[0], i);
        RGB1[1] = RGBCalculateFadeVal(RGBFadeState.rgb1Step[1], RGB1[1], i);
        RGB1[2] = RGBCalculateFadeVal(RGBFadeState.rgb1Step[2], RGB1[2], i);
        
        analogWrite(LED1_RED, RGB1[0]);
        analogWrite(LED1_GREEN, RGB1[1]);
        analogWrite(LED1_BLUE, RGB1[2]);
    }
    if (currentRequest.led == LED2 || currentRequest.led == BOTH) {
        RGB2[0] = RGBCalculateFadeVal(RGBFadeState.rgb2Step[0], RGB2[0], i);
        RGB2[1] = RGBCalculateFadeVal(RGBFadeState.rgb2Step[1], RGB2[1], i);
        RGB2[2] = RGBCalculateFadeVal(RGBFadeState.rgb2Step[2], RGB2[2], i);
        
        analogWrite(LED2_RED, RGB2[0]);
        analogWrite(LED2_GREEN, RGB2[1]);
        analogWrite(LED2_BLUE, RGB2[2]);
    }
    
    // reduce the currentRequest.period by 1
    if (currentRequest.period != 0) {
        currentRequest.period -= 1;
        // and restart the timers
        if (xTimerStart(xTimer, (2/portTICK_PERIOD_MS)) != pdPASS) {
            // TODO: failed to restart fade timer
        }
    }
}

/*
 * RGBCalculateFadeStep
 * used in setting up fade's
 *
 * Borrowed and modified from a Arduino color fading sketch originally by
 * Clay Shirky <clay.shirky@nyu.edu>
 * source unknown
 */
int32_t RGBCalculateFadeStep(int32_t prevValue, int32_t endValue) {
    int32_t step = endValue - prevValue; // What's the overall gap?
    if (step) {                      // If its non-zero,
        step = (int16_t)currentRequest.period/step;              //   divide by current period
    }
    return step;
}

/*
 * RGBCalculateFadeVal
 * used in fade's
 *
 * Borrowed and modified from a Arduino color fading sketch originally by
 * Clay Shirky <clay.shirky@nyu.edu>
 * source unknown
 */
uint8_t RGBCalculateFadeVal(int32_t step, int val, int32_t i) {
    if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
        if (step > 0) {              //   increment the value if step is positive...
            val += 1;
        }
        else if (step < 0) {         //   ...or decrement it if step is negative
            val -= 1;
        }
    }
    // Defensive driving: make sure val stays in the range 0-255
    if (val > 255) {
        val = 255;
    }
    else if (val < 0) {
        val = 0;
    }
    return val;
}

/*
 * RGBProcessRequest
 * setup request, setup leds, create(?) start timers as needed
 * request could be either new or a continuation
 */
void RGBProcessRequest() {
    if (currentRequest.type == OFF){
        // not much to do with an off request
        RGBSetOutput(&currentRequest);
        currentRequest.timer = NULL;
    } else {
        // its not an OFF so do we need a new timer
        if (currentRequest.timer == NULL) {
            // need a new timer
             currentRequest.timer = xTimerCreate(NULL,
                                                 (currentRequest.time/portTICK_PERIOD_MS),
                                                 pdFALSE,
                                                 NULL,
                                                 vRGBTimerCallback);
        }
           
        if (currentRequest.type == TORCH) {
            // for touch we check we are not to bright if orientation is in users face
            // TODO: check IMU state before turning on torch mode
            #define IMU_UP false
            if (IMU_UP) {
                //reduce brightness
                currentRequest.rgb = {128,128,128};
                // TODO: IMU orientation hook, need to add something to set brightness to full if orientation is returned to normal
            }
            
            // start the timer
            if (xTimerStart(currentRequest.timer, (5/portTICK_PERIOD_MS)) != pdPASS) {
                // TODO: timer could not start
            }
            // set LED outputs
            RGBSetOutput(&currentRequest);

        } else if (currentRequest.type == STATIC) {
            // Static is very similar to torch except we don't alter brightness, assume the requesting app has chosen appropriate RGB values
            // start the timer
            if (xTimerStart(currentRequest.timer, (5/portTICK_PERIOD_MS)) != pdPASS) {
                // TODO: timer could not start
            }
            // set LED outputs
            RGBSetOutput(&currentRequest);
            
        } else if (currentRequest.type == FLASH) {
            // for flash we are switching between on and off states at request.peroid
            // so use the xFlashtimer
            // reconfigure the flash period
            if (xTimerChangePeriod(xRGBFlashTimer, (currentRequest.period/portTICK_PERIOD_MS), (2/portTICK_PERIOD_MS)) != pdPASS) {
                // TODO: failed to change flash period
            }  
            // start main timer
            if (xTimerStart(currentRequest.timer, (5/portTICK_PERIOD_MS)) != pdPASS) {
                // TODO: timer could not start
            }
            // start flash timer
            if (xTimerStart(xRGBFlashTimer, (2/portTICK_PERIOD_MS)) != pdPASS) {
                // TODO: flash timer could not start
            }
            // set led output for first flash
            RGBSetOutput(&currentRequest);
            flashState = 1;
        } else if (currentRequest.type == FLASH_ALT) {
            // like flash we switching between on and off but also with alternate led's
            // this assumes led = BOTH
            // reconfigure the flash period
            if (xTimerChangePeriod(xRGBFlashTimer, (currentRequest.period/portTICK_PERIOD_MS), (2/portTICK_PERIOD_MS)) != pdPASS) {
                // TODO: failed to change flash period
            }            
            // start main timer
            if (xTimerStart(currentRequest.timer, (5/portTICK_PERIOD_MS)) != pdPASS) {
                // TODO: timer could not start
            }
            // start flash timer
            if (xTimerStart(xRGBFlashTimer, (2/portTICK_PERIOD_MS)) != pdPASS) {
                // TODO: flash timer could not start
            }
            // set led output for first flash
            // LED2 off LED1 on
            currentRequest.led = LED2;
            RGBSetOutput(&currentRequest, 1);
            currentRequest.led = LED1;
            RGBSetOutput(&currentRequest);
            flashState = 0;
        } else if (currentRequest.type == FADE) {
            // this is the trick one we want to fade between the current state and the request.rgb
            // over the period
            // need to do some maths to work out steps for each color against a tick time for the fade
            // or do it in the fade timer callback
            
            // store the original period so we can use to for later calculations when matching step
            RGBFadeState.period = currentRequest.period;
            Serial.print("RGB: ");
            Serial.print(RGB2[0], DEC);
            Serial.print(" / ");
            Serial.print(RGB2[1], DEC);
            Serial.print(" / ");  
            Serial.println(RGB2[2], DEC); 
            // calculate step each LED as needed
            if (currentRequest.led == LED1 || currentRequest.led == BOTH) {
                // steps first
                RGBFadeState.rgb1Step[0] = RGBCalculateFadeStep(RGB1[0], currentRequest.rgb[0]);
                RGBFadeState.rgb1Step[1] = RGBCalculateFadeStep(RGB1[1], currentRequest.rgb[1]);
                RGBFadeState.rgb1Step[2] = RGBCalculateFadeStep(RGB1[2], currentRequest.rgb[2]);
            }
            if (currentRequest.led == LED2 || currentRequest.led == BOTH) {
                RGBFadeState.rgb2Step[0] = RGBCalculateFadeStep(RGB2[0], currentRequest.rgb[0]);
                RGBFadeState.rgb2Step[1] = RGBCalculateFadeStep(RGB2[1], currentRequest.rgb[1]);
                RGBFadeState.rgb2Step[2] = RGBCalculateFadeStep(RGB2[2], currentRequest.rgb[2]);
            }

            // start main timer
            if (xTimerStart(currentRequest.timer, (5/portTICK_PERIOD_MS)) != pdPASS) {
                // TODO: timer could not start
            }
            // start fade timer
            if (xTimerStart(xRGBFadeTimer, (2/portTICK_PERIOD_MS)) != pdPASS) {
                // TODO: fade timer could not start
            }
            // set led output for first flash
            //RGBSetOutput(&currentRequest);
        }
    }
}

/*
 * RGB task
 */
void vRGBTask(void *pvParameters) {
    Serial.println("RGBTask Start");
    // create RGB request queue
    xRGBRequestQueue = xQueueCreate(3, sizeof(RGBRequest_t ));
    xRGBPendQueue = xQueueCreate(3, sizeof(RGBRequest_t ));
    
    if (xRGBRequestQueue == 0) {
      Serial.println("Failed to create rgbQueue");
    } else if (xRGBPendQueue == 0) {
       Serial.println("Failed to create pendQueue");
    }
    
    BaseType_t xStatus;
    
    xRGBFlashTimer = xTimerCreate(NULL,
                                                (1/portTICK_PERIOD_MS),
                                                pdFALSE,
                                                NULL,
                                                vRGBFlashCallback);
    xRGBFadeTimer = xTimerCreate(NULL,
                                                (1/portTICK_PERIOD_MS),
                                                pdFALSE,
                                                NULL,
                                                vRGBFadeCallback);
    
    RGBRequest_t newRequest;
    RGBRequest_t tempRequest;
    
    currentRequest.prioity = 255;
    currentRequest.type = OFF;
    currentRequest.led = BOTH;
    currentRequest.timer = NULL;
    RGBSetOutput(&currentRequest);
    
    for(;;) {
        /* block on queue forever */
        Serial.println("RGBTask: Block on Queue");
        xStatus = xQueueReceive( xRGBRequestQueue, &newRequest, portMAX_DELAY);
        
        if (xStatus == pdPASS) {
            // we have a new RGB Request to process
            Serial.println("Got a newRequest");
            newRequest.timer = NULL;
            // if we are off we don't need to stash the currentRequest
            if (currentRequest.type !=OFF) {
                // TODO: should compare which LED's have been requested and which are in use
                if (newRequest.prioity < currentRequest.prioity) {
                    // stop the current request timer
                    if (xTimerStop(currentRequest.timer, (2/portTICK_PERIOD_MS)) != pdPASS) {
                        // TODO: failed to stop the timer
                    }
                    // stop the flash or fade timer if needed
                    if (currentRequest.type == FLASH || currentRequest.type == FLASH_ALT) {
                        if (xTimerStop(xRGBFlashTimer, (1/portTICK_PERIOD_MS)) != pdPASS) {
                            // TODO: failed to stop flash timer
                        }
                    } else if (currentRequest.type == FADE) {
                        if (xTimerStop(xRGBFadeTimer, (1/portTICK_PERIOD_MS)) != pdPASS) {
                            // TODO: failed to stop fade timer
                        }
                        // for a FADE task we also need to store some more context
                        if (currentRequest.led == LED1 || currentRequest.led == BOTH) {
                            currentRequest.stateRGB1[0] = RGB1[0];
                            currentRequest.stateRGB1[1] = RGB1[1];
                            currentRequest.stateRGB1[2] = RGB1[2];
                        }
                        if (currentRequest.led == LED2 || currentRequest.led == BOTH) {
                            currentRequest.stateRGB2[0] = RGB2[0];
                            currentRequest.stateRGB2[1] = RGB2[1];
                            currentRequest.stateRGB2[2] = RGB2[2];
                        }
                    }
                    // move current to pendingQueue since we are interrupting
                    if (uxQueueMessagesWaiting(xRGBPendQueue) == 0) {
                        xQueueSendToBack(xRGBPendQueue, &currentRequest, portMAX_DELAY);
                    } else {
                        // peek to deiced if adding to from or back
                        xQueuePeek(xRGBPendQueue, &tempRequest, portMAX_DELAY);
                        if (currentRequest.prioity < tempRequest.prioity) {
                            if (uxQueueSpacesAvailable(xRGBPendQueue)) {
                                // add to front of queue if current is higher priority that front of pend queue
                                xQueueSendToFront(xRGBPendQueue, &currentRequest, portMAX_DELAY);
                            } else {
                                // TODO: make room at the back of the queue and add currentRequest one to the front
                            }
                        } else {
                            if (uxQueueSpacesAvailable(xRGBPendQueue)) {
                                xQueueSendToBack(xRGBPendQueue, &currentRequest, portMAX_DELAY);
                            } else {
                                // don't care
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
                            // newRequest has a higher priority than whats at the front of the queue
                            // so place it in front
                            if (uxQueueSpacesAvailable(xRGBPendQueue)) {
                                // add to front of queue if current is higher priority that front of pend queue
                                xQueueSendToFront(xRGBPendQueue, &newRequest, portMAX_DELAY);
                            } else {
                                // TODO: make room at the back of the queue and add newRequest to the front
                            }
                        } else {
                            // newRequest has same or lower priority as head of queue
                            // so it can go to the back if we have room
                            if (uxQueueSpacesAvailable(xRGBPendQueue)) {
                                xQueueSendToBack(xRGBPendQueue, &newRequest, portMAX_DELAY);
                            } else {
                                // TODO: no room, so drop the new request or we could over write the oldest
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
uint8_t firstLightFire = 1;

/*
 * A BUTTON_LIGHT press will call the ISR and turn on the LED's to white
 * this will then start the timer to turn them back off after LIGHT_FADE_AFTER
 * Another button press before the LIGHT_FADE_AFTER time has expired or during the LightFadeCallback
 * will set the RGB's back to White and reset the time out period
 */
void buttonLightPress(){
    if (firstLightFire) {
      firstLightFire = 0;
      return;
    }
    // called when Light button is pressed
    // put LIGHT onto RGB queue and check for wake task
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
uint8_t firstRightFire = 1;
uint8_t firstLeftFire = 1;
uint8_t firstUpFire = 1;
uint8_t firstDownFire = 1;
/*
 * Tester button interrupts these do the same a lightPress but give different test led use cases
 
 */
void buttonRightPress(){
    if (firstRightFire) {
        firstRightFire = 0;
        return;
    }
    // called when Light button is pressed
    // put LIGHT onto RGB queue and check for wake task
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    RGBRequest_t light;
    
    light.rgb = {0,1,0};
    light.led = BOTH;
    light.type = FLASH_ALT;
    light.period = 100;
    light.time = LIGHT_FADE_AFTER;
    light.prioity = 1;
    xQueueSendToFrontFromISR(xRGBRequestQueue, &light, &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void buttonLeftPress(){
    if (firstLeftFire) {
        firstLeftFire = 0;
        return;
    }
    // called when Light button is pressed
    // put LIGHT onto RGB queue and check for wake task
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    RGBRequest_t light;
    
    light.rgb = {2,0,0};
    light.led = BOTH;
    light.type = FLASH;
    light.period = 200;
    light.time = LIGHT_FADE_AFTER;
    light.prioity = 2;
    xQueueSendToFrontFromISR(xRGBRequestQueue, &light, &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


void buttonUpPress(){
    if (firstUpFire) {
        firstUpFire = 0;
        return;
    }
    // called when Light button is pressed
    // put LIGHT onto RGB queue and check for wake task
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    RGBRequest_t light;
    
    light.rgb = {50,0,2};
    light.led = LED2;
    light.type = STATIC;
    light.time = LIGHT_FADE_AFTER;
    light.prioity = 1;
    xQueueSendToFrontFromISR(xRGBRequestQueue, &light, &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void buttonDownPress(){
    if (firstDownFire) {
        firstDownFire = 0;
        return;
    }
    // called when Light button is pressed
    // put LIGHT onto RGB queue and check for wake task
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    RGBRequest_t light;
    
    light.rgb = {0,8,6};
    light.led = LED2;
    light.type = FADE;
    light.period = 1000;
    light.time = 3000;
    light.prioity = 1;
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

