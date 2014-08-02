/*
 * This sketch is a simple Print benchmark.
 */
#include <SdFat.h>
#include <SdFatUtil.h>

// SD chip select pin
const uint8_t chipSelect = SS;

// number of lines to print
const uint16_t N_PRINT = 20000;

// file system
SdFat sd;

// test file
SdFile file;

// Serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    // wait for Leonardo
  }
cout << pstr("Use a freshly formatted SD for best results.\n");
}
//------------------------------------------------------------------------------
void loop() {
  char fname[] = "BENCH00.TXT";

  while (Serial.read() >= 0) {}

  // pstr stores strings in flash to save RAM
  cout << pstr("Type any character to start\n");
  while (Serial.read() <= 0) {}
  delay(400);  // catch Due reset problem

  cout << pstr("Free RAM: ") << FreeRam() << endl;

  // initialize the SD card at SPI_FULL_SPEED for best performance.
  // try SPI_HALF_SPEED if bus errors occur.
  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();

  cout << pstr("Type is FAT") << int(sd.vol()->fatType()) << endl;

  cout << pstr("\nThis test compares the standard print function\n");
  cout << pstr("with printField.  Each test prints 20,000 values.\n\n");

  // do write test
  for (int test = 0; test < 4; test++) {

    switch(test) {
    case 0:
      cout << pstr("Test of println(uint16_t)\n");
      break;

    case 1:
      cout << pstr("Test of printField(uint16_t, char)\n");
      break;

    case 2:
      cout << pstr("Test of println(double)\n");
      break;

    case 3:
      cout << pstr("Test of printField(double, char)\n");
    }
    fname[6] = test +'0';
    // open or create file - truncate existing file.
    if (!file.open(fname, O_CREAT | O_TRUNC | O_RDWR)) {
      error("open failed");
    }

    uint32_t t = millis();

    switch(test) {
    case 0:
      for (uint16_t i = 0; i < N_PRINT; i++) {
        file.println(i);
      }
      break;

    case 1:
      for (uint16_t i = 0; i < N_PRINT; i++) {
        file.printField(i, '\n');
      }
      break;

    case 2:
      for (uint16_t i = 0; i < N_PRINT; i++) {
        file.println((double)0.0001*i, 4);
      }
      break;

    case 3:
      for (uint16_t i = 0; i < N_PRINT; i++) {
        file.printField((double)0.0001*i, '\n', 4);
      }
      break;
    }
    if (file.writeError) {
      error("write failed");
    }
    file.close();
    t = millis() - t;
    double s = file.fileSize();
    cout << pstr("Time ") << 0.001*t << pstr(" sec\n");
    cout << pstr("File size ") << 0.001*s << pstr(" KB\n");
    cout << pstr("Write ") << s/t << pstr(" KB/sec\n\n");
  }
  cout << pstr("Done!\n\n");
}
