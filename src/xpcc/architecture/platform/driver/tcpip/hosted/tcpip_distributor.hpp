// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_TCPIP_DISTRIBUTOR_HPP
#define XPCC_TCPIP_DISTRIBUTOR_HPP

//-----------------------------------------------------------------------------
//boost lib includes
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
//-----------------------------------------------------------------------------
//stl lib includes
#include <string>
#include <list>
//-----------------------------------------------------------------------------
//xpcc dependencies
#include <xpcc/architecture/platform/driver/tcpip/hosted/tcpip_message.hpp>
#include <xpcc/debug/logger.hpp>

namespace xpcc
{

namespace tcpip
{

// forward declaration
class Server;

/**
 * For each component registered on the server one Distributor is created,
 * this Distrubutor handles sending of messages to the component
 *
 * @author Thorsten Lajewski
 */
class Distributor
{
public:
	Distributor(xpcc::tcpip::Server* parent, std::string ip, uint8_t component_id);

	void
	run();

	// send a xpcc packet to the component
	void
	sendMessage(boost::shared_ptr<xpcc::tcpip::Message> msg);

	// close the tcpip connection
	void
	disconnect();

private:
	void
	connectHandler(const boost::system::error_code& error);

	void
	sendHandler(const boost::system::error_code& error);

	bool connected;
	bool writingMessages;

	boost::shared_ptr< boost::asio::io_service >  ioService;

	//send connection to the component
	Server* server;
	int port;

	boost::asio::ip::tcp::resolver::iterator endpointIter;
	boost::shared_ptr<boost::asio::ip::tcp::socket> sendSocket;
	std::list< boost::shared_ptr<xpcc::tcpip::Message> > messagesToBeSent;
	boost::mutex sendMutex;
};

}	// namespace tcpip

}	// namespace xpcc

#endif // XPCC_TCPIP_DISTRIBUTOR_HPP
