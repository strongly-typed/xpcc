#include <xpcc/architecture/platform.hpp>

#include <xpcc/processing/timer.hpp>
#include <xpcc/processing/protothread.hpp>

using namespace Board;

/*
 * Blinks the green user LED with 1 Hz.
 * It is on for 90% of the time and off for 10% of the time.
 */


/*
 *********** TEST PROTOCOL *************

 [OK] SystemClock at 8 MHz
 [OK] Four LEDs
 [OK] Two Buttons
 [OK] Power supply
 [OK] External Clock with oscilloscope
 [OK] Use External Clock
 [OK] UART
 [OK] GPIOs on PinHeader (some pins unusable because of debugging config)
 [OK] Observe MCO on Oscilloscope
      [OK] 25 MHz from HSE
      [OK] 32 MHz from PLL1 (:5 *4)
 [  ] Create Device file with alternate functions on pin header
 [OK] CAN ** FAIL ** PB8 has short circuit to RESET ** FAIL **
 [OK] PLL
 [  ] Fix Ethernet LEDs?
 [  ] Ethernet

 ToDo
 ====
 [  ] Fix RESET (Done with Wire)
 [  ] Use available uC pins
 [  ] More silkscreen
 [  ] Change LDO
 [  ] Sort schematic

 */

using LedOrange = GpioOutputC7; // orange
using LedYellow = GpioOutputC6; // yellow
using LedRed = xpcc::GpioInverted<GpioOutputB15>;
using LedGreen = xpcc::GpioInverted<GpioOutputB14>;

using Button1 = GpioInputC8;
using Button2 = GpioInputC9;

int
main()
{
	Board::initialize();

	LedOrange::setOutput();
	LedYellow::setOutput();
	LedRed::setOutput();
	LedGreen::setOutput();

	LedRed::reset();
	LedGreen::reset();

	Button1::setInput(Gpio::InputType::PullUp);
	Button2::setInput(Gpio::InputType::PullUp);

	// initialize Uart2 for XPCC_LOG_*
	GpioOutputA9::connect(Usart1::Tx);
	GpioInputA10::connect(Usart1::Rx, Gpio::InputType::PullUp);
	Usart1::initialize<Board::systemClock, 115200>(12);

	// Init CAN
	GpioOutputB9::connect(Can1::Tx, Gpio::OutputType::PushPull);
	GpioInputB8::connect(Can1::Rx, Gpio::InputType::PullUp);
	Can1::initialize<Board::systemClock, Can1::Bitrate::kBps125>(12);

	// Receive every CAN message
	CanFilter::setFilter(0, CanFilter::FIFO0,
                        CanFilter::ExtendedIdentifier(0),
                        CanFilter::ExtendedFilterMask(0));


	// 25 MHz HSE at MCO
	if (false) {
		ClockControl::enableClockOutput(ClockControl::ClockOutputSource::SystemClock);
		GpioOutputA8::connect(ClockControl::ClockOutput);
	}

	if (false) {
		// 25 MHz / 5 * 4 = 20 MHz PLL
		bool ret = ClockControl::enablePll(
			ClockControl::PllSource::ExternalClock,
			/* pllMul    = */ 4,
			/* pllPrediv = */ 5);

		ret ? LedGreen::set() : LedRed::set();

		// Observe 20 MHz / 2 = 10 MHz at MCO
		ClockControl::enableClockOutput(ClockControl::ClockOutputSource::Pll);
		GpioOutputA8::connect(ClockControl::ClockOutput);
	}


	if (false) {
		// 25 MHz / 5 * 9 = 45 MHz PLL
		// PLL Input:   3.0 < 25/5 = 5 <  12.00 MHz
		// PLL Output: 18.0 < 45       <  72.00 MHz
		// PLL VCO:    36.0 < 45*2=90  < 144.00 MHz

		bool ret = ClockControl::enablePll(
			ClockControl::PllSource::ExternalClock,
			/* pllMul    = */ 9,
			/* pllPrediv = */ 5);

		ret ? LedGreen::set() : LedRed::set();

		// Observe 45 MHz / 2 = 22.5 MHz at MCO
		ClockControl::enableClockOutput(ClockControl::ClockOutputSource::Pll);
		GpioOutputA8::connect(ClockControl::ClockOutput);
	}


	if (false)
	{
		// PLL2 at 40 MHz to MCO
		//  3.0 MHz < PLL2 in  <  5.0 MHz
		// 40.0 MHz < Pll2 out < 74.0 MHz
		ClockControl::setPll2Pll3Prescaler(5); // 25 / 5 = 5 OK
		bool ret = ClockControl::enablePll2(8); // 5 * 8 = 40 OK

		ret ? LedGreen::set() : LedRed::set();

		ClockControl::enableClockOutput(ClockControl::ClockOutputSource::Pll2);
		GpioOutputA8::connect(ClockControl::ClockOutput);
	}


	if (true)
	{
		// PLL2 at 40 MHz to Pll
		//  3.0 MHz < PLL2 in  <  5.0 MHz
		// 40.0 MHz < Pll2 out < 74.0 MHz
		ClockControl::setPll2Pll3Prescaler(5); // 25 / 5 = 5 OK
		bool ret = ClockControl::enablePll2(8); // 5 * 8 = 40 OK

		// PLL3 at 50 MHz to MCO
		//  3.0 MHz < PLL3 in  <  5.0 MHz
		// 40.0 MHz < Pll3 out < 74.0 MHz

		// Shared prescaler
		// ClockControl::setPll2Pll3Prescaler(5); // 25 / 5 = 5 OK
		ret &= ClockControl::enablePll3(10); // 5 * 10 = 50 OK

		// PLL at 72 MHz
		ret &= ClockControl::setPrediv1Source(ClockControl::Prediv1Source::Pll2);

		ret &= ClockControl::enablePll(
			ClockControl::PllSource::Hse,
			/* pllMul    = */ 9,
			/* pllPrediv = */ 5);

		// 72 / 2 = 36 MHz from PLL
		// ClockControl::enableClockOutput(ClockControl::ClockOutputSource::Pll);

		// 50 MHz from PLL3 to Ethernet
		ret &= ClockControl::enableClockOutput(ClockControl::ClockOutputSource::Pll3);
		GpioOutputA8::connect(ClockControl::ClockOutput);

		// Run core on PLL with 72 MHz
		ret &= ClockControl::enableSystemClock(ClockControl::SystemClockSource::Pll);

		ret ? LedGreen::set() : LedRed::set();
	}

	// Unusable (debug pins)
	// GpioOutputB4::disconnect();
	// GpioOutputB3::disconnect();

	xpcc::can::Message msg1(1, 1);
    msg1.setExtended(true);
    msg1.data[0] = 0x11;
    Can1::sendMessage(msg1);

	// GpioOutputB9::setOutput();

	while(true)
	{
		LedGreen::toggle();
		// GpioOutputB9::toggle();
		xpcc::delayMilliseconds(500);
	}

	while (true)
	{
		Usart1::write('s');
		xpcc::delayMilliseconds(500);

		Usart1::write('t');
		// if (not Button1::read()) { Led3::set(); }
		// if (not Button2::read()) { Led4::set(); }
		xpcc::delayMilliseconds(500);

		uint8_t cc;
		if (Usart1::read(cc)) { 
			Usart1::write(cc + 1);
		}
	}

	return 0;
}
