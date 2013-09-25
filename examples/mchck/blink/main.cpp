
#include <xpcc/architecture.hpp>

// ----------------------------------------------------------------------------
using namespace xpcc::kinetis;

typedef GpioOutputC5 Led;

// ----------------------------------------------------------------------------
MAIN_FUNCTION
{
	//typedef Pll<ExternalOscillator<MHz8>, MHz72> clockSource;
	//StartupError err =
	//	SystemClock<clockSource>::enable();

	Led::setOutput(xpcc::Gpio::HIGH);

	// Output SystemClock on PA8
	// ClockOut::setOutput(Gpio::PUSH_PULL);
	// ClockOut::connect(MCO::Id);
	// MCO::connect(clockSource::Id);

	while (1)
	{
		Led::toggle();
		xpcc::delay_ms(500);
	}

	return 0;
}
