// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#include "tcpip_client.hpp"

#include <iostream>
#include <sstream>

#include <boost/thread/locks.hpp>

xpcc::tcpip::Client::Client():
	connecting(false),
	connected(false),
	writingMessages(false),
	closeConnection(false),
	ioService(new boost::asio::io_service()),
	work(new boost::asio::io_service::work(*ioService)),
	ioThread(boost::bind(&boost::asio::io_service::run, ioService)),
	connectionTimer(*ioService),
	closeConnectionTimer(*ioService),
	serverPort(-1)
{
	connectionTimer.expires_at(boost::posix_time::pos_infin);
}

void
xpcc::tcpip::Client::connect(std::string ip, int port)
{
	if(!this->isConnected() and !this->isConnecting())
	{
		{
			boost::lock_guard<boost::mutex> lock(this->connectingMutex);
			this->connecting = true;
			this->connectionTimer.expires_from_now(boost::posix_time::seconds(10));
			this->connectionTimer.async_wait(
					boost::bind(&xpcc::tcpip::Client::connection_timeout_handler, this, boost::asio::placeholders::error));
		}

		this->serverPort = port;
		boost::asio::ip::tcp::resolver resolver(*ioService);
		//port required as string
		std::stringstream portStream;
		portStream << port;
		boost::asio::ip::tcp::resolver::query query(ip, portStream.str());
		this->endpointIter = resolver.resolve(query);

		//this->serviceThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&boost::asio::io_service::run, ioService)));

		this->sendSocket.reset(new boost::asio::ip::tcp::socket(*ioService));

		boost::asio::async_connect(*sendSocket, endpointIter,
			boost::bind(&xpcc::tcpip::Client::connect_handler, this,
			boost::asio::placeholders::error));
	}
	else
	{
		XPCC_LOG_ERROR << "Client already connected. Unable to establish new connection to " << ip.c_str() << ":" << port << xpcc::endl;
		XPCC_LOG_ERROR << "Is connecting: " << this->isConnecting() << xpcc::endl;
		XPCC_LOG_ERROR << "Is connected: " << this->isConnected() << xpcc::endl;
	}
}


void
xpcc::tcpip::Client::disconnect()
{
	//put unregister message for all connected receivers into send que and remove corresponding ComponentInfo
	{
		boost::lock_guard<boost::mutex> lock(this->closeConnectionMutex);
		this->closeConnection = true;
	}
	//TODO start deadline timer to force shutdown
	for(auto iter = this->componentReceiver.begin(); iter != this->componentReceiver.end(); iter++)
	{
		uint8_t id = (*iter)->getId();
		//no complete header is needed since the created message will not be a data message
		xpcc::Header messageHeader;
		messageHeader.source = id;
		messageHeader.destination = 0;
		xpcc::SmartPointer payload(0);
		boost::shared_ptr<xpcc::tcpip::Message> message(
				new xpcc::tcpip::Message(xpcc::tcpip::TCPHeader::Type::UNREGISTER, messageHeader, payload));
		this->sendPacket(message);
		this->componentMap.erase(id);
	}
}

bool
xpcc::tcpip::Client::isConnected()
{
	boost::lock_guard<boost::mutex> lock(this->connectedMutex);
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
	this->componentMap.insert(std::make_pair (id, newComponent));
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
	this->receiveThreadPool.emplace(id, receiverThread);
}

void
xpcc::tcpip::Client::update()
{
}

void
xpcc::tcpip::Client::sendAlivePing(int identifier)
{
}

bool
xpcc::tcpip::Client::isConnecting()
{
	boost::lock_guard<boost::mutex> lock(this->connectingMutex);
	return this->connecting;
}

void
xpcc::tcpip::Client::connect_handler(const boost::system::error_code& error)
{
	{
		boost::lock_guard<boost::mutex> lock(this->connectingMutex);
		this->connecting = false;
		this->connectionTimer.cancel();
	}

	if(!error)
	{
		boost::lock_guard<boost::mutex> lock(this->connectedMutex);
		this->connected = true;
		this->writingMessages = true;
		this->readHeader();
	}
	//XPCC_LOG_DEBUG << "Client connected with error-code: " << error.value() << xpcc::endl;

}

