#include "tcpip_connection.hpp"
#include "tcpip_server.hpp"

#include <xpcc/container/smart_pointer.hpp>

xpcc::tcpip::Connection::Connection(boost::shared_ptr<boost::asio::io_service> ioService, xpcc::tcpip::Server* server):
    socket(*ioService),
    server(server),
    listen(false)
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

    	xpcc::tcpip::TCPHeader::Type type = header->getMessageType();

    	switch(type){

    	case xpcc::tcpip::TCPHeader::Type::DATA:{
    		SmartPointer payload(header->getDataSize());
    		memcpy(payload.getPointer(), this->message, header->getDataSize());
    		xpcc::tcpip::Message msg(header->getXpccHeader(), payload);

    		//evaluating data only required if Message is a data message
    		this->server->distributeMessage(msg);
    		break;
    	}

    	case xpcc::tcpip::TCPHeader::Type::REGISTER:{
    		//message is a register message, spawn a new distributor thread
    		std::string ip = socket.remote_endpoint().address().to_string();
    		int componentId = header->getXpccHeader().source;
    		XPCC_LOG_DEBUG<<"Spawning sender for componentId "<< componentId << xpcc::endl;
    		this->server->spawnSendThread(componentId, ip);
    		break;
    	}

    	case xpcc::tcpip::TCPHeader::Type::UNREGISTER:{
    		int id = header->getXpccHeader().source;
    		this->server->shutdownDistributor(id);
    		SmartPointer payload(header->getDataSize());
    		header->getXpccHeader().destination = id;
    		header->getXpccHeader().source = id;
    		std::cout <<"Header size: "<< (int) header->getDataSize() << std::endl;
    		memcpy(payload.getPointer(), this->message, header->getDataSize());
    		boost::shared_ptr<xpcc::tcpip::Message> msg(new xpcc::tcpip::Message(xpcc::tcpip::TCPHeader::Type::UNREGISTER,
    				header->getXpccHeader(), payload));
    		std::cout<< "Confirming unregister message"<<std::endl;
    		this->sendMessage(msg);
    		break;
    	}
    	case xpcc::tcpip::TCPHeader::Type::CLOSE_CONNECTION:{
    		//TODO shutdown the connection;
    		boost::system::error_code ec;
    		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    		if (ec)
    		{
    		  std::cout << "Receive connection shutdown with error code "<< ec << std::endl;
    		}

    		socket.close(ec);
    		if (ec)
    		{
    			std::cout << "Receive Connection closed with error code "<< ec << std::endl;
    		}

    		std::cout << "Connection closed!" << std::endl;
    		this->server->removeClosedConnections();

    		break;
    	}
    	case xpcc::tcpip::TCPHeader::Type::LISTEN:{
    		//TODO send bool with listen messages in order to close listening connections.
    		this->listen = true;
    		break;
    	}
    	default:{
    		//TODO handle other cases;
    			break;
    		}
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

bool
xpcc::tcpip::Connection::isListening(){
	return this->listen;
}


void
xpcc::tcpip::Connection::sendMessage(boost::shared_ptr<xpcc::tcpip::Message> message){
	bool writingMessages = !messagesToBeSent.empty();
	messagesToBeSent.push_back(message);
    if (!writingMessages)
    {

    	messagesToBeSent.front()->encodeMessage();
    	boost::asio::async_write(socket,
    			boost::asio::buffer(messagesToBeSent.front()->getEncodedMessage(),
    					messagesToBeSent.front()->getMessageLength()),
    					boost::bind(&xpcc::tcpip::Connection::handleSendMessage, this,
    							boost::asio::placeholders::error));
    }
}

void
xpcc::tcpip::Connection::handleSendMessage(const boost::system::error_code& error){
    if (!error)
    {
    	//Remove sent message
    	messagesToBeSent.pop_front();
    	if (!messagesToBeSent.empty())
    	{
    		//Prepare next message
    		messagesToBeSent.front()->encodeMessage();
    		boost::asio::async_write(socket,
    		        boost::asio::buffer(messagesToBeSent.front()->getEncodedMessage(),
    		        messagesToBeSent.front()->getMessageLength()),
    				boost::bind(&xpcc::tcpip::Connection::handleSendMessage, this,
    				boost::asio::placeholders::error));
      }
    }
    else{
    	//TODO ERROR handler
    }
}

bool
xpcc::tcpip::Connection::isConnected(){
	return this->socket.is_open();
}
