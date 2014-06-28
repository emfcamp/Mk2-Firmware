void vSerialButtonUpdaterTask(void *pvParameters) {
    #define SERIAL_BUTTON_UPDATER_TASK_PIN_LED_OUT 3

    struct incomingSerialMessage_t *pxPayload;

    vSafePrint("Serial Button Updater Task start");

    // Output
    pinMode(SERIAL_BUTTON_UPDATER_TASK_PIN_LED_OUT, OUTPUT);

    // Check that queue exists
    if( xSerialInQueue == 0 ) {
        vSafePrint("xSerialInQueue is a dependency for vSerialButtonUpdaterTask, make sure it exists");
        while(1) ;
    }

    while(1) {
        if (xQueueReceive(xSerialInQueue, &pxPayload, portMAX_DELAY) == pdTRUE) {
            // ToDo: Set LED based on input
            vSafePrint("Received via task: " + pxPayload->text);
        }
    }
}

void vSerialButtonUpdaterTaskCreate() {
    BaseType_t task = xTaskCreate(vSerialButtonUpdaterTask,
                     "vSerialButtonUpdaterTask",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     1,
                     NULL);
    if (task != pdPASS) {
        Serial.println("Failed to create vSerialButtonUpdaterTask");
        while(1);
    }
}