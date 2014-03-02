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


// timer handle for use by pin interrrupt
xTimerHandle xLightFadeTimer;
xQueueHandle xRGBRequestQueue;

// time to fade lights after in ms
#define LIGHT_FADE_AFTER   3000     // how long should the LED's be on for
#define LIGHT_FADE_STEP    5        // how much to decune the PWM by each time (starts at 255)
#define LIGHT_FADE_DELAY   10       // how much to delay at each level


// (RGB, LED, TYPE, TIME, Period, PRIOITY)
enum RGBLeds {
    LED1,
    LED2,
    BOTH
};

enum RGBType {
    STATIC,
    FLASH,
    TORCH,
    OFF
};

struct RGBRequest_t {
    uint8_t rgb[3];
    RGBLeds led;
    RGBType type;
    uint32_t time;
    uint32_t period;
    uint8_t prioity;
    void* timer;
};

void tildaButtonInterruptsPriority() {
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
    
    // PMIC to CHARGE
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);
    
    // set button pins and attach interrupts
    // dose not matter if we do not define all button interupt handlers
    tildaButtonSetup();
    tildaButtonInterruptPriority();
    tildaButtonAttachInterrupts();
    
    pinMode(SRF_SLEEP, OUTPUT);
    digtialWrite(SRF_SLEEP, LOW);
    
    // setup  Serial
    Serial.begin(115200);
    delay(250);
    Serial.println("TiLDA Mk2 RGB task tester tester");
    
    // setup RGB task
    t1 = xTaskCreate(vBlinkTask,
                    NULL,
                    configMINIMAL_STACK_SIZE,
                    NULL,
                    1,
                    NULL);
    
    if (t1 != pdPASS) {
        // tasked didn't get created
        while(1);
    }
    // start scheduler
    vTaskStartScheduler();
    Serial.println("Insufficient RAM");
    while(1);
}

// in FreeRTOS loop is use as the vApplicationIdleHook
void loop() {
}

/******************************************************************************/



/* 
    RGB task
 */
void vRGBTask(void *pvParameters) {
    // LED's off by defualt
    analogWrite(LED1_RED, 0);
    analogWrite(LED1_GREEN, 0);
    analogWrite(LED1_BLUE, 0);
    analogWrite(LED2_RED, 0);
    analogWrite(LED2_GREEN, 0);
    analogWrite(LED2_BLUE, 0);
    
    // create RGB request queue
    xRGBRequestQueue = xQueueCreate(3, sizeof(RGBRequest_t));
    xQueueHandle xRGBPendQueue = xQueueCreate(3, sizeof(RGBRequest_t));
    portBASE_TYPE xStatus;
    
    static RGBRequest_t newRequest;
    static RGBRequest_t currentRequest;
    RGBRequest_t tempRequest;
    
    currentRequest.prioity = 255;
    currentRequest.type = OFF;
    
    /*
    // create Ligth Timmer
    xLightFadeTimer = xTimerCreate(NULL,
                                    (LIGHT_FADE_AFTER / portTICK_RATE_MS),
                                    pdFALSE,
                                    NULL,
                                    vLightFadeCallback);
    */
    
    for(;;) {
        /* block on queue forever */
        xStatus = xQueueReceive( xRGBRequestQueue, &newRequest, portMAX_DELAY);
        
        if (xStatus == pdPASS) {
            // we have a new RGB Request to process
            // TODO: lots of processing of RGBRequest_t
            
            if (newRequest.prioity > currentRequest.prioity) {
                // move current to previous if we are interrupting
                if (uxQueueMessageWaiting(xRGBPendQueue) == 0) {
                    xQueueSendToBack(xRGBPendQueue, currentRequest, portMAX_DELAY);
                } else {
                    // peek to deiced if adding to from or back
                    xQueuePeek(xRGBPendQueue, tempRequest, portMAX_DELAY);
                    if (currentRequest.prioity > tempRequest.prioity) {
                        if (uxQueueSpacesAvailable(xRGBPendQueue)) {
                            // add to front of queue if current is higher prioity that front of pend queue
                            xQueueSendToFront(xRGBPendQueue, currentRequest, portMAX_DELAY);
                        } else {
                            // TODO: make room in que
                        }
                    } else {
                        if (uxQueueSpacesAvailable(xRGBPendQueue)) {
                            xQueueSendToBack(xRGBPendQueue, currentRequest, portMAX_DELAY);
                        } else {
                            // dont care
                        }
                    }
                    
                }
                // if check status in pend queue
                
                
                if (newRequest.type == TORCH) {
                    // TODO: check IMU state
#define IMU_UP false
                    if (IMU_UP) {
                        //reduce brightness
                        currentRequest.RGB = {128,128,128};
                        // TODO: IMU orientation HOOK
                    }
                    // set LED outputs
                    RGBSetOutput(&currentRequest);
                    // start timmer
                    
                    
                } else if (newRequest.type == STATIC) {
                    
                } else if (newRequest.type == FLASH) {
                    
                } else if (newRequest.Type == OFF){
                    
                }
            } else {
                // stuff it in pending
                pendingRequest = newRequest;
            }
        } else {
            // WE SHOULD NEVER GET HERE
        }
        
    }
    
}

