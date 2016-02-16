#include "../../stm32f429_nucleo144.hpp"
#include <xpcc/debug/logger.hpp>
#include <xpcc/processing/timer.hpp>
#include <xpcc/driver/rtc/ds1302/ds1302.hpp>

// ----------------------------------------------------------------------------
// Set the log level
#undef	XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

// Create an IODeviceWrapper around the Uart Peripheral we want to use
xpcc::IODeviceWrapper< Usart3, xpcc::IOBuffer::BlockIfFull > loggerDevice;

// Set all four logger streams to use the UART
xpcc::log::Logger xpcc::log::debug(loggerDevice);
xpcc::log::Logger xpcc::log::info(loggerDevice);
xpcc::log::Logger xpcc::log::warning(loggerDevice);
xpcc::log::Logger xpcc::log::error(loggerDevice);

struct ds1302_pin_set {
	using Ce = GpioOutputC0;
	using Sclk = GpioOutputA3;
	using Io = GpioC3;
};
using ds1302 = xpcc::Ds1302< ds1302_pin_set >;

// ----------------------------------------------------------------------------
/**
 * Realtime Clock RTC DS1302 test.
 *
 * This example compares the internal oscillator of the STM with an external
 * RTC.
 *
 * The expected result should be:
 * \code
 * Welcome to RTC DS1302 demo.
 * CPU frequency is 168000000 Hz
 * .........  9.995 seconds by RTC are:  9
 * ..........  9.996 seconds by RTC are:  9
 * ..........  9.997 seconds by RTC are:  9
 * ..........  9.998 seconds by RTC are:  9
 * ..........  9.999 seconds by RTC are:  9
 * .......... 10.000 seconds by RTC are: 10
 * ..........  9.999 seconds by RTC are:  9
 * .......... 10.000 seconds by RTC are: 10
 * /endcode
 */
MAIN_FUNCTION
{
	Board::initialize();

	// initialize Uart3 for XPCC_LOG_
	GpioOutputD8::connect(Usart3::Tx);
	GpioInputD9::connect(Usart3::Rx, Gpio::InputType::PullUp);
	Usart3::initialize<Board::systemClock, 115200>(12);

	XPCC_LOG_INFO << xpcc::endl;
	XPCC_LOG_INFO << "Welcome to RTC DS1302 demo." << xpcc::endl;

	Board::LedBlue::set();

	ds1302_pin_set::Ce::setOutput();
	ds1302_pin_set::Sclk::setOutput();
	ds1302::initialize();

	// Disable write protect
	ds1302::writeProtect(false);

	// Enable RTC oscillator
	// Side effect: set seconds to 0
	ds1302::enableOscillator();

	uint16_t tt = 9995;
	xpcc::Timeout timeout;
	timeout.restart(tt);

	// Periodically report progress
	xpcc::PeriodicTimer pt(1000);

	XPCC_LOG_DEBUG.printf("CPU frequency is %ld Hz\n", Board::systemClock::Frequency);

	while(true)
	{
		if (pt.execute()) {
			XPCC_LOG_DEBUG << ".";
			Board::LedBlue::toggle();
		}
		if (timeout.execute())
		{
			xpcc::ds1302::Data rtc_data;
			ds1302::readRtc(rtc_data);
			uint8_t seconds = rtc_data.getSeconds();

			XPCC_LOG_DEBUG.printf(" %2d.%03d seconds by RTC are: %2d\n",
				tt / 1000,
				tt % 1000,
				seconds);

			// Set seconds to 0
			ds1302::write(0x80, 0x00);
			if (seconds >= 10) {
				--tt;
			} else {
				++tt;
			}
			timeout.restart(tt);
		}
	};
	return 0;
}
