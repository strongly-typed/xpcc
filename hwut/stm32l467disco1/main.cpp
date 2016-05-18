#include <xpcc/architecture/platform.hpp>

using namespace Board;

// ----------------------------------------------------------------------------
int
main()
{
	initialize();

	LedRed::set();

	while (true)
	{
		LedGreen::toggle();
		LedRed::toggle();
		xpcc::delayMilliseconds(25);

		LedGreen::toggle();
		LedRed::toggle();
		xpcc::delayMilliseconds(Button::read() ? 50 : 100);
	}

	return 0;
}