/*
 * helper to set led's
 */
void RGBSetOutput(RGBRequest_t *request){
    if (request->led == LED1 || request->led == BOTH) {
        analogWrite(LED1_RED, request->RGB[0]);
        analogWrite(LED1_GREEN, request->RGB[1]);
        analogWrite(LED1_BLUE, request->RGB[2]);
    } else if (request->led == LED2 || request->led == BOTH) {
        analogWrite(LED2_RED, request->RGB[0]);
        analogWrite(LED2_GREEN, request->RGB[1]);
        analogWrite(LED2_BLUE, request->RGB[2]);
    }
}

/*
 * A BUTTON_LIGHT press will call the ISR and turn on the LED's to white
 * this will then start the timer to turn them back off after LIGHT_FADE_AFTER
 * Another button press befor the LIGHT_FADE_AFTER time has expired or during the LightFadeCallback
 * will set the RGB's back to White and reset the time out period
 */
void buttonLightPress(){
    // called when Light button is pressed
    // put LIGHT onto RGB que and check for wake task
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    RGBRequest_t light;
    
    light->RGB = {255,255,255};
    light->led = BOTH;
    light->type = TORCH;
    light->time = LIGHT_FADE_AFTER;
    light->prioity = 0;

    xQueueSendToFrontFromIRS(xRGBRequestQueue, light, &xHigherPriorityTaskWoken);

    /* If xHigherPriorityTaskWoken equals pdTRUE, then a context switch
     should be performed.  The syntax required to perform a context switch
     from inside an ISR varies from port to port, and from compiler to
     compiler.  Inspect the demos for the port you are using to find the
     actual syntax required. */
    if( xHigherPriorityTaskWoken != pdFALSE )
    {
        /* Call the interrupt safe yield function here (actual function
         depends on the FreeRTOS port being used). */
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken)
    }
}




void vLightOnCallback() {
    // turn on LED's to White
    analogWrite(LED1_RED, 255);
    analogWrite(LED1_GREEN, 255);
    analogWrite(LED1_BLUE, 255);
    analogWrite(LED2_RED, 255);
    analogWrite(LED2_GREEN, 255);
    analogWrite(LED2_BLUE, 255);
    
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    // start the timer task to turn off the LED's
    xTimerStartFromISR(xLightFadeTimer, &xHigherPriorityTaskWoken);
    
    /* If xHigherPriorityTaskWoken equals pdTRUE, then a context switch
     should be performed.  The syntax required to perform a context switch
     from inside an ISR varies from port to port, and from compiler to
     compiler.  Inspect the demos for the port you are using to find the
     actual syntax required. */
    if( xHigherPriorityTaskWoken != pdFALSE )
    {
        /* Call the interrupt safe yield function here (actual function
         depends on the FreeRTOS port being used). */
    }
    
}


/* This should get run LIGHT_FADE_AFTER ms after a call to vLightOnCallback
 * evemtulay we will fade the RGB's to off, for testing we are using digitalWrite LOW
 */
void vLightFadeCallback(xTimerHandle xTimer) {
    // just turn off ligts for now, will work out fade later as not sure we want to do it in ISR
    
    for(int i=255; i>=0; i-=LIGHT_FADE_STEP){
        analogWrite(LED1_RED, i);
        analogWrite(LED1_GREEN, i);
        analogWrite(LED1_BLUE, i);
        analogWrite(LED2_RED, i);
        analogWrite(LED2_GREEN, i);
        analogWrite(LED2_BLUE, i);
        delay(LIGHT_FADE_DELAY);
    }
}


