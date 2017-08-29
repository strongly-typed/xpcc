#ifndef XPCC_STM32_ETH_HPP
#define XPCC_STM32_ETH_HPP


#include "../../../device.hpp"
#include "../../../type_ids.hpp"


namespace xpcc
{

namespace stm32
{

class Eth
{


public:
	/// TypeId used to connect GPIO pins to this peripheral's pins.
	static const TypeId::EthMdc  Mdc;
	static const TypeId::EthMdio Mdio;

};

} // stm32 namespace
} // xpcc namespace

#endif // XPCC_STM32_ETH_HPP