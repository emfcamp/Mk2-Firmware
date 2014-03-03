int red = LED1_RED;
int green = LED1_GREEN;
int blue = LED1_BLUE;

#define LED_OFF HIGH
#define LED_ON LOW

void setup() {
    // PMIC to CHARGE
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);

    SerialUSB.begin(115200);
    while (!SerialUSB); // in case we've hosed it

    pinMode(red, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(blue, OUTPUT);
}

void loop() {
    digitalWrite(blue, LED_ON);
    delay(1000);
    digitalWrite(blue, LED_OFF);
    delay(1000);
}

