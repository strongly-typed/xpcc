#include <xpcc/architecture.hpp>
#include <xpcc/debug/logger.hpp>
#include "../../mchck.hpp"

class DummyClock
{
public:
	static constexpr int Uart0 = 21 * 1000 * 1000 / 2;
};

// Create an IODeviceWrapper around the Uart Peripheral we want to use
xpcc::IODeviceWrapper< Uart0 > loggerDevice;

// Set all four logger streams to use the UART
xpcc::log::Logger xpcc::log::debug(loggerDevice);
xpcc::log::Logger xpcc::log::info(loggerDevice);
xpcc::log::Logger xpcc::log::warning(loggerDevice);
xpcc::log::Logger xpcc::log::error(loggerDevice);

// Set the log level
#undef	XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG


// ----------------------------------------------------------------------------
MAIN_FUNCTION
{
	//StartupError err =
	//	SystemClock<Pll<ExternalClock<MHz8>, MHz48>>::enable();

	// Initialize Usart
	PL12Output::connect(Uart0::Tx);
	Uart0::initialize<DummyClock, 115200>();

	// Use the logging streams to print some messages.
	// Change XPCC_LOG_LEVEL above to enable or disable these messages 
	XPCC_LOG_DEBUG   << "debug"   << xpcc::endl;
	XPCC_LOG_INFO    << "info"    << xpcc::endl;
	XPCC_LOG_WARNING << "warning" << xpcc::endl;
	XPCC_LOG_ERROR   << "error"   << xpcc::endl;

	while (1)
	{
	}

	return 0;
}