void
xpcc::tcpip::Client::disconnect_handler(const boost::system::error_code& error)
{
	boost::system::error_code ec;
	sendSocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

	if (ec)
	{
		std::cout << "Client shutdown with error code " << ec << std::endl;
	}

	sendSocket->close(ec);
	if (ec)
	{
		std::cout << "Client closed with error code " << ec << std::endl;
	}

	{
		boost::lock_guard<boost::mutex> lock1(this->closeConnectionMutex);
		this->closeConnection = false;
	}

	{
		boost::lock_guard<boost::mutex> lock(this->connectedMutex);
		XPCC_LOG_DEBUG << "Setting connected to false!" << xpcc::endl;
			this->connected = false;
	}
	XPCC_LOG_DEBUG << "Is connected: " << this->isConnected() << xpcc::endl;
	std::cout << "Connection closed!" << std::endl;

	{
		boost::lock_guard<boost::mutex> lock(this->receiveMessagesMutex);
		this->receivedMessages.clear();
	}
	{
		boost::lock_guard<boost::mutex> lock(this->sendMessagesMutex);
		this->messagesToBeSent.clear();
	}
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
		std::cout << "Sending message of type " <<
				(int) messagesToBeSent.front()->getTCPHeader().getMessageType() << std::endl;
		messagesToBeSent.front()->encodeMessage();

		if(messagesToBeSent.front()->getTCPHeader().getMessageType() !=
				xpcc::tcpip::TCPHeader::Type::CLOSE_CONNECTION)
		{
			boost::asio::async_write(*sendSocket,
				boost::asio::buffer(messagesToBeSent.front()->getEncodedMessage(),
				messagesToBeSent.front()->getMessageLength()),
				boost::bind(&xpcc::tcpip::Client::writeHandler, this,
				boost::asio::placeholders::error));
		}
		else
		{
			boost::asio::async_write(*sendSocket,
				boost::asio::buffer(messagesToBeSent.front()->getEncodedMessage(),
				messagesToBeSent.front()->getMessageLength()),
				boost::bind(&xpcc::tcpip::Client::disconnect_handler, this,
				boost::asio::placeholders::error));
		}
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


//register client to receive all messages (independent of running components in the process)
void
xpcc::tcpip::Client::listen()
{
	if(this->isConnected())
	{
		xpcc::Header header;
		xpcc::SmartPointer payload(0);

		boost::shared_ptr<xpcc::tcpip::Message> listenMsg(new xpcc::tcpip::Message(TCPHeader::Type::LISTEN, header, payload));
		this->sendPacket(listenMsg);
	}
	else {
		XPCC_LOG_WARNING << "Cannot listen, no connection to server" << xpcc::endl;
	}
}

boost::shared_ptr< boost::asio::io_service >
xpcc::tcpip::Client::getIOService()
{
	return this->ioService;
}


void
xpcc::tcpip::Client::connection_timeout_handler(const boost::system::error_code& error)
{
	//TODO error handling
	if (error == boost::asio::error::operation_aborted) {
		//timer was cancelled...
	}
	else if (error) {
		XPCC_LOG_ERROR << "Timer error: " << error.message().c_str() << xpcc::endl;
	}
	else{
		XPCC_LOG_WARNING << "Connecting to server timed out!" << xpcc::endl;
	}

	{
		boost::lock_guard<boost::mutex> lock(this->connectingMutex);
		this->connecting = false;
	}
}

void
xpcc::tcpip::Client::readHeader()
{
	boost::asio::async_read(*sendSocket,
		boost::asio::buffer(this->header, xpcc::tcpip::TCPHeader::headerSize()),
		boost::bind(
			&xpcc::tcpip::Client::readHeaderHandler, this,
			boost::asio::placeholders::error));
}

void
xpcc::tcpip::Client::readMessage(const xpcc::tcpip::TCPHeader& header)
{
	int dataSize = header.getDataSize();

	boost::asio::async_read(*sendSocket,
		boost::asio::buffer(this->message, dataSize),
		boost::bind(
			&xpcc::tcpip::Client::readMessageHandler, this,
			boost::asio::placeholders::error));
}

void
xpcc::tcpip::Client::readHeaderHandler(const boost::system::error_code& error)
{
	if(!error)
	{
		xpcc::tcpip::TCPHeader* messageHeader = reinterpret_cast<xpcc::tcpip::TCPHeader*>(this->header);
		this->readMessage(*messageHeader);
	}
	else{
		//TODO error handling
		std::cout << "Error receiving header: " << error.message() << std::endl;
	}
}

void
xpcc::tcpip::Client::readMessageHandler(const boost::system::error_code& error)
{
	if(!error)
	{
		xpcc::tcpip::TCPHeader* messageHeader = reinterpret_cast<xpcc::tcpip::TCPHeader*>(this->header);
		SmartPointer payload(messageHeader->getDataSize());
		memcpy(payload.getPointer(), this->message, messageHeader->getDataSize());

		boost::shared_ptr<xpcc::tcpip::Message> message(
				new xpcc::tcpip::Message(messageHeader->getMessageType(), messageHeader->getXpccHeader(), payload));
		/*
		if(message->getTCPHeader().getMessageType() == xpcc::tcpip::TCPHeader::Type::DATA &&
			!(message->getXpccHeader().destination == 0) && !this->registered(message->getXpccHeader().destination))
		{
			this->receiveNewMessage(message);
		}*/
		switch(message->getTCPHeader().getMessageType())
		{
			case TCPHeader::Type::DATA:
			{
				//data messages are normally received via the receiver classes
				//all data messages sent to the client directly are only for listening purposes
				// in order to prevent messages from active components or events
				// to be received twice filter them
				if(!this->componentMap.empty() and !(message->getXpccHeader().destination == 0) and
						!this->registered(message->getXpccHeader().destination))
				{
					this->receiveNewMessage(message);
				}
				break;
			}

			case TCPHeader::Type::UNREGISTER:
			{
				//confirmation from server that the receiver can be shut down
				int componentId = message->getXpccHeader().destination;
				bool found = false;

				for(auto iter = this->componentReceiver.begin(); iter != this->componentReceiver.end(); iter++)
				{
					if((*iter)->getId() == componentId)
					{
						found = true;
						(*iter)->shutdownCommand();
						this->componentReceiver.erase(iter);

						//try to join thread, at the moment this is blocking!
						XPCC_LOG_DEBUG << "Joining Receiver Thread for id "<<componentId << xpcc::endl;
						this->receiveThreadPool.at(componentId)->join();
						XPCC_LOG_DEBUG << "Joined Receiver Thread for id "<<componentId << xpcc::endl;

						this->receiveThreadPool.erase(componentId);
						boost::lock_guard<boost::mutex> lock(this->closeConnectionMutex);

						if(this->closeConnection and this->receiveThreadPool.empty())
						{
							//send a shutdown message to the server
							//the client connection will be closed in the callback of this send message
							boost::shared_ptr<xpcc::tcpip::Message> msg(
									new xpcc::tcpip::Message(
											xpcc::tcpip::TCPHeader::Type::CLOSE_CONNECTION, xpcc::Header(),
											xpcc::SmartPointer()));

							this->sendPacket(msg);
						}
						break;
					}
				}
				if(!found) {
					std::cout << "Error received shudown message for unknown receiver with id " << componentId << std::endl;
				}
				break;
			}

			default:
				XPCC_LOG_ERROR << "TCPIP Client not able to handle message of type TODO" << xpcc::endl;
				break;
		}

		if(this->connected)
		{
			this->readHeader();
			std::cout << "Sending message of type " <<
					(int) messagesToBeSent.front()->getTCPHeader().getMessageType() << std::endl;
		}
	}
	else {
		//TODO error handling
		std::cout << "Error while receiving message" << std::endl;
	}
}

bool
xpcc::tcpip::Client::registered(uint8_t componentId)
{
	for(auto iter = this->componentReceiver.begin(); iter != this->componentReceiver.end(); iter++)
	{
		if((*iter)->getId() == static_cast<int>(componentId))
			return true;
	}
	return false;
}
