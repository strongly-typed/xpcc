
#include <xpcc/architecture.hpp>

#include <xpcc/communication.hpp>
// #include <xpcc/communication/xpcc/backend/tipc.hpp>
// #include <xpcc/communication/xpcc/backend/zeromq.hpp>
#include <xpcc/communication/xpcc/backend/ethernet.hpp>

#include <pcap.h>

#include <xpcc/debug/logger.hpp>

// set new log level
#undef XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

#include "component_receiver/receiver.hpp"
#include "component_sender/sender.hpp"

#include "communication/postman.hpp"
#include "communication/identifier.hpp"

// Use TIPC on Linux only
// xpcc::TipcConnector connector;

// Use ZeroMQ on Linux and Darwin
// const std::string endpointIn  = "tcp://127.0.0.1:8211";
// const std::string endpointOut = "tcp://127.0.0.1:8212";
// static xpcc::ZeroMQConnector connector(endpointIn, endpointOut, xpcc::ZeroMQConnector::Mode::SubPush);

// Use raw Ethernet frames on Linux
static uint8_t
containerLut(const xpcc::Header& header)
{
  robot::container::Identifier ret = robot::container::Identifier::Gui;

  switch (header.destination)
  {
    case robot::component::Identifier::SENDER:
      ret = robot::container::Identifier::Sender;
      break;
    case robot::component::Identifier::RECEIVER:
      ret = robot::container::Identifier::Receiver;
      break;
  }

  return static_cast<uint8_t>(ret);

}

namespace xpcc {
typedef uint8_t EthernetFrame[64];

class EthernetDevice
{
public:
  EthernetDevice()
  {
  	handle = pcap_open_live("en4", 65536, 1, 25, errbuf);
  	if (handle == NULL) {
  		XPCC_LOG_ERROR.printf("Open failed\n");
  	}
  }

  ~EthernetDevice()
  {
  	if (handle != nullptr)
  	{
  		pcap_close(handle);
  	}
  }

  static bool
  sendMessage(EthernetFrame &frame)
  {
  	pcap_sendpacket(handle, frame, 64);
  	return true;
  }

  static bool
  getMessage(uint8_t &length, EthernetFrame &frame)
  {
  	// XPCC_LOG_DEBUG.printf("getMessage \n");
  	struct pcap_pkthdr header;
  	const uint8_t *packet;
  	packet = pcap_next(handle, &header);

  	// XPCC_LOG_DEBUG.printf("length = %d\n", header.len);
  	if (header.len == 64)
  	{
  		if (packet != nullptr)
  		{
			length = 64;
			memcpy(frame, packet, 64);
			return true;
  		}
  	}
  	return false;
  }

private:
	static char errbuf[PCAP_ERRBUF_SIZE];
	static pcap_t *handle;
};

char EthernetDevice::errbuf[];
pcap_t *EthernetDevice::handle;

} // xpcc namespace

static xpcc::EthernetDevice ethDevice;
static xpcc::EthernetConnector< xpcc::EthernetDevice > connector(ethDevice, containerLut);


// create an instance of the generated postman
Postman postman;

xpcc::Dispatcher dispatcher(&connector, &postman);

namespace component
{
	Sender sender(robot::component::SENDER, dispatcher);
	Receiver receiver(robot::component::RECEIVER, dispatcher);
}

int
main()
{
	// Required for TIPC on Linux only
	// connector.addReceiverId(robot::component::SENDER);
	// connector.addReceiverId(robot::component::RECEIVER);
	
	XPCC_LOG_INFO << "Welcome to the communication test!" << xpcc::endl; 
	
	while (true)
	{
		// deliver received messages
		dispatcher.update();
		
		component::receiver.update();
		// component::sender.update();
		
		xpcc::delayMicroseconds(100);
	}
}
