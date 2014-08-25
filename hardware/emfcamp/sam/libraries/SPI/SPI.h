/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include "variant.h"
#include <stdio.h>

#define SPI_MODE0 0x02
#define SPI_MODE1 0x00
#define SPI_MODE2 0x03
#define SPI_MODE3 0x01

enum SPITransferMode {
	SPI_CONTINUE,
	SPI_LAST
};

class SPIClass {
  public:
	SPIClass(Spi *_spi, uint32_t _id, void(*_initCb)(void));

	byte transfer(uint8_t _data, SPITransferMode _mode = SPI_LAST) { return transfer(BOARD_SPI_DEFAULT_SS, _data, _mode); }
	byte transfer(byte _channel, uint8_t _data, SPITransferMode _mode = SPI_LAST);

	// SPI Configuration methods

	void attachInterrupt(void);
	void detachInterrupt(void);

	void begin(void);
	void end(void);

	// Attach/Detach pin to/from SPI controller
	void begin(uint8_t _pin);
	void end(uint8_t _pin);

	// These methods sets a parameter on a single pin
	void setBitOrder(uint8_t _pin, BitOrder);
	void setDataMode(uint8_t _pin, uint8_t);
	void setClockDivider(uint8_t _pin, uint8_t);
	void setDLYBCT(uint8_t _pin, uint8_t);

	// These methods sets the same parameters but on default pin BOARD_SPI_DEFAULT_SS
	void setBitOrder(BitOrder _order) { setBitOrder(BOARD_SPI_DEFAULT_SS, _order); };
	void setDataMode(uint8_t _mode) { setDataMode(BOARD_SPI_DEFAULT_SS, _mode); };
	void setClockDivider(uint8_t _div) { setClockDivider(BOARD_SPI_DEFAULT_SS, _div); };
	void setDLYBCT(uint8_t _delay) {setDLYBCT(BOARD_SPI_DEFAULT_SS, _delay); };
	
	// DMA Transfer interface
	// in order to use 8 bit buffers dma transfer is done using fixed periphal selection instead of varible, this mean we have to reconfigure the SPI before and after an xfer
	void configureDMA(uint8_t _txChannel=0, uint8_t _rxChannel=1);
	// once a tranfser is commplete the DMA interrupt is called, during which we do some clean up and call the user function
	void registerDMACallback(void (*_dmaCb)(void)) { dmaCb = _dmaCb;};
	// tx and rx buffer can be the same or diffrent
	byte transferDMA(uint32_t _pin, uint8_t *txBuffer, uint8_t *rxBuffer, uint16_t length, SPITransferMode _mode = SPI_LAST);
    // called from the DMA_Handler
    void DMAFinished(uint32_t _status);

  private:
	void init();

	Spi *spi;
	uint32_t id;
	BitOrder bitOrder[SPI_CHANNELS_NUM];
	uint32_t divider[SPI_CHANNELS_NUM];
	uint32_t mode[SPI_CHANNELS_NUM];
	uint32_t dlybct[SPI_CHANNELS_NUM];
	uint8_t txChannel;
	uint8_t rxChannel;
	void (*initCb)(void);
	void (*dmaCb) (void);
	bool initialized;
};

#if SPI_INTERFACES_COUNT > 0
extern SPIClass SPI;
#endif

#endif
