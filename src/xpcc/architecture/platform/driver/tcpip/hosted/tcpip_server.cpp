#include "tcpip_server.hpp"

#include <iostream>

#include <xpcc/debug/logger.hpp>
#include <xpcc/architecture/platform/driver/tcpip/hosted/tcpip_distributor.hpp>

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
xpcc::tcpip::Server::distributeMessage(xpcc::tcpip::Message msg)
{

	//handle different messages differently, only data messages can have more than one receiver
	xpcc::tcpip::TCPHeader::Type type = msg.getTCPHeader().getMessageType();
	uint8_t destination = msg.getXpccHeader().destination;

	//independent of destination distribute msg to all listening clients
	switch(type){
		case xpcc::tcpip::TCPHeader::Type::DATA:{


			for(std::list<boost::shared_ptr<xpcc::tcpip::Connection> >::iterator iter = this->receiveConnections.begin();
					iter!=this->receiveConnections.end(); iter++){
				if((*iter)->isListening()){

					(*iter)->sendMessage(boost::shared_ptr<xpcc::tcpip::Message>(
							new xpcc::tcpip::Message(msg)));

				}
			}

			if(destination == 0)
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
			break;
		}
	}

	//handle component specific sending
	if(destination != 0){
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
}

void
xpcc::tcpip::Server::shutdownDistributor(int id){
	//2. remove distributor from list
	boost::shared_ptr<xpcc::tcpip::Distributor> distributor = (this->distributorMap.find( (uint8_t) id))->second;
	this->distributorMap.erase((uint8_t) id);
	//3. close distributor
	distributor->disconnect();

}

void
xpcc::tcpip::Server::removeClosedConnections(){
	for(std::list< boost::shared_ptr<xpcc::tcpip::Connection> >::iterator iter = receiveConnections.begin();
			iter != receiveConnections.end(); iter++){
		if(!(*iter)->isConnected()){
			iter->reset();
		}
	}
}
