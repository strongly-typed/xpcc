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
#ifndef XPCC_TCPIP__RECEIVER_HPP
#define XPCC_TCPIP__RECEIVER_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>

#include <list>

#include <xpcc/architecture/platform/hosted/tcpip/tcpip_message.hpp>

namespace xpcc{
namespace tcpip{

	/**
	 * \brief receiver for all messages to a component.
 	 *
 	 *  \author Thorsten Lajewski
 	 */

	//forward delaration
	class Client;

    class Receiver {
    public:

    	Receiver(xpcc::tcpip::Client* parent, int componentId);

    	void readMessage(const xpcc::tcpip::TCPHeader& header);

    	void run();

    	//places the first received message in the client message list,
    	//where it's handled by the postman
    	// returns true if a message was published
    	// false if no message is available
    	//TODO make thread safe
    	bool publishMessage();


    private:

    	void acceptHandler(const boost::system::error_code& error);

    	void readHeader();

    	void readHeaderHandler(const boost::system::error_code& error);

    	void readMessageHandler(const boost::system::error_code& error);

    	xpcc::tcpip::Client* parent;
    	int componentId;

		boost::asio::ip::tcp::endpoint endpoint;
		boost::asio::ip::tcp::acceptor acceptor;
		boost::asio::ip::tcp::socket socket;


		//storage for current received message
		char header[xpcc::tcpip::TCPHeader::HSIZE];
		char message[xpcc::tcpip::Message::MSIZE];

		//storage for last received messages
		//std::list<boost::shared_ptr<xpcc::tcpip::Message> > receivedMessages;

		bool connected;
		boost::mutex connectedMutex;

    	bool shutdown;

    };
}}
#endif
