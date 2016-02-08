#include "../stm32f429_nucleo144.hpp"

using namespace Board;

// ----------------------------------------------------------------------------
MAIN_FUNCTION
{
	Board::initialize();

	LedRed::set();
	// usb::VBus::setOutput(xpcc::Gpio::Low);
	// usb::Overcurrent::setOutput(xpcc::Gpio::Low);

	while (true)
	{
		LedRed::toggle();
		LedGreen::toggle();
		LedBlue::toggle();

		xpcc::delayMilliseconds(Button::read() ? 125 : 500);

		// usb::VBus::toggle();
		// usb::Overcurrent::toggle();

		xpcc::delayMilliseconds(Button::read() ? 125 : 500);
	}

	return 0;
}
