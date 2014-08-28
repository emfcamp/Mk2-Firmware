/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "SPI.h"
#include "utility/dmac.h"

SPIClass::SPIClass(Spi *_spi, uint32_t _id, void(*_initCb)(void)) :
	spi(_spi), id(_id), initCb(_initCb), initialized(false)
{
	// Empty
}

void SPIClass::begin() {
	init();

	// NPCS control is left to the user

	// Default speed set to 4Mhz
	setClockDivider(BOARD_SPI_DEFAULT_SS, 21);
	setDataMode(BOARD_SPI_DEFAULT_SS, SPI_MODE0);
	setBitOrder(BOARD_SPI_DEFAULT_SS, MSBFIRST);

	// SPI_CSR_DLYBCT(1) keeps CS enabled for 32 MCLK after a completed
	// transfer. Some device needs that for working properly.
	setDLYBCT(BOARD_SPI_DEFAULT_SS, 1);
}

void SPIClass::begin(uint8_t _pin) {
	init();

	uint32_t spiPin = BOARD_PIN_TO_SPI_PIN(_pin);
	PIO_Configure(
		g_APinDescription[spiPin].pPort,
		g_APinDescription[spiPin].ulPinType,
		g_APinDescription[spiPin].ulPin,
		g_APinDescription[spiPin].ulPinConfiguration);

	// Default speed set to 4Mhz
	setClockDivider(_pin, 21);
	setDataMode(_pin, SPI_MODE0);
	setBitOrder(_pin, MSBFIRST);
	// SPI_CSR_DLYBCT(1) keeps CS enabled for 32 MCLK after a completed
	// transfer. Some device needs that for working properly.
	setDLYBCT(_pin, 1);
}

void SPIClass::init() {
	if (initialized)
		return;
	initCb();
	SPI_Configure(spi, id, SPI_MR_MSTR | SPI_MR_PS | SPI_MR_MODFDIS);
	SPI_Enable(spi);
	initialized = true;
}

void SPIClass::end(uint8_t _pin) {
	uint32_t spiPin = BOARD_PIN_TO_SPI_PIN(_pin);
	// Setting the pin as INPUT will disconnect it from SPI peripheral
	pinMode(spiPin, INPUT);
}

void SPIClass::end() {
	SPI_Disable(spi);
	initialized = false;
}

void SPIClass::setBitOrder(uint8_t _pin, BitOrder _bitOrder) {
	uint32_t ch = BOARD_PIN_TO_SPI_CHANNEL(_pin);
	bitOrder[ch] = _bitOrder;
}

void SPIClass::setDataMode(uint8_t _pin, uint8_t _mode) {
	uint32_t ch = BOARD_PIN_TO_SPI_CHANNEL(_pin);
	mode[ch] = _mode;
	SPI_ConfigureNPCS(spi, ch, mode[ch] | SPI_CSR_CSAAT | SPI_CSR_SCBR(divider[ch]) | SPI_CSR_DLYBCT(dlybct[ch]));
}

void SPIClass::setClockDivider(uint8_t _pin, uint8_t _divider) {
	uint32_t ch = BOARD_PIN_TO_SPI_CHANNEL(_pin);
	divider[ch] = _divider;
	SPI_ConfigureNPCS(spi, ch, mode[ch] | SPI_CSR_CSAAT | SPI_CSR_SCBR(divider[ch]) | SPI_CSR_DLYBCT(dlybct[ch]));
}

void SPIClass::setDLYBCT(uint8_t _pin, uint8_t _delay) {
	uint32_t ch = BOARD_PIN_TO_SPI_CHANNEL(_pin);
	dlybct[ch] = _delay;
	SPI_ConfigureNPCS(spi, ch, mode[ch] | SPI_CSR_CSAAT | SPI_CSR_SCBR(divider[ch]) | SPI_CSR_DLYBCT(dlybct[ch]));
}

byte SPIClass::transfer(byte _pin, uint8_t _data, SPITransferMode _mode) {
	uint32_t ch = BOARD_PIN_TO_SPI_CHANNEL(_pin);
	// Reverse bit order
	if (bitOrder[ch] == LSBFIRST)
		_data = __REV(__RBIT(_data));
	uint32_t d = _data | SPI_PCS(ch);
	if (_mode == SPI_LAST)
		d |= SPI_TDR_LASTXFER;

	// SPI_Write(spi, _channel, _data);
    while ((spi->SPI_SR & SPI_SR_TDRE) == 0)
    	;
    spi->SPI_TDR = d;

    // return SPI_Read(spi);
    while ((spi->SPI_SR & SPI_SR_RDRF) == 0)
    	;
    d = spi->SPI_RDR;
	// Reverse bit order
	if (bitOrder[ch] == LSBFIRST)
		d = __REV(__RBIT(d));
    return d & 0xFF;
}

void SPIClass::attachInterrupt(void) {
	// Should be enableInterrupt()
}

void SPIClass::detachInterrupt(void) {
	// Should be disableInterrupt()
}

/* DMA interface by dps.lwk
*/

/** DMAC Channel HW Interface Number for SPI. */
#define SPI_TX_IDX              1
#define SPI_RX_IDX              2

