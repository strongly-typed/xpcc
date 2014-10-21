#include "tcpip_connection.hpp"
#include "tcpip_server.hpp"

#include <xpcc/container/smart_pointer.hpp>

xpcc::tcpip::Connection::Connection(boost::shared_ptr<boost::asio::io_service> ioService, xpcc::tcpip::Server* server):
    socket(*ioService),
    server(server)
{

}


boost::asio::ip::tcp::socket&
xpcc::tcpip::Connection::getSocket()
{
	return this->socket;
}


void
xpcc::tcpip::Connection::start()
{

    boost::asio::async_read(socket,
        boost::asio::buffer(this->header, xpcc::tcpip::TCPHeader::headerSize()),
        boost::bind(
          &xpcc::tcpip::Connection::handleReadHeader, this,
          boost::asio::placeholders::error));
}


void
xpcc::tcpip::Connection::handleReadHeader(const boost::system::error_code& error)
{

	if(! error){
		xpcc::tcpip::TCPHeader* header = reinterpret_cast<xpcc::tcpip::TCPHeader*>(this->header);
	    boost::asio::async_read(socket,
	        boost::asio::buffer(this->message, header->getDataSize()),
	        boost::bind(
	          &xpcc::tcpip::Connection::handleReadBody, this,
	          boost::asio::placeholders::error));
	}
}

void
xpcc::tcpip::Connection::handleReadBody(const boost::system::error_code& error)
{

    if(! error){

    	xpcc::tcpip::TCPHeader* header = reinterpret_cast<xpcc::tcpip::TCPHeader*>(this->header);

    	if(header->isDataMessage())
    	{
    		SmartPointer payload(header->getDataSize());
    		memcpy(payload.getPointer(), this->message, header->getDataSize());
    		xpcc::tcpip::Message msg(header->getXpccHeader(), payload);

    		//evaluating data only required if Message is a data message
    		this->server->distributeDataMessage(msg);
    	}
    	else
    	{
    		//message is a register message, spawn a new distributor thread
    		std::string ip = socket.remote_endpoint().address().to_string();
    		int componentId = header->getXpccHeader().source;
    		XPCC_LOG_DEBUG<<"Spawning sender for componentId "<< componentId << xpcc::endl;
    		this->server->spawnSendThread(componentId, ip);
    	}

        boost::asio::async_read(socket,
            boost::asio::buffer(this->header, xpcc::tcpip::TCPHeader::headerSize()),
            boost::bind(
              &xpcc::tcpip::Connection::handleReadHeader, this,
              boost::asio::placeholders::error));
    }
    else{
    	//TODO error handling
    }
}
