#include <DueFlashStorage.h>
#include <Arduino.h>

uint32_t id[4];
bool success;

void setup() {
    success = (flash_init(FLASH_ACCESS_MODE_128, 4) == FLASH_RC_OK) &&
              (flash_read_unique_id(id, 4) == FLASH_RC_OK);

    SerialUSB.begin(115200);
    delay(250);
}


void loop() {
    if (success) {
        SerialUSB.print("Unique ID: ");
        SerialUSB.print(String(id[0]) + " ");
        SerialUSB.print(String(id[1]) + " ");
        SerialUSB.print(String(id[2]) + " ");
        SerialUSB.println(String(id[3]));
    } else {
        SerialUSB.println("Error has occured");
    }

    delay(1000);
}