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
	//StartupError err =
	//	SystemClock<Pll<ExternalClock<MHz8>, MHz48>>::enable();

	// Initialize Usart
	PL12Output::connect(Uart0::Tx);
	Uart0::initialize<DummyClock, 115200>();

	// If you want to output strings more comfortably
	// have a look at the logger example.
	// Write Single Bytes
	Uart0::writeBlocking('H');
	Uart0::writeBlocking('e');
	Uart0::writeBlocking('l');
	Uart0::writeBlocking('l');
	Uart0::writeBlocking('o');
	Uart0::writeBlocking('\n');

	// Write Block
	const uint8_t str [] = { 'W', 'o', 'r', 'l', 'd', '!', '\n' };
	Uart0::writeBlocking(str, sizeof(str));

	while (1)
	{
	}

	return 0;
}