/* we need to know which dma hardware channels to use for rx and tx, defaults of 0 & 1 should be fine unless there have been used for something else */
void SPIClass::configureDMA(uint8_t _txChannel, uint8_t _rxChannel) {
    txChannel = _txChannel;
    rxChannel = _rxChannel;
    
    // setup dma channel basics
    uint32_t ul_cfg;
    
    /* Initialize and enable DMA controller. */
    pmc_enable_periph_clk(ID_DMAC);
    dmac_init(DMAC);
    dmac_set_priority_mode(DMAC, DMAC_PRIORITY_ROUND_ROBIN);
    dmac_enable(DMAC);
    
    /* Configure SPI DMA RX channel. */
    ul_cfg = 0;
    ul_cfg |= DMAC_CFG_SRC_PER(SPI_RX_IDX) |
        DMAC_CFG_SRC_H2SEL |
        DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;
    dmac_channel_set_configuration(DMAC, rxChannel, ul_cfg);
    
    /* Configure SPI DMA TX channel. */
    ul_cfg = 0;
    ul_cfg |= DMAC_CFG_DST_PER(SPI_TX_IDX) |
        DMAC_CFG_DST_H2SEL |
        DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;
    dmac_channel_set_configuration(DMAC, txChannel, ul_cfg);
    
    /* Enable receive channel interrupt for DMAC. */
    NVIC_EnableIRQ(DMAC_IRQn);
    dmac_enable_interrupt(DMAC, (1 << rxChannel));

}

/* start a dma transfer, need to setup spi for a fixed peripheral xfer, and setup the dma channels
*/
byte SPIClass::transferDMA(uint32_t _pin, uint8_t *txBuffer, uint8_t *rxBuffer, uint16_t length, SPITransferMode _mode) {
    
    // reconfigure SPI for fixed peripheral mode
    //SPI_Disable(spi);
    spi->SPI_MR = SPI_MR_MSTR | SPI_MR_MODFDIS | SPI_MR_PCS(~(1 << BOARD_PIN_TO_SPI_CHANNEL(_pin)));
    if (_mode == SPI_LAST) {
        spi->SPI_CR = SPI_CR_LASTXFER;
    } else {
        spi->SPI_CR = 0; // clear a posible set SPI_CR_LASTXFER bit
    }
    //SPI_Enable(spi);

    // setup dma xfers
    dma_transfer_descriptor_t dmac_trans;
    
    dmac_channel_disable(DMAC, rxChannel);
    dmac_trans.ul_source_addr = (uint32_t) & spi->SPI_RDR;
    dmac_trans.ul_destination_addr = (uint32_t) rxBuffer;
    dmac_trans.ul_ctrlA = length | DMAC_CTRLA_SRC_WIDTH_BYTE |
                            DMAC_CTRLA_DST_WIDTH_BYTE;
    dmac_trans.ul_ctrlB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR |
                            DMAC_CTRLB_FC_PER2MEM_DMA_FC |
                            DMAC_CTRLB_SRC_INCR_FIXED |
                            DMAC_CTRLB_DST_INCR_INCREMENTING;
    dmac_trans.ul_descriptor_addr = 0;
    dmac_channel_single_buf_transfer_init(DMAC, rxChannel, &dmac_trans);
    dmac_channel_enable(DMAC, rxChannel);

    dmac_channel_disable(DMAC, txChannel);
    dmac_trans.ul_source_addr = (uint32_t) txBuffer;
    dmac_trans.ul_destination_addr = (uint32_t) & spi->SPI_TDR;
    dmac_trans.ul_ctrlA = length | DMAC_CTRLA_SRC_WIDTH_BYTE |
                            DMAC_CTRLA_DST_WIDTH_BYTE;
    dmac_trans.ul_ctrlB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR |
                            DMAC_CTRLB_FC_MEM2PER_DMA_FC |
                            DMAC_CTRLB_SRC_INCR_INCREMENTING |
                            DMAC_CTRLB_DST_INCR_FIXED;
    dmac_trans.ul_descriptor_addr = 0;
    dmac_channel_single_buf_transfer_init(DMAC, txChannel, &dmac_trans);        // &Dmac_trans was cast in the example don't think we need it as the example was not consistent in its casting
    dmac_channel_enable(DMAC, txChannel);

}

/* This is called from the DMA_Handler
 clean up the spi post xfer and call the users callback function.
 keep things short as we are called from interrupt
 */
void SPIClass::DMAFinished(uint32_t _status) {
    // check that it was our rxChannel finish interrupt that fired
    if (_status & (1 << rxChannel)) {
        
        // Clear CS
        SPI_Disable(spi);
        spi->SPI_MR = (SPI_MR_MSTR | SPI_MR_PS | SPI_MR_MODFDIS);
        SPI_Enable(spi);

        // clean up DMA?
        // channels should have been automatically disabled at end of xfer
        // TODO: Check DMA doesn't need clean up
        // call user callback
        dmaCb();
    }
}

/* DMA interrupt handler
 grab the DMA status and call the SPIClass::DMAFinish function
 */
void DMAC_Handler(void) {
    static uint32_t ul_status;
    ul_status = dmac_get_status(DMAC);
    SPI.DMAFinished(ul_status);
}

#if SPI_INTERFACES_COUNT > 0
static void SPI_0_Init(void) {
	PIO_Configure(
			g_APinDescription[PIN_SPI_MOSI].pPort,
			g_APinDescription[PIN_SPI_MOSI].ulPinType,
			g_APinDescription[PIN_SPI_MOSI].ulPin,
			g_APinDescription[PIN_SPI_MOSI].ulPinConfiguration);
	PIO_Configure(
			g_APinDescription[PIN_SPI_MISO].pPort,
			g_APinDescription[PIN_SPI_MISO].ulPinType,
			g_APinDescription[PIN_SPI_MISO].ulPin,
			g_APinDescription[PIN_SPI_MISO].ulPinConfiguration);
	PIO_Configure(
			g_APinDescription[PIN_SPI_SCK].pPort,
			g_APinDescription[PIN_SPI_SCK].ulPinType,
			g_APinDescription[PIN_SPI_SCK].ulPin,
			g_APinDescription[PIN_SPI_SCK].ulPinConfiguration);
}

SPIClass SPI(SPI_INTERFACE, SPI_INTERFACE_ID, SPI_0_Init);
#endif
