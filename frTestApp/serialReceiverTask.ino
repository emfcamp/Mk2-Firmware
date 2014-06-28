void vSerialReceiverTask(void *pvParameters) {
    #define QUEUE_LENGTH 10

    xSerialInQueue = xQueueCreate(QUEUE_LENGTH, sizeof( struct incomingSerialMessage_t * ) );
    if( xSerialInQueue == 0 )
    {
        vSafePrint("Queue for SerialReceiverTask wasn't created");
        while(1);
    }

    String bufferString = "";
    vSafePrint("Serial Receiver start");

    while(1) {
        while (Serial.available() > 0) {
            char inChar = (char)Serial.read();
            if (inChar != '\n') {
                bufferString += inChar;
            }
            if (inChar == '\n') {
                struct incomingSerialMessage_t xPayload;
                xPayload.type = INCOMING_SERIAL_MESSAGE_TYPE_NORMAL;
                xPayload.text = bufferString;

                struct incomingSerialMessage_t *pxPayload;
                pxPayload = &xPayload;

                if( xQueueSendToBack(xSerialInQueue, ( void * ) &pxPayload,( TickType_t ) 10 ) != pdPASS)  {
                    vSafePrint("Error trying to queue up serial message");
                }
                vSafePrint("Added to queue: " + bufferString);
                bufferString = "";
            }
        }

        vTaskDelay((100/portTICK_PERIOD_MS));
    }
}

void vSerialReceiverTaskCreate() {
    BaseType_t task = xTaskCreate(vSerialReceiverTask,
                     "vSerialReceiverTask",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     1,
                     NULL);
    if (task != pdPASS) {
        vSafePrint("Failed to create vSerialReceiverTask");
        while(1);
    }
}