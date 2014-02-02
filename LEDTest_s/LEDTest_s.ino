void setup() {
  delay(100);
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("LED Test");
  pinMode(PIN_LED_RXL, OUTPUT);
  digitalWrite(PIN_LED_RXL, LOW);
  pinMode(PIN_LED_TXL, OUTPUT);
  digitalWrite(PIN_LED_TXL, LOW);
   
  analogWrite(LED1_RED, 1);
  analogWrite(LED1_GREEN, 0);
  analogWrite(LED1_BLUE, 0);
  analogWrite(LED2_RED, 0);
  analogWrite(LED2_GREEN, 1);
  analogWrite(LED2_BLUE, 0); 
}
  

void loop() {
  // put your main code here, to run repeatedly:

}
