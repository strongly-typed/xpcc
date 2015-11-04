
#include <xpcc/architecture.hpp>
#include <xpcc/communication.hpp>
#include <xpcc/communication/xpcc/backend/ethernet.hpp>
#include <xpcc/debug/logger.hpp>

#include "../../stm32f4_discovery.hpp"
#include "../common/enc28j60_spi2.hpp"

#include <unistd.h>

// set new log level
#undef XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

#include "component_receiver/receiver.hpp"

#include "communication/postman.hpp"
#include "communication/identifier.hpp"

// ----------------------------------------------------------------------------
// Logging

xpcc::IODeviceWrapper< xpcc::stm32::Usart2, xpcc::IOBuffer::BlockIfFull > loggerDevice;
xpcc::log::Logger xpcc::log::debug(loggerDevice);
xpcc::log::Logger xpcc::log::info(loggerDevice);

// Set the log level
#undef  XPCC_LOG_LEVEL
#define XPCC_LOG_LEVEL xpcc::log::DEBUG

static enc28j60::EthernetDevice ethernetDevice;
static xpcc::EthernetConnector< enc28j60::EthernetDevice > ethernetConnector(&ethernetDevice);

// create an instance of the generated postman
Postman ethernetPostman;
xpcc::Dispatcher ethernetDispatcher(&ethernetConnector, &ethernetPostman);

namespace component
{
	Receiver receiver(robot::component::RECEIVER, &ethernetDispatcher);
}

// ----------------------------------------------------------------------------
MAIN_FUNCTION
{
    Board::systemClock::enable();
    xpcc::cortex::SysTickTimer::initialize<Board::systemClock>();

    // UART
    xpcc::stm32::GpioA2::connect(xpcc::stm32::Usart2::Tx);
    xpcc::stm32::GpioA3::connect(xpcc::stm32::Usart2::Rx, Gpio::InputType::PullUp);
    xpcc::stm32::Usart2::initialize<Board::systemClock, 115200>(12);

    // Setup GPIO
	Board::LedOrange::setOutput(xpcc::Gpio::Low);
	Board::LedGreen::setOutput(xpcc::Gpio::Low);
	Board::LedRed::setOutput(xpcc::Gpio::High);
	Board::LedBlue::setOutput(xpcc::Gpio::High);

    initEnc28J60();

	XPCC_LOG_INFO << "Welcome to the XPCC over Ethernet communication test!" << xpcc::endl;

	while (1)
	{
		// deliver received messages
		ethernetDispatcher.update();

		component::receiver.update();

		xpcc::delayMicroseconds(100);
	}
}
