// coding: utf-8
// ----------------------------------------------------------------------------
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Roboterclub Aachen e.V. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ROBOTERCLUB AACHEN E.V. ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ROBOTERCLUB AACHEN E.V. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// ----------------------------------------------------------------------------
#ifndef XPCC_TCPIP__CONNECTION_HPP
#define XPCC_TCPIP__CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <xpcc/debug/logger.hpp>

#include "tcpip_message.hpp"

namespace xpcc {
namespace tcpip {

class Server;

/**
 * \brief a receiving connection for each container connected to the server,
 * 		  may receive messages from different components
 *
 *  \author Thorsten Lajewski
 */

class Connection: public boost::enable_shared_from_this<xpcc::tcpip::Connection> {

	public:

		Connection(boost::shared_ptr<boost::asio::io_service> ioService,
		        xpcc::tcpip::Server* server);

		void start();

		boost::asio::ip::tcp::socket& getSocket();

		void receiveMessage();

		void sendMessage(boost::shared_ptr<xpcc::tcpip::Message> message);

		bool isListening();

		typedef boost::shared_ptr<Connection> ConnectionPtr;

	private:

		void handleReadHeader(const boost::system::error_code& error);

		void handleReadBody(const boost::system::error_code& error);

		void handleSendMessage(const boost::system::error_code& error);

		boost::asio::ip::tcp::socket socket;

		//pointer to parent server
		xpcc::tcpip::Server* server;
		bool listen;

		//storage for current received message
		char header[xpcc::tcpip::TCPHeader::HSIZE];
		char message[xpcc::tcpip::Message::MSIZE];

};
}
}
#endif

