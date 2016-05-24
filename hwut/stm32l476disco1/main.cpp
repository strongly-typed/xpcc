#include <xpcc/architecture/platform.hpp>

using namespace Board;

// ----------------------------------------------------------------------------
void
cpufreqtest()
{
	LedRed::set();

	for (uint8_t ii = 10; ii > 0; --ii)
	{
		LedGreen::toggle();
		LedRed::toggle();
		xpcc::delayMilliseconds(25);

		LedGreen::toggle();
		LedRed::toggle();
		xpcc::delayMilliseconds(Button::read() ? 50 : 100);
	}
}

void
uarttest115200()
{
	LedRed::set();
	LedGreen::set();

	for (uint8_t ii = 10; ii > 0; --ii)
	{
		LedGreen::toggle();
		LedRed::toggle();
		xpcc::delayMilliseconds(25);

		LedGreen::toggle();
		LedRed::toggle();
		xpcc::delayMilliseconds(Button::read() ? 50 : 100);
	}

}

void hwut_begin()         { asm volatile ("nop"); }
void hwut_end()           { asm volatile ("nop"); }
void cpufreqtest_end()    { asm volatile ("nop"); }
void uarttest115200_end() { asm volatile ("nop"); }

// ----------------------------------------------------------------------------
int
main()
{
	initialize();

	hwut_begin();

	cpufreqtest();
	cpufreqtest_end();

	uarttest115200();
	uarttest115200_end();

	hwut_end();

	while(true)
		{};

	return 0;
}
