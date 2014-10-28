#include "tcpip_receiver.hpp"
#include <xpcc/debug/logger.hpp>
#include <xpcc/architecture/platform/hosted/tcpip/tcpip_client.hpp>

#include <iostream>

// set new log level
#undef XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

xpcc::tcpip::Receiver::Receiver(xpcc::tcpip::Client* parent, int componentId):
	parent(parent), componentId(componentId),
	endpoint(boost::asio::ip::tcp::v4(), parent->getServerPort() + 1 + componentId),
	acceptor(*(parent->getIOService()), endpoint),
	socket(*(parent->getIOService())),
	connected(false),
	shutdown(false)
{
	XPCC_LOG_DEBUG<< "start receiver for component: "<<componentId<<" on port "<< parent->getServerPort() + 1 + componentId
			<< xpcc::endl;
	this->acceptor.async_accept(socket,
			 boost::bind(&xpcc::tcpip::Receiver::acceptHandler, this,
					 boost::asio::placeholders::error));
}


/*
void
xpcc::tcpip::Receiver::run()
{
	if(this->publishMessage())
	{
		XPCC_LOG_INFO<< "component "<<componentId << received Message <<xpcc::endl;
	}
}*/


void
xpcc::tcpip::Receiver::readHeader()
{

    boost::asio::async_read(socket,
        boost::asio::buffer(this->header, xpcc::tcpip::TCPHeader::headerSize()),
        boost::bind(
          &xpcc::tcpip::Receiver::readHeaderHandler, this,
          boost::asio::placeholders::error));
}

void
xpcc::tcpip::Receiver::readMessage(const xpcc::tcpip::TCPHeader& header)
{

	int dataSize = header.getDataSize();

    boost::asio::async_read(socket,
        boost::asio::buffer(this->message, dataSize),
        boost::bind(
          &xpcc::tcpip::Receiver::readMessageHandler, this,
          boost::asio::placeholders::error));

}


void
xpcc::tcpip::Receiver::acceptHandler(
		const boost::system::error_code& error)
{
	XPCC_LOG_DEBUG << "Component " << this->componentId << ": connection established" << xpcc::endl;

    if (!error)
    {
    	//this->readHeader();
		boost::lock_guard<boost::mutex> lock(this->connectedMutex);
    	this->connected = true;
    }
    // TODO error handling
}

void
xpcc::tcpip::Receiver::run()
{
	bool connection = false;

	while(!connection)
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
		{
			boost::lock_guard<boost::mutex> lock(this->connectedMutex);
			connection=this->connected;
		}
	}

	this->readHeader();
}

void
xpcc::tcpip::Receiver::readHeaderHandler(const boost::system::error_code& error)
{
	if(!error)
	{
		xpcc::tcpip::TCPHeader* messageHeader = reinterpret_cast<xpcc::tcpip::TCPHeader*>(this->header);
		this->readMessage(*messageHeader);
	}
	else{
		//TODO error handling
	}
}

void
xpcc::tcpip::Receiver::readMessageHandler(const boost::system::error_code& error)
{
	if(!error)
	{
		xpcc::tcpip::TCPHeader* messageHeader = reinterpret_cast<xpcc::tcpip::TCPHeader*>(this->header);
		SmartPointer payload(messageHeader->getDataSize());
		memcpy(payload.getPointer(), this->message, messageHeader->getDataSize());

		boost::shared_ptr<xpcc::tcpip::Message> message( new xpcc::tcpip::Message(messageHeader->getXpccHeader(), payload));
		this->parent->receiveNewMessage(message);
		if(!this->shutdown)
		{
			this->readHeader();
		}
	}
	else{
		//TODO error handling
	}
}
