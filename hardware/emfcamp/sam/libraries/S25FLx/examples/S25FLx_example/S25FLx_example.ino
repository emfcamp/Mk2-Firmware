/*
 * Modified for TiLDA Mk2 by LWK
 */

#include <S25FLx.h>
#include <SPI.h>
#define array_length 16 //length of array to write, read, and print

unsigned long d, t, cycles, errors;
unsigned long location = 0; //starting memory location to read and write too.


byte random_bank[array_length] = {};

//start with data in it to see if it's returing 0's because it's empty or because the mremory it's reading is.
byte read_bank[array_length] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

/////////////////////////////////////////////////////////

void setup() {
  //PMIC
  pinMode(PMIC_ENOTG, OUTPUT);
  digitalWrite(PMIC_ENOTG, LOW);
  
  randomSeed(A2);
  SerialUSB.begin(115200);
  while (!SerialUSB) {
  } // wait until serial monitor is open to begin.
  delay(500);
  SerialUSB.println("Flash Read Write test");

  // call new pin setup routine
  Flash.begin();

  if (!Flash.read_info()) {  //will return an error if the chip isn't wired up correctly.
      while (1) {
          // die here
      }
  }
}

/////////////////////////////////////////////////////////


void loop() {
  
  ///////////////////////////////////////////////////////random up a bank
  //Random is used to make sure it is errasing and programming.
  for (int i=0; i < array_length; i++) {
    random_bank[i] = random(0, 255);

  }

  ///////////////////////////////////////////////////////erase

  SerialUSB.println("Erasing");
  SerialUSB.println();
  t = micros();
 // Flash.erase_all();
  Flash.erase_4k(location);
  Flash.wait_write();
  d = micros() - t;

  SerialUSB.println();
  SerialUSB.print("Erased in ");
  SerialUSB.print(d);
  SerialUSB.println(" microseconds");

  ////////////////////////////////////////////////////write
  t = micros();
  Flash.page_program(location, random_bank, array_length); //the middle variable is a pointer meaning you put an
  Flash.wait_write();
  d = micros() - t;

  SerialUSB.println();
  SerialUSB.print("Written in ");
  SerialUSB.print(d);
  SerialUSB.println(" microseconds");

  ////////////////////////////////////////////////////read


  t = micros();
  Flash.read(location, read_bank, array_length); //the middle variable is a pointer meaning you put an
  //arrays name there but not it's location "[x]" 
  d = micros() - t;

  SerialUSB.println();
  SerialUSB.print("Read in ");
  SerialUSB.print(d);
  SerialUSB.println(" microseconds");
  SerialUSB.println();

  ////////////////////////////////////////////////////check
  for (int i=0; i < array_length; i++) {
    if (random_bank[i] != read_bank[i]) {
      errors++;
    }
  }
  ////////////////////////////////////////////////////print
  /*
  for (int i=0; i < array_length; i++) {
    
    SerialUSB.print(i + loation);
    SerialUSB.print(" ");
    SerialUSB.print(random_bank[i]);    
    SerialUSB.print(" ");
    SerialUSB.print(read_bank[i]);    
    SerialUSB.print(" ");
    SerialUSB.println( );


  }
  */
  
  cycles++;

  SerialUSB.print(errors);   
  SerialUSB.print(" errors in ");   
  SerialUSB.print(cycles);   
  SerialUSB.println(" cycles");

  SerialUSB.println();
  SerialUSB.println("done");
  SerialUSB.println();

  delay(5000);

}




