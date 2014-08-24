#include <SPI.h>
#include <S25FLx.h>

Flash flash(FLASH_CS, FLASH_HOLD);

void setup() {
    //PMIC
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);

    Serial.begin(115200);
    delay(500);
    Serial.println("FALSH id test");
    // Logic analiser trigger
    pinMode(11, OUTPUT);
    digitalWrite(11, LOW);
    digitalWrite(11, HIGH);
    digitalWrite(11, LOW);
    
    flash.begin();
    Serial.println("Flash SPI Started");

    flash.waitforit();
    Serial.println("Flash ready?");
    
    flash.read_info();
    digitalWrite(11, HIGH);
}

void loop(){
  delay(100);
}
