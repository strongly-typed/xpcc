// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_TCPIP_SERVER_HPP
#define XPCC_TCPIP_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <list>

#include "tcpip_connection.hpp"
#include "tcpip_message.hpp"


namespace xpcc
{

namespace tcpip
{

//forward declaration
class Distributor;

/**
 * This class handles the communication between all components via tcp/ip.
 * All clients register to on the server. The server keeps track of all alive
 * client components and distributes all messages according to the xpcc message
 * header.
 *
 * @author Thorsten Lajewski
 */
class Server
{
public:
	Server(int port);

	//~Server();

	void
	spawnReceiveConnection();

	void
	spawnSendThread(uint8_t componentId, std::string ip);

	void
	shutdownDistributor(int id);

	void
	distributeMessage(xpcc::tcpip::Message msg);

	void
	update();

	boost::shared_ptr<boost::asio::io_service>
	getIoService();

	inline int
	getPort() const
	{
		return serverPort;
	}

	void
	removeClosedConnections();

private:
	void
	accept_handler(boost::shared_ptr<xpcc::tcpip::Connection> receive, const boost::system::error_code& error);

	boost::shared_ptr<boost::asio::io_service> ioService;
	boost::shared_ptr<boost::asio::io_service::work> work;
	boost::thread ioThread;

	boost::asio::ip::tcp::endpoint endpoint;
	boost::asio::ip::tcp::acceptor acceptor;

	int serverPort;
	std::list<boost::shared_ptr<xpcc::tcpip::Connection> > receiveConnections;

	std::map<uint8_t, boost::shared_ptr<xpcc::tcpip::Distributor> > distributorMap;
	boost::mutex distributorMutex;
};

}	// namespace tcpip

}	// namespace xpcc

#endif // XPCC_TCPIP_SERVER_HPP
