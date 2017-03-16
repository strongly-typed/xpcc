#include <xpcc/architecture/platform.hpp>

using namespace Board;

// ----------------------------------------------------------------------------
int
main()
{
	initialize();

	LedGreen::set();

	// Use the logging streams to print some messages.
	// Change XPCC_LOG_LEVEL above to enable or disable these messages
	XPCC_LOG_DEBUG   << "debug"   << xpcc::endl;
	XPCC_LOG_INFO    << "info"    << xpcc::endl;
	XPCC_LOG_WARNING << "warning" << xpcc::endl;
	XPCC_LOG_ERROR   << "error"   << xpcc::endl;

	uint32_t counter(0);

	while (true)
	{
		Leds::write(1 << (counter % (Leds::width + 1)));
		xpcc::delayMilliseconds(Button::read() ? 100 : 500);

		XPCC_LOG_INFO << "loop: " << counter++ << xpcc::endl;
	}

	return 0;
}
