#include "tcpip_client.hpp"

#include <iostream>
#include <sstream>


#include <boost/thread/locks.hpp>

xpcc::tcpip::Client::Client(/*std::string ip, int port*/):
	connected(false),
	writingMessages(false),
	closeConnection(false),
	ioService(new boost::asio::io_service()),
	work(new boost::asio::io_service::work(*ioService)),
	ioThread(boost::bind(&boost::asio::io_service::run, ioService)),
	serverPort(-1)
{
}

void
xpcc::tcpip::Client::connect(std::string ip, int port){
	if(!this->isConnected()){
		this->serverPort = port;
		boost::asio::ip::tcp::resolver resolver(*ioService);
		//port required as string
		std::stringstream portStream << port;
		boost::asio::ip::tcp::resolver::query query(ip, portStream.str());
		this->endpointIter = resolver.resolve(query);

		//this->serviceThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&boost::asio::io_service::run, ioService)));

		this->sendSocket.reset(new boost::asio::ip::tcp::socket(*ioService));

		boost::asio::async_connect(*sendSocket, endpointIter,
	        boost::bind(&xpcc::tcpip::Client::connect_handler, this,
	          boost::asio::placeholders::error));
	}
	else{
		XPCC_LOG_ERROR << "Client already connected. Unable to establish new connection to "<< ip<<":"<<port<<xpcc::endl;
	}
}


void
xpcc::tcpip::Client::disconnect(){

}

bool
xpcc::tcpip::Client::isConnected(){
	return this->connected;
}

int
xpcc::tcpip::Client::getServerPort() const
{
	return this->serverPort;
}

void
xpcc::tcpip::Client::addComponent(uint8_t id)
{
	boost::shared_ptr<ComponentInfo> newComponent = boost::shared_ptr<ComponentInfo>(new ComponentInfo(id));
	this->componentMap.insert(std::make_pair (id,newComponent));
	boost::shared_ptr<xpcc::tcpip::Message> msg(new xpcc::tcpip::Message(id));
	this->spawnReceiveThread(id);
	this->sendPacket(msg);

}

void
xpcc::tcpip::Client::spawnReceiveThread(uint8_t id)
{
	boost::shared_ptr<xpcc::tcpip::Receiver> receiver(new xpcc::tcpip::Receiver(this, id));
	this->componentReceiver.push_back(receiver);
	boost::shared_ptr<boost::thread> receiverThread(
			new boost::thread(boost::bind(&xpcc::tcpip::Receiver::run, &*receiver)));
	this->receiveThreadPool.push_back(receiverThread);
}

void
xpcc::tcpip::Client::update()
{
}


void
xpcc::tcpip::Client::sendAlivePing(int identifier)
{

}

void
xpcc::tcpip::Client::connect_handler(const boost::system::error_code& error)
{
	this->connected = true;
	this->writingMessages = true;
	//XPCC_LOG_DEBUG << "Client connected with error-code: "<< error.value() <<xpcc::endl;

}

void
xpcc::tcpip::Client::disconnect_handler(const boost::system::error_code& error){
	this->connected = false;
}

//send a xpcc packet to the server
void
xpcc::tcpip::Client::sendPacket(boost::shared_ptr<xpcc::tcpip::Message> msg)
{
	boost::lock_guard<boost::mutex> lock(this->sendMessagesMutex);
	writingMessages = !messagesToBeSent.empty();
	messagesToBeSent.push_back(msg);
    if (!writingMessages)
    {

      messagesToBeSent.front()->encodeMessage();

      boost::asio::async_write(*sendSocket,
          boost::asio::buffer(messagesToBeSent.front()->getEncodedMessage(),
          messagesToBeSent.front()->getMessageLength()),
          boost::bind(&xpcc::tcpip::Client::writeHandler, this,
            boost::asio::placeholders::error));
    }
}

void
xpcc::tcpip::Client::writeHandler(const boost::system::error_code& error)
{
    if (!error)
    {
    	//Remove sent message
    	boost::lock_guard<boost::mutex> lock(this->sendMessagesMutex);
    	messagesToBeSent.pop_front();
    	if (!messagesToBeSent.empty())
    	{
    		//Prepare next message
    		messagesToBeSent.front()->encodeMessage();
    		boost::asio::async_write(*sendSocket,
    		        boost::asio::buffer(messagesToBeSent.front()->getEncodedMessage(),
    		        messagesToBeSent.front()->getMessageLength()),
    				boost::bind(&xpcc::tcpip::Client::writeHandler, this,
    				boost::asio::placeholders::error));
      }
    }
}

//TODO make thread safe
void
xpcc::tcpip::Client::receiveNewMessage(boost::shared_ptr<xpcc::tcpip::Message> message)
{
	boost::lock_guard<boost::mutex> lock(this->receiveMessagesMutex);
	this->receivedMessages.push_back(message);
}

bool
xpcc::tcpip::Client::isMessageAvailable() const
{
	boost::lock_guard<boost::mutex> lock(this->receiveMessagesMutex);
	return !this->receivedMessages.empty();
}

boost::shared_ptr<xpcc::tcpip::Message>
xpcc::tcpip::Client::getMessage() const
{
	boost::lock_guard<boost::mutex> lock(this->receiveMessagesMutex);
	boost::shared_ptr<xpcc::tcpip::Message> msg = this->receivedMessages.front();
	return msg;
}

void
xpcc::tcpip::Client::deleteMessage()
{
	boost::lock_guard<boost::mutex> lock(receiveMessagesMutex);
	this->receivedMessages.pop_front();
}


boost::shared_ptr< boost::asio::io_service >
xpcc::tcpip::Client::getIOService()
{
	return this->ioService;
}
