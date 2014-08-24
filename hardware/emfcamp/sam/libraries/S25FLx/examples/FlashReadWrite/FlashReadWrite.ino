#include <S25FLx.h>
#include <SPI.h>
#define array_length 16 //length of array to write, read, and print
Flash flash(FLASH_CS, FLASH_HOLD);  //starts flash class and initilzes SPI

unsigned long t = 0;
unsigned long d = 0;
byte read_array[16];
byte write_array[16];

void setup() {
    //PMIC
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);
  Serial.begin(115200);
  while (!Serial) {
  } // wait until serial monitor is open to begin.
  delay(500);
  Serial.println("Flash Read Write test");

  // call new pin setup routine
  flash.begin();

  flash.waitforit(); // use between each communication to make sure S25FLxx is ready to go.
  if (!flash.read_info()){  //will return an error if the chip isn't wired up correctly.
      while (1) {
          // die here
      }
  }
  
  Serial.println("Erasing");
  Serial.println();
  t=micros(); 
  //flash.erase_all();
  flash.erase_4k(0);
  d=micros()-t;  

  Serial.println();
  Serial.print("Erased in ");
  Serial.print(d);
  Serial.println(" microseconds");
  
  Serial.println("Reading");
  flash.waitforit();
  t=micros();
  flash.read(0, read_array, 16);
  d=micros()-t;
  Serial.println();
  Serial.print("Read in ");
  Serial.print(d);
  Serial.println(" microseconds");
  Serial.println();
  
  for(int i=0; i < 16; i++) {
    Serial.print("Loc: 0x");
    Serial.print(i, HEX);
    Serial.print("  Val: 0x");
    Serial.println(read_array[i], HEX);
    Serial.flush();
    delay(100);
  }
  
    Serial.println("fill Write array");
  for(int i=0; i < 16; i++) {
    write_array[i] = i;
  }
  Serial.println("Writing");
  flash.waitforit();
  t=micros();
  flash.write(0, write_array, 16);
  d=micros()-t;
  Serial.println();
  Serial.print("write in ");
  Serial.print(d);
  Serial.println(" microseconds");
  Serial.println();
  
  Serial.println("Reading");
  flash.waitforit();
  t=micros();
  flash.read(0, read_array, 16);
  d=micros()-t;
  Serial.println();
  Serial.print("Read in ");
  Serial.print(d);
  Serial.println(" microseconds");
  Serial.println();
  
  for(int i=0; i < 16; i++) {
    Serial.print("Loc: 0x");
    Serial.print(i, HEX);
    Serial.print("  Val: 0x");
    Serial.println(read_array[i], HEX);
    Serial.flush();
    delay(100);
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
