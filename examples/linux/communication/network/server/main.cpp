#include <xpcc_config.hpp>

#include <xpcc/architecture/platform/driver/tcpip/tcpip_server.hpp>
#include <xpcc/communication/xpcc/backend/tcpip/tcpip.hpp>
#include <xpcc/debug/logger.hpp>

// set new log level
#undef XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

int
main(void)
{
	XPCC_LOG_INFO << "Welcome to the communication test!" << xpcc::endl; 
	XPCC_LOG_INFO << "TCP/IP server"<< xpcc::endl;
    xpcc::tcpip::Server server(7666);
	while(true){
		//prevent process from ending
	}
}
