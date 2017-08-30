#ifndef XPCC_STM32_ETH_HPP
#define XPCC_STM32_ETH_HPP


#include "../../../device.hpp"
#include "../../../type_ids.hpp"


namespace xpcc
{

namespace stm32
{

struct eth
{
	static constexpr uint32_t ETH_MACMIIAR_CR_MASK = 0xFFFFFFE3;
};

class Eth : public eth
{
public:
	/// TypeId used to connect GPIO pins to this peripheral's pins.
	static const TypeId::EthMdc  Mdc;
	static const TypeId::EthMdio Mdio;

	static const TypeId::EthTxEn TxEn;
	static const TypeId::EthTxD0 TxD0;
	static const TypeId::EthTxD1 TxD1;

	static const TypeId::EthPps  Pps;

public:
	static bool
	initialize();

	static uint16_t
	readPhy(const uint8_t reg_address);

	static void
	writePhy(const uint8_t reg_address, const uint16_t value);
};

} // stm32 namespace
} // xpcc namespace

#include "eth_impl.hpp"

#endif // XPCC_STM32_ETH_HPP