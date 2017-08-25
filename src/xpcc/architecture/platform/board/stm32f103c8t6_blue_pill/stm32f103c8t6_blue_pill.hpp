// coding: utf-8
/* Copyright (c) 2016, Roboterclub Aachen e. V.
* All Rights Reserved.
*
* The file is part of the xpcc library and is released under the 3-clause BSD
* license. See the file `LICENSE` for the full license governing this code.
*/
// ----------------------------------------------------------------------------

//
// STM32F103C8T6 "Blue Pill" Minimum System Development Board
//
// Cheap and bread-board-friendly board for STM32 F1 series.
// Sold for less than 2 USD on well known Internet shops from China.
//
// http://wiki.stm32duino.com/index.php?title=Blue_Pill
//

#ifndef XPCC_STM32_F103C8T6_BLUE_PILL_HPP
#define XPCC_STM32_F103C8T6_BLUE_PILL_HPP

#include <xpcc/architecture/platform.hpp>

using namespace xpcc::stm32;


namespace Board
{

/// STM32F108 running at 72MHz generated from the external 25MHz oscillator
// Dummy clock for devices
struct systemClock {
	static constexpr uint32_t Frequency = MHz72;
	static constexpr uint32_t Ahb = Frequency;
	static constexpr uint32_t Apb1 = Frequency / 2;
	static constexpr uint32_t Apb2 = Frequency;

	static constexpr uint32_t Adc1 = Apb2;
	static constexpr uint32_t Adc2 = Apb2;
	static constexpr uint32_t Adc3 = Apb2;

	static constexpr uint32_t Spi1 = Apb2;
	static constexpr uint32_t Spi2 = Apb1;
	static constexpr uint32_t Spi3 = Apb1;

	static constexpr uint32_t Usart1 = Apb2;
	static constexpr uint32_t Usart2 = Apb1;
	static constexpr uint32_t Usart3 = Apb1;
	static constexpr uint32_t Uart4  = Apb1;
	static constexpr uint32_t Uart5  = Apb1;

	static constexpr uint32_t Can1   = Apb1;
	static constexpr uint32_t Can2   = Apb1;

	static constexpr uint32_t I2c1   = Apb1;
	static constexpr uint32_t I2c2   = Apb1;

	static bool inline
	enable()
	{
		ClockControl::enableExternalClock();

		// PllMull = 4 ... 9
		// 
		// ClockControl::enablePll(
			// /* PllSource */ ClockControl::PllSource::ExternalClock,
			// /* PllMul    */ 6,
			// /* PllPrediv */ 2 , ClockControl::UsbPrescaler::Div1_5 , 3);

		// set flash latency for 72MHz
		ClockControl::setFlashLatency(Frequency);

		// switch system clock to PLL output
		// ClockControl::enableSystemClock(ClockControl::SystemClockSource::Pll);
		ClockControl::enableSystemClock(ClockControl::SystemClockSource::ExternalClock);

		// AHB has max 72MHz
		ClockControl::setAhbPrescaler(ClockControl::AhbPrescaler::Div1);

		// APB1 has max. 36MHz
		ClockControl::setApb1Prescaler(ClockControl::Apb1Prescaler::Div2);

		// APB2 has max. 72MHz
		ClockControl::setApb2Prescaler(ClockControl::Apb2Prescaler::Div1);

		// update frequencies for busy-wait delay functions
		xpcc::clock::fcpu     = Frequency;
		xpcc::clock::fcpu_kHz = Frequency / 1000;
		xpcc::clock::fcpu_MHz = Frequency / 1000000;
		xpcc::clock::ns_per_loop = ::round(3000.f / (Frequency / 1000000));

		return true;
	}
};

// Not using SystemClock as it is not yet finished for F1 series.
// But this works by accident.
// using systemClock = SystemClock<Pll<ExternalCrystal<MHz8>, MHz72> >;

// User LED (inverted, because connected to 3V3)
// using LedGreen = xpcc::GpioInverted< GpioOutputC13 >;
// using Leds = xpcc::SoftwareGpioPort< LedGreen >;

using Button = xpcc::GpioUnused;

inline void
initialize()
{
	systemClock::enable();
	xpcc::cortex::SysTickTimer::initialize<systemClock>();

	// LedGreen::setOutput(xpcc::Gpio::Low);
}

} // Board namespace

#endif	// XPCC_STM32_F103C8T6_BLUE_PILL_HPP
