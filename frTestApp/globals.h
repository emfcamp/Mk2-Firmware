extern SemaphoreHandle_t xSerialMutex = NULL;

extern QueueHandle_t xSerialInQueue = NULL;

#define INCOMING_SERIAL_MESSAGE_TYPE_NORMAL 1;

struct incomingSerialMessage_t
{
    uint8_t type;
    String text;
};