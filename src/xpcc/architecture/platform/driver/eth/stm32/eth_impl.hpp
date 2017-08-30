// coding: utf-8
/* Copyright (c) 2017 Sascha Schade (strongly-typed)
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_STM32_ETH_HPP
#	error 	"Don't include this file directly, use 'eth.hpp' instead!"
#endif


bool
xpcc::stm32::Eth::initialize()
{
	// Ethernet MAC clock enable
	RCC->AHBENR |= RCC_AHBENR_ETHMACEN;

	// ToDo: Set CSR Clock Range

	// ETH_MACMIIAR
	// ETH_MACMIIDR

	return true;
}

uint16_t
xpcc::stm32::Eth::readPhy(const uint8_t reg_address)
{
	uint32_t tmp = ETH->MACMIIAR;

	// Only keep CSR Clock Range bits
	tmp &= ~ETH_MACMIIAR_CR_MASK;

	uint8_t phy_address = 0;
	tmp |= (phy_address << 11U) & ETH_MACMIIAR_PA;
	tmp |= (reg_address <<  6U) & ETH_MACMIIAR_MR;

	tmp &= ~ETH_MACMIIAR_MW;
	tmp |=  ETH_MACMIIAR_MB;

	ETH->MACMIIAR = tmp;

	// ToDo: Deadlock preventer and xpcc_assert
	while((tmp & ETH_MACMIIAR_MB) == ETH_MACMIIAR_MB) {
		tmp = ETH->MACMIIAR;
	}

	return ETH->MACMIIDR;
}

void
xpcc::stm32::Eth::writePhy(const uint8_t reg_address, const uint16_t value)
{
	uint32_t tmp = ETH->MACMIIAR;

	// Only keep CSR Clock Range bits
	tmp &= ~ETH_MACMIIAR_CR_MASK;

	uint8_t phy_address = 0;
	tmp |= (phy_address << 11U) & ETH_MACMIIAR_PA;
	tmp |= (reg_address <<  6U) & ETH_MACMIIAR_MR;

	// Write and busy mode
	tmp |=  ETH_MACMIIAR_MW;
	tmp |=  ETH_MACMIIAR_MB;

  	ETH->MACMIIDR = value;

	// Start write
  	ETH->MACMIIAR = tmp;

	// ToDo: Deadlock preventer and xpcc_assert
	while((tmp & ETH_MACMIIAR_MB) == ETH_MACMIIAR_MB) {
		tmp = ETH->MACMIIAR;
	}
}
