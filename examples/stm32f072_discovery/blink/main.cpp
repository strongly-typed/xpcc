#include <xpcc/architecture.hpp>
#include "../stm32f072_discovery.hpp"

MAIN_FUNCTION
{
	LedRight::setOutput(xpcc::Gpio::Low);
	LedUp::setOutput(xpcc::Gpio::High);
	LedLeft::setOutput(xpcc::Gpio::Low);
	LedDown::setOutput(xpcc::Gpio::High);


	while (1)
	{
		LedRight::toggle();
		LedUp::toggle();
		LedLeft::toggle();
		LedDown::toggle();
		xpcc::delayMilliseconds(1000);
	}

	return 0;
}
