// -*- mode: c++ -*-
// Copyright 2013 Pervasive Displays, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied.  See the License for the specific language
// governing permissions and limitations under the License.

/*
 Orignal from https://github.com/repaper/gratis/
 Modified my EMF Camp for the TiLDA Mk2
 */

// Simple demo to toggle EPD between two images.

// Operation from reset:
// * display version
// * display compiled-in display setting
// * display temperature (displayed before every image is changed)
// * clear screen
// * delay 5 seconds (flash LED)
// * display text image
// * delay 5 seconds (flash LED)
// * display picture
// * delay 5 seconds (flash LED)
// * back to text display


#include <inttypes.h>
#include <ctype.h>

// required libraries
#include <SPI.h>
//#include <FLASH.h>
#include <TiLDA_EPD.h>
#include <S5813A.h>


// Change this for different display size
// supported sizes: 144 200 270
#define SCREEN_SIZE 270

// select two images from:  text_image text-hello cat aphrodite venus saturn
#define IMAGE_1  saturn
#define IMAGE_2  cat

// set up images from screen size2
#if (SCREEN_SIZE == 144)
#define EPD_SIZE EPD_1_44
#define FILE_SUFFIX _1_44.xbm
#define NAME_SUFFIX _1_44_bits

#elif (SCREEN_SIZE == 200)
#define EPD_SIZE EPD_2_0
#define FILE_SUFFIX _2_0.xbm
#define NAME_SUFFIX _2_0_bits

#elif (SCREEN_SIZE == 270)
#define EPD_SIZE EPD_2_7
#define FILE_SUFFIX _2_7.xbm
#define NAME_SUFFIX _2_7_bits

#else
#error "Unknown EPB size: Change the #define SCREEN_SIZE to a supported value"
#endif

// Error message for MSP430
#if (SCREEN_SIZE == 270) && defined(__MSP430_CPU__)
#error MSP430: not enough memory
#endif

// no futher changed below this point

// current version number
#define DEMO_VERSION "2"


// pre-processor convert to string
#define MAKE_STRING1(X) #X
#define MAKE_STRING(X) MAKE_STRING1(X)

// other pre-processor magic
// tiken joining and computing the string for #include
#define ID(X) X
#define MAKE_NAME1(X,Y) ID(X##Y)
#define MAKE_NAME(X,Y) MAKE_NAME1(X,Y)
#define MAKE_JOIN(X,Y) MAKE_STRING(MAKE_NAME(X,Y))

// calculate the include name and variable names
#define IMAGE_1_FILE MAKE_JOIN(IMAGE_1,FILE_SUFFIX)
#define IMAGE_1_BITS MAKE_NAME(IMAGE_1,NAME_SUFFIX)
#define IMAGE_2_FILE MAKE_JOIN(IMAGE_2,FILE_SUFFIX)
#define IMAGE_2_BITS MAKE_NAME(IMAGE_2,NAME_SUFFIX)


// Add Images library to compiler path
#include <Images.h>  // this is just an empty file

// images
PROGMEM const
#define unsigned
#define char uint8_t
#include IMAGE_1_FILE
#undef char
#undef unsigned

PROGMEM const
#define unsigned
#define char uint8_t
#include IMAGE_2_FILE
#undef char
#undef unsigned


#if defined(__MSP430_CPU__)

// TI LaunchPad IO layout
const int Pin_TEMPERATURE = A4;
const int Pin_PANEL_ON = P2_3;
const int Pin_BORDER = P2_5;
const int Pin_DISCHARGE = P2_4;
const int Pin_PWM = P2_1;
const int Pin_RESET = P2_2;
const int Pin_BUSY = P2_0;
const int Pin_EPD_CS = P2_6;
const int Pin_FLASH_CS = P2_7;
const int Pin_SW2 = P1_3;
const int Pin_RED_LED = P1_0;

#elif defined(_SAM3XA_)
/*
 // Arduino DUE IO layout
 const int Pin_TEMPERATURE = A0;
 const int Pin_PANEL_ON = 2;
 const int Pin_BORDER = 3;
 const int Pin_DISCHARGE = 9;
 const int Pin_PWM = 5;
 const int Pin_RESET = 6;
 const int Pin_BUSY = 7;
 const int Pin_EPD_CS = 52;
 const int Pin_FLASH_CS = 10;
 const int Pin_SW2 = 12;
 const int Pin_RED_LED = 13;
 */
/*
 // TILDA IO layout
 const int Pin_TEMPERATURE = A0;
 const int Pin_PANEL_ON = 33;
 const int Pin_BORDER = 40;
 const int Pin_DISCHARGE = 32;
 const int Pin_PWM = 35;
 const int Pin_RESET = 34;
 const int Pin_BUSY = 38;
 const int Pin_EPD_CS = 52;
 const int Pin_FLASH_CS = 10;
 const int Pin_SW2 = 12;
 const int Pin_RED_LED = 13;
 */
