#include "../stm32f429_nucleo144.hpp"
#include <xpcc/debug/logger.hpp>

// ----------------------------------------------------------------------------
// Set the log level
#undef	XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::INFO

// Create an IODeviceWrapper around the Uart Peripheral we want to use
xpcc::IODeviceWrapper< Usart3, xpcc::IOBuffer::BlockIfFull > loggerDevice;

// Set all four logger streams to use the UART
xpcc::log::Logger xpcc::log::debug(loggerDevice);
xpcc::log::Logger xpcc::log::info(loggerDevice);
xpcc::log::Logger xpcc::log::warning(loggerDevice);
xpcc::log::Logger xpcc::log::error(loggerDevice);

// ----------------------------------------------------------------------------
MAIN_FUNCTION
{
	Board::initialize();

	// initialize Uart3 for XPCC_LOG_
	GpioOutputD8::connect(Usart3::Tx);
	GpioInputD9::connect(Usart3::Rx, Gpio::InputType::PullUp);
	Usart3::initialize<Board::systemClock, 115200>(12);

	Board::LedBlue::set();

	// Use the logging streams to print some messages.
	// Change XPCC_LOG_LEVEL above to enable or disable these messages
	XPCC_LOG_DEBUG   << "debug"   << xpcc::endl;
	XPCC_LOG_INFO    << "info"    << xpcc::endl;
	XPCC_LOG_WARNING << "warning" << xpcc::endl;
	XPCC_LOG_ERROR   << "error"   << xpcc::endl;

	uint32_t loop_counter = 0;

	while (true)
	{
		Board::LedRed::toggle();
		Board::LedGreen::toggle();
		Board::LedBlue::toggle();

		xpcc::delayMilliseconds(Board::Button::read() ? 125 : 500);

		XPCC_LOG_INFO.printf("Loop cycle %d\n", loop_counter);
		++loop_counter;
	}

	return 0;
}
