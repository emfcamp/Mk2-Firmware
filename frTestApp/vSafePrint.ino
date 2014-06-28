void vSafePrint(String text) {
    // Don't wait forever - That could lead to deadlocks
    if (xSemaphoreTake(xSerialMutex, 10) == pdTRUE) {
        Serial.println(text);
        xSemaphoreGive(xSerialMutex);
    }
}