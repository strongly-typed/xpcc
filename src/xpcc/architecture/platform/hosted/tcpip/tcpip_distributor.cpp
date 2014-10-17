#include "tcpip_distributor.hpp"
#include <xpcc/architecture/platform/hosted/tcpip/tcpip_server.hpp>
#include <sstream>

xpcc::tcpip::Distributor::Distributor(xpcc::tcpip::Server* parent, std::string ip, uint8_t component_id):
	connected(false),
	writingMessages(false),
	ioService(parent->getIoService()),
	server(parent),
	port(server->getPort()+1+component_id)
{
	XPCC_LOG_DEBUG << "Distributor for "<< port<< " started"<<xpcc::endl;
	boost::asio::ip::tcp::resolver resolver(*ioService);
	//port required as string
	std::stringstream portStream;
	portStream<< port;
	boost::asio::ip::tcp::resolver::query query(ip, portStream.str());
	this->endpointIter = resolver.resolve(query);
	this->sendSocket.reset(new boost::asio::ip::tcp::socket(*ioService));

	boost::asio::async_connect(*sendSocket, endpointIter,
        boost::bind(&xpcc::tcpip::Distributor::connectHandler, this,
          boost::asio::placeholders::error));
}

void
xpcc::tcpip::Distributor::run()
{

}

void
xpcc::tcpip::Distributor::disconnect()
{

}

void
xpcc::tcpip::Distributor::sendMessage(boost::shared_ptr<xpcc::tcpip::Message> msg)
{
	boost::lock_guard<boost::mutex> lock(sendMutex);
	writingMessages = !messagesToBeSent.empty();
	messagesToBeSent.push_back(msg);
    if (!writingMessages)
    {
    	messagesToBeSent.front()->encodeMessage();
    	boost::asio::async_write(*sendSocket,
    			boost::asio::buffer(messagesToBeSent.front()->getEncodedMessage(),
    					messagesToBeSent.front()->getMessageLength()),
    					boost::bind(&xpcc::tcpip::Distributor::sendHandler, this,
    							boost::asio::placeholders::error));
    }
}

void
xpcc::tcpip::Distributor::connectHandler(const boost::system::error_code& error)
{
	//XPCC_LOG_DEBUG << "Distributor connected with error: "<< error << xpcc::endl;
}

void
xpcc::tcpip::Distributor::sendHandler(const boost::system::error_code& error)
{
    if (!error)
    {
    	//Remove sent message
    	boost::lock_guard<boost::mutex> lock(sendMutex);
    	messagesToBeSent.pop_front();
    	if (!messagesToBeSent.empty())
    	{
    		//Prepare next message
    		messagesToBeSent.front()->encodeMessage();
    		boost::asio::async_write(*sendSocket,
    		        boost::asio::buffer(messagesToBeSent.front()->getEncodedMessage(),
    		        messagesToBeSent.front()->getMessageLength()),
    				boost::bind(&xpcc::tcpip::Distributor::sendHandler, this,
    				boost::asio::placeholders::error));
      }
    }
    else{
    	//TODO ERROR handler
    }
}

//TODO
void
xpcc::tcpip::Distributor::close()
{

}
