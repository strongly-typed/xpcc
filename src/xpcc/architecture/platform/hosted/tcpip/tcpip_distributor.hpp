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
#ifndef XPCC_TCPIP__DISTRIBUTOR_HPP
#define XPCC_TCPIP__DISTRIBUTOR_HPP

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
#include <xpcc/architecture/platform/hosted/tcpip/tcpip_message.hpp>
#include <xpcc/debug/logger.hpp>

namespace xpcc
{
	namespace tcpip
	{

	    /**
		 * For each component registered on the server one Distributor is created,
		 * this Distrubutor handles sending of messages to the component
	     *
	     *  \author Thorsten Lajewski
	     */

		//forward declaration
		class Server;

		class Distributor
		{
		public:

			Distributor(xpcc::tcpip::Server* parent, std::string ip, uint8_t component_id);

			void run();

			//send a xpcc packet to the component
			void sendMessage(boost::shared_ptr<xpcc::tcpip::Message> msg);

			//close the tcpip connection
			void disconnect();

		private:

			void connectHandler(const boost::system::error_code& error);

			void sendHandler(const boost::system::error_code& error);

			void close();

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
	}
}
#endif // XPCC_TCPIP__CLIENT_HPP
