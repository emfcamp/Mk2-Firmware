/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _VARIANT_TILDA_MK2_
#define _VARIANT_TILDA_MK2_

/*----------------------------------------------------------------------------
 *        Definitions
 *----------------------------------------------------------------------------*/

/** Frequency of the board main oscillator */
#define VARIANT_MAINOSC		12000000

/** Master clock frequency */
#define VARIANT_MCK			84000000

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "Arduino.h"
#ifdef __cplusplus
#include "UARTClass.h"
#include "USARTClass.h"
#endif

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

/**
 * Libc porting layers
 */
#if defined (  __GNUC__  ) /* GCC CS3 */
#    include <syscalls.h> /** RedHat Newlib minimal stub */
#endif

/*----------------------------------------------------------------------------
 *        Pins
 *----------------------------------------------------------------------------*/

// Number of pins defined in PinDescription array
#define PINS_COUNT           (79u)
#define NUM_DIGITAL_PINS     (54u)
#define NUM_ANALOG_INPUTS    (12u)

#define digitalPinToPort(P)        ( g_APinDescription[P].pPort )
#define digitalPinToBitMask(P)     ( g_APinDescription[P].ulPin )
#define digitalPinToTimer(P)       (  )
//#define analogInPinToBit(P)        ( )
#define portOutputRegister(port)   ( &(port->PIO_ODSR) )
#define portInputRegister(port)    ( &(port->PIO_PDSR) )
//#define portModeRegister(P)        (  )
#define digitalPinHasPWM(P)        ( g_APinDescription[P].ulPWMChannel != NOT_ON_PWM || g_APinDescription[P].ulTCChannel != NOT_ON_TIMER )

// Interrupts
#define digitalPinToInterrupt(p)  ((p) < NUM_DIGITAL_PINS ? (p) : -1)

// LEDs
#define PIN_LED_13           (13u)
#define PIN_LED_RXL          (72u)
#define PIN_LED_TXL          (73u)
#define PIN_LED              PIN_LED_13
#define PIN_LED2             PIN_LED_RXL
#define PIN_LED3             PIN_LED_TXL
#define LED_BUILTIN          13

/*
 * SPI Interfaces
 */
#define SPI_INTERFACES_COUNT 1

#define SPI_INTERFACE        SPI0
#define SPI_INTERFACE_ID     ID_SPI0
#define SPI_CHANNELS_NUM 4
#define PIN_SPI_SS0          (77u)
#define PIN_SPI_SS1          (87u)
#define PIN_SPI_SS2          (86u)
#define PIN_SPI_SS3          (78u)
#define PIN_SPI_MOSI         (75u)
#define PIN_SPI_MISO         (74u)
#define PIN_SPI_SCK          (76u)
#define BOARD_SPI_SS0        (10u)
#define BOARD_SPI_SS1        (4u)
#define BOARD_SPI_SS2        (52u)
#define BOARD_SPI_SS3        PIN_SPI_SS3
#define BOARD_SPI_DEFAULT_SS BOARD_SPI_SS3

#define BOARD_PIN_TO_SPI_PIN(x) \
	(x==BOARD_SPI_SS0 ? PIN_SPI_SS0 : \
	(x==BOARD_SPI_SS1 ? PIN_SPI_SS1 : \
	(x==BOARD_SPI_SS2 ? PIN_SPI_SS2 : PIN_SPI_SS3 )))
#define BOARD_PIN_TO_SPI_CHANNEL(x) \
	(x==BOARD_SPI_SS0 ? 0 : \
	(x==BOARD_SPI_SS1 ? 1 : \
	(x==BOARD_SPI_SS2 ? 2 : 3)))

#define SS   BOARD_SPI_SS0
#define SS1  BOARD_SPI_SS1
#define SS2  BOARD_SPI_SS2
#define SS3  BOARD_SPI_SS3
#define MOSI PIN_SPI_MOSI
#define MISO PIN_SPI_MISO
#define SCK  PIN_SPI_SCK

/*
 * Wire Interfaces
 */
#define WIRE_INTERFACES_COUNT 2

#define PIN_WIRE_SDA         (20u)
#define PIN_WIRE_SCL         (21u)
#define WIRE_INTERFACE       TWI1
#define WIRE_INTERFACE_ID    ID_TWI1
#define WIRE_ISR_HANDLER     TWI1_Handler

#define PIN_WIRE1_SDA        (70u)
#define PIN_WIRE1_SCL        (71u)
#define WIRE1_INTERFACE      TWI0
#define WIRE1_INTERFACE_ID   ID_TWI0
#define WIRE1_ISR_HANDLER    TWI0_Handler

/*
 * UART/USART Interfaces
 */
// Serial
#define PINS_UART            (81u)
// Serial1
#define PINS_USART0          (82u)
// Serial2
#define PINS_USART1          (83u)
// Serial3
#define PINS_USART3          (84u)

/*
 * USB Interfaces
 */
#define PINS_USB             (85u)

/*
 * Analog pins
 */
#define A0    54
#define A1    55
#define A2    56
#define A3    57
#define A4    58
#define A5    59
#define A6    60
#define A7    61
#define A8    62
#define A9    63
#define A10   64
#define A11   65
#define DAC0  66
#define DAC1  67
#define CANRX 68
#define CANTX 69
#define ADC_RESOLUTION		12

/*
 * Complementary CAN pins
 */
#define CAN1RX 88
#define CAN1TX 89

// CAN0
#define PINS_CAN0            (90u)
// CAN1
#define PINS_CAN1            (91u)

