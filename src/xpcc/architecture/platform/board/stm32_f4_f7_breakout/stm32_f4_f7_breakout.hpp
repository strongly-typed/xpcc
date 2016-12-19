// coding: utf-8
/* Copyright (c) 2016, Roboterclub Aachen e. V.
* All Rights Reserved.
*
* The file is part of the xpcc library and is released under the 3-clause BSD
* license. See the file `LICENSE` for the full license governing this code.
*/
// ----------------------------------------------------------------------------

//
// STM32 F4 F7 Breakout
//

#ifndef XPCC_STM32_F4_F7_BREAKOUT_HPP
#define XPCC_STM32_F4_F7_BREAKOUT_HPP

#include <xpcc/architecture/platform.hpp>

using namespace xpcc::stm32;


namespace Board
{

/* SystemClock generator is only available for selected STM32F4 devices.
 * The idea is that it is generated automatically for you like the rest of the
 * HAL, however, xpcc does not have this capability yet. See PR #36.
 */
// using systemClock = SystemClock<Pll<ExternalCrystal<MHz8>, MHz168, MHz48> >;

// Instead this manual implementation of the system clock is used:
/// STM32F407 running at 168MHz generated from the external 8MHz crystal
struct systemClock {
	static constexpr uint32_t Frequency = 168 * MHz1;
	static constexpr uint32_t Ahb = Frequency;
	static constexpr uint32_t Apb1 = Frequency / 4;
	static constexpr uint32_t Apb2 = Frequency / 2;

	static constexpr uint32_t Adc = Apb2;

	static constexpr uint32_t Can1   = Apb1;
	static constexpr uint32_t Can2   = Apb1;

	static constexpr uint32_t Spi1   = Apb2;
	static constexpr uint32_t Spi2   = Apb1;
	static constexpr uint32_t Spi3   = Apb1;
	static constexpr uint32_t Spi4   = Apb2;
	static constexpr uint32_t Spi5   = Apb2;
	static constexpr uint32_t Spi6   = Apb2;

	static constexpr uint32_t Usart1 = Apb2;
	static constexpr uint32_t Usart2 = Apb1;
	static constexpr uint32_t Usart3 = Apb1;
	static constexpr uint32_t Uart4  = Apb1;
	static constexpr uint32_t Uart5  = Apb1;
	static constexpr uint32_t Usart6 = Apb2;
	static constexpr uint32_t Uart7  = Apb1;
	static constexpr uint32_t Uart8  = Apb1;

	static constexpr uint32_t I2c1   = Apb1;
	static constexpr uint32_t I2c2   = Apb1;
	static constexpr uint32_t I2c3   = Apb1;

	static constexpr uint32_t Apb1Timer = Apb1 * 2;
	static constexpr uint32_t Apb2Timer = Apb2 * 2;
	static constexpr uint32_t Timer1  = Apb2Timer;
	static constexpr uint32_t Timer2  = Apb1Timer;
	static constexpr uint32_t Timer3  = Apb1Timer;
	static constexpr uint32_t Timer4  = Apb1Timer;
	static constexpr uint32_t Timer5  = Apb1Timer;
	static constexpr uint32_t Timer6  = Apb1Timer;
	static constexpr uint32_t Timer7  = Apb1Timer;
	static constexpr uint32_t Timer8  = Apb2Timer;
	static constexpr uint32_t Timer9  = Apb2Timer;
	static constexpr uint32_t Timer10 = Apb2Timer;
	static constexpr uint32_t Timer11 = Apb2Timer;
	static constexpr uint32_t Timer12 = Apb1Timer;
	static constexpr uint32_t Timer13 = Apb1Timer;
	static constexpr uint32_t Timer14 = Apb1Timer;

	static bool inline
	enable()
	{
		ClockControl::enableExternalCrystal();	// 8MHz
		ClockControl::enablePll(
			ClockControl::PllSource::ExternalCrystal,
			4,		// 8MHz / N=2 -> 2MHz
			168,	// 2MHz * M=168 -> 336MHz
			2,		// 336MHz / P=2 -> 168MHz = F_cpu
			7		// 336MHz / Q=7 -> 48MHz = F_usb
		);
		// set flash latency for 168MHz
		ClockControl::setFlashLatency(Frequency);
		// switch system clock to PLL output
		ClockControl::enableSystemClock(ClockControl::SystemClockSource::Pll);
		ClockControl::setAhbPrescaler(ClockControl::AhbPrescaler::Div1);
		// APB1 has max. 42MHz
		// APB2 has max. 84MHz
		ClockControl::setApb1Prescaler(ClockControl::Apb1Prescaler::Div4);
		ClockControl::setApb2Prescaler(ClockControl::Apb2Prescaler::Div2);
		// update frequencies for busy-wait delay functions
		xpcc::clock::fcpu     = Frequency;
		xpcc::clock::fcpu_kHz = Frequency / 1000;
		xpcc::clock::fcpu_MHz = Frequency / 1000000;
		xpcc::clock::ns_per_loop = ::round(3000 / (Frequency / 1000000));

		return true;
	}
};

using ButtonInv = GpioInputE9;

using Button = xpcc::GpioInverted< ButtonInv >;

using LedRed    = GpioOutputE7; // LED1
using LedGreen  = GpioOutputE8; // LED3

using Leds = xpcc::SoftwareGpioPort< LedRed, LedGreen >;


namespace usb
{
using Dm = GpioA11;			// OTG_FS_DM: USB_OTG_FS_DM
using Dp = GpioA12;			// OTG_FS_DP: USB_OTG_FS_DP
using Id = GpioA10;			// OTG_FS_ID: USB_OTG_FS_ID

// using Overcurrent = GpioD5;	// OTG_FS_OverCurrent
// using Power = GpioOutputC0;	// OTG_FS_PowerSwitchOn
// using VBus = GpioInputA9;	// VBUS_FS: USB_OTG_HS_VBUS
// using Device = UsbFs;
}


inline void
initialize()
{
	systemClock::enable();
	xpcc::cortex::SysTickTimer::initialize<systemClock>();

	LedGreen::setOutput(xpcc::Gpio::Low);
	LedRed::setOutput(xpcc::Gpio::Low);

	// Enable USART 2
	GpioOutputA9::connect(Usart1::Tx);
	GpioInputA10::connect(Usart1::Rx, Gpio::InputType::PullUp);
	Usart1::initialize<Board::systemClock, 115200>(12);

	Button::setInput();
	Button::setInputTrigger(Gpio::InputTrigger::RisingEdge);
	Button::enableExternalInterrupt();
//	Button::enableExternalInterruptVector(12);
}

/// not supported yet, due to missing USB driver
inline void
initializeUsb()
{
//	usb::Dm::connect(usb::Device::Dm);
//	usb::Dp::connect(usb::Device::Dp);
//	usb::Id::connect(usb::Device::Id);
}

}

#endif	// XPCC_STM32_F4_F7_BREAKOUT_HPP
