#include <xpcc/architecture.hpp>
#include "../../mchck.hpp"

class DummyClock
{
public:
	static constexpr int Uart0 = 21 * 1000 * 1000 / 2;
};

// ----------------------------------------------------------------------------
MAIN_FUNCTION
{


	// Initialize Uart Hal
	PL12Output::connect(UartHal0::Tx);
	UartHal0::initialize<DummyClock, 115200>(UartHal0::Parity::Odd);
	UartHal0::enableInterruptVector(true, 6);
	UartHal0::enableInterrupt(UartHal0::Interrupt::TxEmpty);
	UartHal0::setTransmitterEnable(true);

	while (1)
	{
	}

	return 0;
}


// Interrupt Handler
extern "C" void
UART0_RX_TX_IRQHandler(void)
{
	if(UartHal0::isTransmitRegisterEmpty()) {
		UartHal0::write('A');
	}
}
