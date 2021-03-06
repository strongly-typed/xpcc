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
		xpcc::delayMilliseconds(Button::read() ? 250 : 500);
	}

	return 0;
}
