#include <S25FLx.h>
#include <SPI.h>

#define FLASH_TEST_LENGTH 256 //length of array to write, read, and print
unsigned long t = 0;
unsigned long d = 0;
byte read_array[FLASH_TEST_LENGTH];
byte write_array[FLASH_TEST_LENGTH];

void dumpBuffer(uint8_t* buf, uint32_t size) {
    for (int i = 0; i < size; i += 0x10) {
        if (i < 0x10) SerialUSB.print(0);
        SerialUSB.print(i, HEX);
        SerialUSB.print(":");
        for (int j = 0; j < 0x10; j++) {
            SerialUSB.print(" ");
            if (buf[i + j] < 0x10) SerialUSB.print("0");
            SerialUSB.print(buf[i + j], HEX);
        }
        SerialUSB.println();
    }
}

void setup() {
    //PMIC
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);

    SerialUSB.begin(115200);
    while (!SerialUSB); // wait until serial monitor is open to begin.
    delay(500);
    SerialUSB.println("Flash Read Write test");

    // call new pin setup routine
    Flash.begin();

    if (!Flash.read_info()){  //will return an error if the chip isn't wired up correctly.
        SerialUSB.println("Unable to communicate with Flash");
        while (1); // die here
    }

}

void test_erase() {

    SerialUSB.println("Erasing");
    SerialUSB.println();
    t = micros();
    Flash.erase_4k(0);
    Flash.wait_write();
    d = micros() - t;

    SerialUSB.print("Erased in ");
    SerialUSB.print(d);
    SerialUSB.println(" microseconds");
    SerialUSB.println();

}

void test_read() {
    SerialUSB.println("Reading");
    SerialUSB.println();
    t = micros();
    Flash.read(0, read_array, FLASH_TEST_LENGTH);
    d = micros() - t;
    SerialUSB.print("Read in ");
    SerialUSB.print(d);
    SerialUSB.println(" microseconds");
    SerialUSB.println();

    dumpBuffer(read_array, FLASH_TEST_LENGTH);
    SerialUSB.flush();
    delay(100);

}

void test_write() {
    SerialUSB.println("fill Write array");
    for(int i=0; i < FLASH_TEST_LENGTH; i++) {
        write_array[i] = i;
    }
    SerialUSB.println("Writing");
    SerialUSB.println();
    t = micros();
    Flash.page_program(0, write_array, FLASH_TEST_LENGTH);
    Flash.wait_write();
    d = micros() - t;
    SerialUSB.print("write in ");
    SerialUSB.print(d);
    SerialUSB.println(" microseconds");
    SerialUSB.println();

}

void run_tests() {
    test_read();
    test_erase();
    test_read();
    test_write();
    test_read();
}

void loop() {
    run_tests();
    while(true); // end
}
