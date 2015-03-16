// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------
#ifndef XPCC_TCPIP_CONNECTION_HPP
#define XPCC_TCPIP_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <xpcc/debug/logger.hpp>

#include <list>

#include "tcpip_message.hpp"


namespace xpcc
{

namespace tcpip
{

// forward declaration
class Server;

/**
 * A receiving connection for each container connected to the server,
 * may receive messages from different components
 *
 * @author Thorsten Lajewski
 */
class Connection: public boost::enable_shared_from_this<xpcc::tcpip::Connection>
{
public:
	Connection(boost::shared_ptr<boost::asio::io_service> ioService,
			xpcc::tcpip::Server* server);

	void
	start();

	boost::asio::ip::tcp::socket&
	getSocket();

	void
	receiveMessage();

	void
	sendMessage(boost::shared_ptr<xpcc::tcpip::Message> message);

	bool
	isListening();

	bool
	isConnected();

	typedef boost::shared_ptr<Connection> ConnectionPtr;

private:
	void
	handleReadHeader(const boost::system::error_code& error);

	void
	handleReadBody(const boost::system::error_code& error);

	void
	handleSendMessage(const boost::system::error_code& error);

	boost::asio::ip::tcp::socket socket;

	//pointer to parent server
	xpcc::tcpip::Server* server;
	bool listen;

	//storage for current received message
	char header[xpcc::tcpip::TCPHeader::HSIZE];
	char message[xpcc::tcpip::Message::MSIZE];

	//store for all messages which need to be sent
	std::list< boost::shared_ptr<xpcc::tcpip::Message> > messagesToBeSent;
};

}	// namespace tcpip

}	// namespace xpcc

#endif	// XPCC_TCPIP_CONNECTION_HPP
