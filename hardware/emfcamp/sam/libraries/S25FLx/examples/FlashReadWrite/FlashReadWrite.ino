#include <S25FLx.h>
#include <SPI.h>

#define FLASH_TEST_ADDRESS 4096
#define FLASH_TEST_LENGTH 256 //length of array to write, read, and print
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

    Flash.begin();
    SPI.setDLYBCT(FLASH_CS, 0);
    Flash.setCallback(dma_callback);

    if (!Flash.read_info()){  // will return false if the chip isn't wired up correctly.
        SerialUSB.println("Unable to communicate with Flash");
        while (1); // die here
    }

}

bool dma_complete = false;
void dma_callback() {
    dma_complete = true;
}
void wait_dma() {
    while (!dma_complete) {
        delay(1);
    }
    dma_complete = false;
    // Workaround for DMA bug - we seem not to release CS in time
    Flash.write_disable();
}

void test_erase() {

    SerialUSB.println("Erasing");
    SerialUSB.println();
    int before = micros();
    Flash.erase_4k(0);
    Flash.wait_write();
    int after = micros();

    SerialUSB.print("Erased in ");
    SerialUSB.print(after - before);
    SerialUSB.println("us");
    SerialUSB.println();

}

void print_status_twice() {
    byte stat1 = Flash.stat();
    byte stat2 = Flash.stat();
    SerialUSB.print("Status: 0x");
    SerialUSB.print(stat1, HEX);
    SerialUSB.print(", 0x");
    SerialUSB.print(stat2, HEX);
    SerialUSB.println();
}

#define NO_DMA

void test_read() {
    uint8_t* buf = Flash.allocBuffer(FLASH_TEST_LENGTH);
    Flash.clearBuffer();
    SerialUSB.println("Reading");
    SerialUSB.println();
    int before = micros();
    #ifdef NO_DMA
        Flash.fast_read(FLASH_TEST_ADDRESS, buf, FLASH_TEST_LENGTH);
    #else
        Flash.fast_read_dma(FLASH_TEST_ADDRESS, FLASH_TEST_LENGTH);
        wait_dma();
        //print_status_twice();
    #endif
    int after = micros();
    SerialUSB.print("Read in ");
    SerialUSB.print(after - before);
    SerialUSB.println("us");
    SerialUSB.println();

    dumpBuffer(buf, FLASH_TEST_LENGTH);
    SerialUSB.println();
    SerialUSB.flush();
    delay(200);

}

void test_write() {
    uint8_t* buf = Flash.allocBuffer(FLASH_TEST_LENGTH);
    SerialUSB.println("fill Write array");
    for(int i = 0; i < FLASH_TEST_LENGTH; i++) {
        buf[i] = 255 - i;
    }
    SerialUSB.println("Writing");
    SerialUSB.println();
    int before = micros();
    #ifdef NO_DMA
        Flash.page_program(FLASH_TEST_ADDRESS, buf, FLASH_TEST_LENGTH);
    #else
        Flash.wait_write();
        Flash.page_program_dma(FLASH_TEST_ADDRESS, FLASH_TEST_LENGTH);
        wait_dma();
    #endif
    Flash.wait_write();
    int after = micros();
    SerialUSB.print("Written in ");
    SerialUSB.print(after - before);
    SerialUSB.println("us");
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
