#include <xpcc/architecture/driver/delay.hpp>
#include <xpcc/communication.hpp>
#include <xpcc_config.hpp>


#ifdef USE_TCPIP
#include <xpcc/communication/xpcc/backend/tcpip.hpp>
xpcc::TcpIpConnector connector;
#endif

#ifdef USE_TIPC
#include <xpcc/communication/xpcc/backend/tipc.hpp>
xpcc::TipcConnector connector;
#endif

#include <xpcc/debug/logger.hpp>

// set new log level
#undef XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

#include "component_sender/sender.hpp"

#include "../communication/postman.hpp"
#include "../communication/identifier.hpp"



// create an instance of the generated postman
Postman postman;

xpcc::Dispatcher dispatcher(&connector, &postman);


namespace component
{
	Sender sender(robot::component::SENDER, &dispatcher);
}

int
main(void)
{

     XPCC_LOG_INFO << "Welcome to the network communication test!" << xpcc::endl;

#ifdef USE_TCPIP
    bool connected = connector.connect("127.0.0.1", 7666);
    if(!connected){
		XPCC_LOG_ERROR << 
			"Unable to establish connection to server (127.0.0.1:7666)." << xpcc::endl;
			return 1;
	}
#endif

	connector.addReceiverId(robot::component::SENDER);
	
    XPCC_LOG_INFO << "Component Sender"<< xpcc::endl;
	
	while (1)
	{
		// deliver received messages
		dispatcher.update();
		
		component::sender.update();
		
		xpcc::delayMicroseconds(100);
	}
}
