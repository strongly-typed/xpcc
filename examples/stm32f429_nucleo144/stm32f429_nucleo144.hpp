// coding: utf-8
/* Copyright (c) 2016, strongly-typed
* All Rights Reserved.
*
* The file is part of the xpcc library and is released under the 3-clause BSD
* license. See the file `LICENSE` for the full license governing this code.
*/
// ----------------------------------------------------------------------------

//
// STM32F429INUCLEO144
// Nucleo for STM32F429 line
// http://www.st.com/web/catalog/tools/FM116/SC959/SS1532/LN1847/PF262637
//

#ifndef XPCC_STM32_F429_NUCLEO144_HPP
#define XPCC_STM32_F429_NUCLEO144_HPP

#include <xpcc/architecture/platform.hpp>

using namespace xpcc::stm32;

namespace Board
{

/// STM32F429 running at 168MHz (USB Clock qt 48MHz) generated from the
/// external 8 MHz MCO clock from ST-Link
typedef SystemClock<Pll<ExternalClock<MHz8>, MHz168, MHz48> > systemClock;

// User Button
typedef GpioInputC13 Button;

// User LEDs
using LedGreen = GpioOutputB0;	// User LED (LD1)
using LedBlue  = GpioOutputB7;	// User LED (LD2)
using LedRed   = GpioOutputB14;	// User LED (LD3)


namespace usb
{
typedef GpioOutputA11	Dm;		// OTG_FS_DM: USB_OTG_HS_DM
typedef GpioOutputA12	Dp;		// OTG_FS_DP: USB_OTG_HS_DP
typedef GpioOutputA10	Id;		// OTG_FS_ID: USB_OTG_HS_ID

typedef GpioInputG7		Overcurrent;	// OTG_FS_OC [OTG_FS_OverCurrent]: GPXTI5
typedef GpioOutputG6	Power;			// OTG_FS_PSO [OTG_FS_PowerSwitchOn]
typedef GpioInputA9		VBus;			// VBUS_FS: USB_OTG_HS_VBUS
//typedef UsbFs Device;
}


inline void
initialize()
{
	systemClock::enable();
	xpcc::cortex::SysTickTimer::initialize<systemClock>();

	LedGreen::setOutput(xpcc::Gpio::Low);
	LedRed::setOutput(xpcc::Gpio::Low);
	LedBlue::setOutput(xpcc::Gpio::High);

	Button::setInput();
	Button::setInputTrigger(Gpio::InputTrigger::RisingEdge);
	Button::enableExternalInterrupt();
//	Button::enableExternalInterruptVector(12);
}

}

#endif	// XPCC_STM32_F429_NUCLEO144_HPP
