#include "tcpip_server.hpp"
#include "tcpip_distributor.hpp"

#include <iostream>
#include <xpcc/debug/logger.hpp>

xpcc::tcpip::Server::Server(int port):
	ioService(new boost::asio::io_service()),
	work(new boost::asio::io_service::work(*ioService)),
	ioThread(boost::bind(&boost::asio::io_service::run, ioService)),
	endpoint(boost::asio::ip::tcp::v4(), port),
	acceptor(*ioService, endpoint),
	serverPort(port)
{
	spawnReceiveConnection();
}

boost::shared_ptr< boost::asio::io_service >
xpcc::tcpip::Server::getIoService()
{
	return this->ioService;
}

void
xpcc::tcpip::Server::accept_handler(boost::shared_ptr<xpcc::tcpip::Connection> receive,
		const boost::system::error_code& error)
{
	//XPCC_LOG_DEBUG<< "Connection from " << receive->getSocket().remote_endpoint().address().to_string()
	//		<< " accepted" << xpcc::endl;

    if (!error)
    {
      this->receiveConnections.push_back(receive);
      receive->start();
    }

    spawnReceiveConnection();
}


void
xpcc::tcpip::Server::spawnSendThread(uint8_t componentId, std::string ip)
{
	boost::shared_ptr<xpcc::tcpip::Distributor> sender(
			new xpcc::tcpip::Distributor(this, ip, componentId));
	boost::lock_guard<boost::mutex> lock(distributorMutex);
	this->distributorMap.emplace(componentId, sender);
}

void
xpcc::tcpip::Server::spawnReceiveConnection()
{
	boost::shared_ptr<xpcc::tcpip::Connection> receiveConnection(new Connection(ioService, this));
		this->acceptor.async_accept(receiveConnection->getSocket(),
				 boost::bind(&xpcc::tcpip::Server::accept_handler, this,
						 receiveConnection, boost::asio::placeholders::error));
}

void
xpcc::tcpip::Server::update()
{
}

//TODO make function parameter shared_ptr to prevent copying of the whole message
void
xpcc::tcpip::Server::distributeDataMessage(xpcc::tcpip::Message msg)
{
	uint8_t destination = msg.getXpccHeader().destination;
	if(destination != 0)
	{
		//handle action

		//check if destination is a registered component, else drop message
		boost::lock_guard<boost::mutex> lock(distributorMutex);
		if(this->distributorMap.find(destination)!= this->distributorMap.end())
		{
			this->distributorMap[destination]->sendMessage(boost::shared_ptr<xpcc::tcpip::Message>(
					new xpcc::tcpip::Message(msg)));
		}
		else
		{
			//drop message print warning
			XPCC_LOG_WARNING << "Message to Component with id "<< msg.getXpccHeader().destination << " dropped! "<<
					"Component " << msg.getXpccHeader().destination << " not registered on server!" << xpcc::endl;
		}

	}
	else
	{
		//handle event
		//send message to all registered components
		boost::lock_guard<boost::mutex> lock(distributorMutex);
		for (auto& pair : this->distributorMap)
		{
			pair.second->sendMessage(boost::shared_ptr<xpcc::tcpip::Message>(
					new xpcc::tcpip::Message(msg)));
		}
	}
}