// EMAC
#define PIN_EMAC_EREFCK      (92u)
#define PIN_EMAC_ETXEN       (93u)
#define PIN_EMAC_ETX0        (94u)
#define PIN_EMAC_ETX1        (95u)
#define PIN_EMAC_ECRSDV      (96u)
#define PIN_EMAC_ERX0        (97u)
#define PIN_EMAC_ERX1        (98u)
#define PIN_EMAC_ERXER       (99u)
#define PIN_EMAC_EMDC        (100u)
#define PIN_EMAC_EMDIO       (101u)
#define PIN_TC0_TIOA2        (102u)
#define PIN_EMAC_FLAGS       (PIO_PERIPH_A | PIO_DEFAULT)
/** EMAC PHY address */
#define BOARD_EMAC_PHY_ADDR  0
/** EMAC RMII mode */
#define BOARD_EMAC_MODE_RMII 1
#define ETXCK                 PIN_EMAC_EREFCK
#define ETXEN                 PIN_EMAC_ETXEN
#define ETX0                  PIN_EMAC_ETX0
#define ETX1                  PIN_EMAC_ETX1
#define ECRSDV                PIN_EMAC_ECRSDV
#define ERX0                  PIN_EMAC_ERX0
#define ERX1                  PIN_EMAC_ERX1
#define ERXER                 PIN_EMAC_ERXER
#define EMDC                  PIN_EMAC_EMDC
#define EMDIO                 PIN_EMAC_EMDIO
#define TIOA2                 PIN_TC0_TIOA2

#define ERXD1                 ERX1
#define ETXD1                 ETX1
//#define ERXER                 PIN_EMAC_ERXER
#define ERXDV                 ECRSDV
//#define EMDC                  PIN_EMAC_EMDC
#define ERXD0                 ERX0
//#define EMDIO                 PIN_EMAC_EMDIO
#define ETXD0                 ETX0
//#define ETXEN                 PIN_EMAC_ETXEN
#define EXINT                 TIOA2
#define ETXCLK                ETXCK
#define CLKEN                 (109)       //PC11

//Taijiuino additional pins
#define PIN_TAIJI_TP0         (103u)
#define PIN_TAIJI_TP1         (104u)
#define PIN_TAIJI_TP2         (105u)
#define PIN_TAIJI_TP3         (106u)
#define PIN_TAIJI_TP4         (107u)
#define PIN_TAIJI_TP5         (108u)
#define PIN_TAIJI_TP6         (109u)
#define TP0                   PIN_TAIJI_TP0
#define TP1                   PIN_TAIJI_TP1
#define TP2                   PIN_TAIJI_TP2
#define TP3                   PIN_TAIJI_TP3
#define TP4                   PIN_TAIJI_TP4
#define TP5                   PIN_TAIJI_TP5
#define TP6                   PIN_TAIJI_TP6
#define TP7                   ERXD1
#define TP8                   ETXD1
#define TP9                   ERXER
#define TP10                  ERXDV
#define TP11                  EMDC
#define TP12                  ERXD0
#define TP13                  EMDIO
#define TP14                  ETXD0
#define TP15                  ETXEN
#define TP16                  EXINT
#define TP17                  ETXCLK
#define PB24                  TP0
#define PB23                  TP1
#define PB22                  TP2
#define PB27                  TP3
#define PC27                  TP3
#define PC20                  TP4
#define PC10                  TP5
#define PC11                  TP6

/*
 * DACC
 */
#define DACC_INTERFACE		DACC
#define DACC_INTERFACE_ID	ID_DACC
#define DACC_RESOLUTION		12
#define DACC_ISR_HANDLER    DACC_Handler
#define DACC_ISR_ID         DACC_IRQn

/*
 * PWM
 */
#define PWM_INTERFACE		PWM
#define PWM_INTERFACE_ID	ID_PWM
#define PWM_FREQUENCY		1000
#define PWM_MAX_DUTY_CYCLE	255
#define PWM_MIN_DUTY_CYCLE	0
#define PWM_RESOLUTION		8

/*
 * TC
 */
#define TC_INTERFACE        TC0
#define TC_INTERFACE_ID     ID_TC0
#define TC_FREQUENCY        1000
#define TC_MAX_DUTY_CYCLE   255
#define TC_MIN_DUTY_CYCLE   0
#define TC_RESOLUTION		8

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus

extern UARTClass Serial;
extern USARTClass Serial1;
extern USARTClass Serial2;
extern USARTClass Serial3;

#endif

// These serial port names are intended to allow libraries and architecture-neutral
// sketches to automatically default to the correct port name for a particular type
// of use.  For example, a GPS module would normally connect to SERIAL_PORT_HARDWARE_OPEN,
// the first hardware serial port whose RX/TX pins are not dedicated to another use.
//
// SERIAL_PORT_MONITOR        Port which normally prints to the Arduino Serial Monitor
//
// SERIAL_PORT_USBVIRTUAL     Port which is USB virtual serial
//
// SERIAL_PORT_LINUXBRIDGE    Port which connects to a Linux system via Bridge library
//
// SERIAL_PORT_HARDWARE       Hardware serial port, physical RX & TX pins.
//
// SERIAL_PORT_HARDWARE_OPEN  Hardware serial ports which are open for use.  Their RX & TX
//                            pins are NOT connected to anything by default.
#define SERIAL_PORT_MONITOR         Serial
#define SERIAL_PORT_USBVIRTUAL      SerialUSB
#define SERIAL_PORT_HARDWARE_OPEN   Serial1
#define SERIAL_PORT_HARDWARE_OPEN1  Serial2
#define SERIAL_PORT_HARDWARE_OPEN2  Serial3
#define SERIAL_PORT_HARDWARE        Serial
#define SERIAL_PORT_HARDWARE1       Serial1
#define SERIAL_PORT_HARDWARE2       Serial2
#define SERIAL_PORT_HARDWARE3       Serial3

#endif /* _VARIANT_TILDA_MK2_ */

