extern byte _sapp;
extern byte _eapp;

#define LED_OFF HIGH
#define LED_ON LOW

int red = LED1_RED;
int green = LED1_GREEN;
int blue = LED1_BLUE;

int lastMillis;
int lastPin;
byte * appLoadDest = 0;
bool appLoaded = false;
bool verify = false;

void setup() {
    // PMIC to CHARGE
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);

    // setup LED's
    pinMode(red, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(blue, OUTPUT);
    SerialUSB.begin(115200);
    while (!SerialUSB);

    lastMillis = millis();
    lastPin = -1;
}

bool flash(int pin, int interval) {
    int newMillis = millis();
    int lastState = (lastMillis / interval) % 2 ? LED_ON : LED_OFF;
    int newState = (newMillis / interval) % 2 ? LED_ON : LED_OFF;
    lastMillis = newMillis;
    if (lastPin != pin) {
        if (lastPin != -1) digitalWrite(lastPin, LED_OFF);
        lastPin = pin;
    }
    if (newState != lastState || lastPin != pin) {
        digitalWrite(pin, newState);
    }
    return newState != lastState;
}

int chartoi(char c) {
    return (c < 'a') ? c - '0' : (c - 'a' + 10);
}

void loop() {
    if (SerialUSB.available() > 0) {
        if (appLoadDest == 0) {
            appLoadDest = &_sapp;
        }
        byte b = SerialUSB.read();

        if (verify) {
            if (((int) appLoadDest & 0xf) == 0) SerialUSB.println();
            if (b < 0x10) SerialUSB.print("0");
            SerialUSB.print(b, HEX);
            if (*appLoadDest < 0x10) SerialUSB.print("0");
            SerialUSB.print(*appLoadDest, HEX);
            SerialUSB.print(" ");
        }

        *appLoadDest++ = b;
        if (appLoadDest >= &_eapp) {
            appLoaded = true;
            appLoadDest = 0;
        }
    } else if (appLoadDest) {
        flash(red, 100);
    } else if (appLoaded) {
        app();
    } else {
        flash(blue, 2000);
    }
}

__attribute__ ((used, section(".appdata")))
int counter = 0x1234;

__attribute__ ((used, section(".app")))
void app() {
    // NB let touch .data globals, instruction bus will be contended, library calls will go via veneer
    if (flash(green, 500)) {
        SerialUSB.print("Hi from app: ");
        SerialUSB.println(counter++, HEX);
    }
}

