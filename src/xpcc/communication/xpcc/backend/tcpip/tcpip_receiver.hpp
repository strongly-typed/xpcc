// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_TCPIP_RECEIVER_HPP
#define XPCC_TCPIP_RECEIVER_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>

#include <list>

#include <xpcc/communication/xpcc/backend/tcpip/tcpip_message.hpp>

namespace xpcc
{

namespace tcpip
{

// forward delaration
class Client;

/**
 * Receiver for all messages to a component.
 *
 * @author Thorsten Lajewski
 */
class Receiver
{
public:
	Receiver(xpcc::tcpip::Client* parent, int componentId);

	void
	readMessage(const xpcc::tcpip::TCPHeader& header);

	void
	run();

	int
	getId();

	// this method should only be called by the parent client, never shutdown one receiver separately
	void
	shutdownCommand();

private:
	void
	acceptHandler(const boost::system::error_code& error);

	void
	readHeader();

	void
	readHeaderHandler(const boost::system::error_code& error);

	void
	readMessageHandler(const boost::system::error_code& error);

	xpcc::tcpip::Client* parent;
	int componentId;

	boost::asio::ip::tcp::endpoint endpoint;
	boost::asio::ip::tcp::acceptor acceptor;
	boost::asio::ip::tcp::socket socket;

	// storage for current received message
	char header[xpcc::tcpip::TCPHeader::HSIZE];
	char message[xpcc::tcpip::Message::MSIZE];

	// storage for last received messages
	// std::list<boost::shared_ptr<xpcc::tcpip::Message> > receivedMessages;

	bool connected;
	boost::mutex connectedMutex;

	bool shutdown;
};

}	// namespace tpcip

}	// namespace xpcc

#endif	// XPCC_TCPIP_RECEIVER_HPP
