#define PIN_LED_IN_BUTTON_TASK 8
#define PIN_LED_OUT_BUTTON_TASK 10

SemaphoreHandle_t xButtonTaskSemaphore = NULL;

void vButtonTask(void *pvParameters) {
    #define LONG_TIME 0xffff

    vSafePrint("Button Task start");

    // Output
    pinMode(PIN_LED_OUT_BUTTON_TASK, OUTPUT);

    // Input
    pinMode(PIN_LED_IN_BUTTON_TASK, INPUT_PULLUP);
    setDebounce(PIN_LED_IN_BUTTON_TASK);
    attachInterrupt(PIN_LED_IN_BUTTON_TASK, vButtonTaskInterrupt, FALLING);

    // semaphore used for the
    xButtonTaskSemaphore = xSemaphoreCreateBinary();

    uint8_t state = 0;
    while(1) {
        digitalWrite(PIN_LED_OUT_BUTTON_TASK, state);
        if( xSemaphoreTake( xButtonTaskSemaphore, LONG_TIME ) == pdTRUE ) {
            state = !state;
        }
    }
}

void vButtonTaskInterrupt() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR( xButtonTaskSemaphore, &xHigherPriorityTaskWoken );
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void vButtonTaskCreate() {
    BaseType_t task = xTaskCreate(vButtonTask,
                     "vButtonTask",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     1,
                     NULL);
    if (task != pdPASS) {
        Serial.println("Failed to create vButtonTask");
        while(1);
    }
}