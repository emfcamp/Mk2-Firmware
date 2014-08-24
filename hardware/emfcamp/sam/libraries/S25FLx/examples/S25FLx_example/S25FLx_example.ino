/*
 * Modified for TiLDA Mk2 by LWK
 */

#include <S25FLx.h>
#include <SPI.h>
#define array_length 16 //length of array to write, read, and print

Flash flash(FLASH_CS, FLASH_HOLD);  //starts flash class and initilzes SPI

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
  Serial.begin(115200);
  while (!Serial) {
  } // wait until serial monitor is open to begin.
  delay(500);
  Serial.println("Flash Read Write test");

  // call new pin setup routine
  flash.begin();

  flash.waitforit(); // use between each communication to make sure S25FLxx is ready to go.
  if (!flash.read_info()) {  //will return an error if the chip isn't wired up correctly.
      while (1) {
          // die here
      }
  }
}

/////////////////////////////////////////////////////////


void loop() {
  
  flash.waitforit(); 

  ///////////////////////////////////////////////////////random up a bank
  //Random is used to make sure it is errasing and programming.
  for (int i=0; i < array_length; i++) {
    random_bank[i] = random(0, 255);

  }

  ///////////////////////////////////////////////////////erase

  Serial.println("Erasing");
  Serial.println();
  t = micros();
 // flash.erase_all();
  flash.erase_4k(location);
  d = micros() - t;

  Serial.println();
  Serial.print("Erased in ");
  Serial.print(d);
  Serial.println(" microseconds");

  ////////////////////////////////////////////////////write
  t = micros();
  flash.write(location, random_bank, array_length); //the middle variable is a pointer meaning you put an
  d = micros() - t;

  Serial.println();
  Serial.print("Written in ");
  Serial.print(d);
  Serial.println(" microseconds");

  flash.waitforit();

  ////////////////////////////////////////////////////read


  t = micros();
  flash.read(location, read_bank, array_length); //the middle variable is a pointer meaning you put an
  //arrays name there but not it's location "[x]" 
  d = micros() - t;

  Serial.println();
  Serial.print("Read in ");
  Serial.print(d);
  Serial.println(" microseconds");
  Serial.println();

  flash.waitforit();

  ////////////////////////////////////////////////////check
  for (int i=0; i < array_length; i++) {
    if (random_bank[i] != read_bank[i]) {
      errors++;
    }
  }
  ////////////////////////////////////////////////////print
  /*
  for (int i=0; i < array_length; i++) {
    
    Serial.print(i + loation);
    Serial.print(" ");
    Serial.print(random_bank[i]);    
    Serial.print(" ");
    Serial.print(read_bank[i]);    
    Serial.print(" ");
    Serial.println( );


  }
  */
  
  cycles++;

  Serial.print(errors);   
  Serial.print(" errors in ");   
  Serial.print(cycles);   
  Serial.println(" cycles");

  Serial.println();
  Serial.println("done");
  Serial.println();

  delay(5000);

}




