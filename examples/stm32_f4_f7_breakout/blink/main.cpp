#include <xpcc/architecture/platform.hpp>

using namespace Board;

// ----------------------------------------------------------------------------
int
main()
{
	initialize();

	LedGreen::set();
	LedRed::reset();

	uint8_t c = 'A';
	while (true)
	{
		LedGreen::toggle();
		LedRed::toggle();

		Usart1::write(c);
		++c;
		if (c > 'Z') {
			c = 'A';
		}

		xpcc::delayMilliseconds(Button::read() ? 250 : 500);
	}

	return 0;
}
