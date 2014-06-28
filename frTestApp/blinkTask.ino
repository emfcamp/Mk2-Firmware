#define BLINK_TASK_PIN_LED_OUT 13 //  is the one on the board

void vBlinkTask(void *pvParameters) {
    pinMode(BLINK_TASK_PIN_LED_OUT, OUTPUT);

    vSafePrint("Blink Task start");
    // int to hold led state
    uint8_t state = 0;
    while(1) {
        digitalWrite(BLINK_TASK_PIN_LED_OUT, state);
        state = !state;

        vTaskDelay((500/portTICK_PERIOD_MS));
    }
}

void vBlinkTaskCreate() {
    BaseType_t task = xTaskCreate(vBlinkTask,
                     "vBlinkTask",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     1,
                     NULL);
    if (task != pdPASS) {
        Serial.println("Failed to create vBlinkTask");
        while(1);
    }
}