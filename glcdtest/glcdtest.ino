#include "glcd.h"
#include "SPI.h"
#include "logo.h"
void setup() {
  SerialUSB.begin(115200);
  while(!SerialUSB){};
  SerialUSB.println("Powering LCD");
  SerialUSB.println(BLACK);
  pinMode(PIN_LED_TXL, OUTPUT);
  digitalWrite(PIN_LED_TXL, HIGH);
  pinMode(PIN_LED_RXL, OUTPUT);
  digitalWrite(PIN_LED_RXL, HIGH);
  pinMode(LCD_POWER, OUTPUT);
  digitalWrite(LCD_POWER, LOW);
  pinMode(LCD_BACKLIGHT, OUTPUT);
  digitalWrite(LCD_BACKLIGHT, LOW);
  SerialUSB.println("Initing LCD");
  GLCD.Init(NON_INVERTED);  
  GLCD.DrawBitmap(logo, 0,  (64-46)/2); 
  GLCD.display();
}
 
void loop() {
  // put your main code here, to run repeatedly:
 
}