// TILDA IO layout with Defines
const int Pin_TEMPERATURE = A0;
const int Pin_PANEL_ON = EPD_PANEL_ON;
const int Pin_BORDER = EPD_BORDER_CONTROL;
const int Pin_DISCHARGE = EPD_DISCHARGE;
const int Pin_PWM = EPD_PWM;
const int Pin_RESET = EPD_RESET;
const int Pin_BUSY = EPD_BUSY;
const int Pin_EPD_CS = EPD_CS;
const int Pin_FLASH_CS = 10;
const int Pin_SW2 = 12;
const int Pin_RED_LED = 13;



#else

// Arduino IO layout
const int Pin_TEMPERATURE = A0;
const int Pin_PANEL_ON = 3;
const int Pin_BORDER = 3;
const int Pin_DISCHARGE = 4;
const int Pin_PWM = 5;
const int Pin_RESET = 6;
const int Pin_BUSY = 7;
const int Pin_EPD_CS = 8;
const int Pin_FLASH_CS = 9;
const int Pin_SW2 = 12;
const int Pin_RED_LED = 13;

#endif


// LED anode through resistor to I/O pin
// LED cathode to Ground
#define LED_ON  HIGH
#define LED_OFF LOW


// define the E-Ink display
EPD_Class EPD(EPD_SIZE, Pin_PANEL_ON, Pin_BORDER, Pin_DISCHARGE, Pin_PWM, Pin_RESET, Pin_BUSY, Pin_EPD_CS);


// I/O setup
void setup() {
	pinMode(Pin_RED_LED, OUTPUT);
	pinMode(Pin_SW2, INPUT);
	pinMode(Pin_TEMPERATURE, INPUT);
    //	pinMode(Pin_PWM, OUTPUT);
	pinMode(Pin_BUSY, INPUT);
	pinMode(Pin_RESET, OUTPUT);
	pinMode(Pin_PANEL_ON, OUTPUT);
	pinMode(Pin_DISCHARGE, OUTPUT);
	pinMode(Pin_BORDER, OUTPUT);
	pinMode(Pin_EPD_CS, OUTPUT);
	pinMode(Pin_FLASH_CS, OUTPUT);
    
	digitalWrite(Pin_RED_LED, LOW);
    //	digitalWrite(Pin_PWM, LOW);
	digitalWrite(Pin_RESET, LOW);
	digitalWrite(Pin_PANEL_ON, LOW);
	digitalWrite(Pin_DISCHARGE, LOW);
	digitalWrite(Pin_BORDER, LOW);
	digitalWrite(Pin_EPD_CS, LOW);
	digitalWrite(Pin_FLASH_CS, HIGH);
    
    // PMIC to CHARGE
    pinMode(PMIC_ENOTG, OUTPUT);
    digitalWrite(PMIC_ENOTG, LOW);
    
	Serial.begin(115200);
    #if !defined(__MSP430_CPU__)
	// wait for USB CDC serial port to connect.  Arduino Leonardo only
	while (!Serial) {
	}
    #endif
	Serial.println();
	Serial.println();
	Serial.println("Demo version: " DEMO_VERSION);
	Serial.println("Display: " MAKE_STRING(EPD_SIZE));
	Serial.println();
    
    // if DUE skip the flash stuff it's not used or fixed
    #if !defined(_SAM3XA_)
	FLASH.begin(Pin_FLASH_CS);
	if (FLASH.available()) {
		Serial.println("FLASH chip detected OK");
	} else {
		uint8_t maufacturer;
		uint16_t device;
		FLASH.info(&maufacturer, &device);
		Serial.print("unsupported FLASH chip: MFG: 0x");
		Serial.print(maufacturer, HEX);
		Serial.print("  device: 0x");
		Serial.print(device, HEX);
		Serial.println();
	}
    #endif
    
	// configure temperature sensor
	S5813A.begin(Pin_TEMPERATURE);
}


static int state = 0;


// main loop
void loop() {
	int temperature = S5813A.read();
	Serial.print("Temperature = ");
	Serial.print(temperature);
	Serial.println(" Celcius");
    
	EPD.begin(); // power up the EPD panel
	EPD.setFactor(20); // adjust for current temperature
    
	int delay_counts = 50;
	switch(state) {
        default:
        case 0:         // clear the screen
        Serial.println("Clear");
		EPD.clear();
		state = 1;
		delay_counts = 5;  // reduce delay so first image come up quickly
		break;
        
        case 1:         // clear -> text
        Serial.println("Set 1");
		EPD.image(IMAGE_1_BITS);
		++state;
		break;
        
        case 2:         // text -> picture
        Serial.println("Set 2");
		EPD.image(IMAGE_1_BITS, IMAGE_2_BITS);
		++state;
		break;
        
        case 3:        // picture -> text
        Serial.println("Set 1");
		EPD.image(IMAGE_2_BITS, IMAGE_1_BITS);
		state = 2;  // backe to picture nex time
		break;
	}
	EPD.end();   // power down the EPD panel
    
	// flash LED for 5 seconds
	for (int x = 0; x < delay_counts; ++x) {
		digitalWrite(Pin_RED_LED, LED_ON);
		delay(50);
		digitalWrite(Pin_RED_LED, LED_OFF);
		delay(50);
	}
}
