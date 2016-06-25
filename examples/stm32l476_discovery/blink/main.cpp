#include <xpcc/architecture/platform.hpp>

using namespace Board;

// ----------------------------------------------------------------------------
int
main()
{
	initialize();

	LedRed::set();

	// Enable USART 2
	GpioOutputA2::connect(xpcc::stm32::Usart2::Tx);
	GpioInputA3::connect(xpcc::stm32::Usart2::Rx, Gpio::InputType::PullUp);
	Usart2::initialize<Board::systemClock, 230400>(12);

	while (true)
	{
		static uint8_t c = 'A';
		LedGreen::toggle();
		LedRed::toggle();

		Usart2::write(c);
		++c;
		if (c > 'Z') {
			c = 'A';
		}
		xpcc::delayMilliseconds(Button::read() ? 250 : 500);
	}

	return 0;
}
