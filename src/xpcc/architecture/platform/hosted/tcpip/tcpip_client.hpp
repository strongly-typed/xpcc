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
#ifndef XPCC_TCPIP__CLIENT_HPP
#define XPCC_TCPIP__CLIENT_HPP

//-----------------------------------------------------------------------------
//boost lib includes
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//-----------------------------------------------------------------------------
//stl lib includes
#include <list>
#include <string>
#include <map>

//-----------------------------------------------------------------------------
//xpcc dependencies
#include <xpcc/processing/periodic_timer.hpp>
#include <xpcc/architecture/platform/hosted/tcpip/tcpip_message.hpp>
#include <xpcc/architecture/platform/hosted/tcpip/tcpip_receiver.hpp>
#include <xpcc/debug/logger.hpp>


namespace xpcc
{
	namespace tcpip
	{

	    /**
         * The Client sends all actions and events to the Server and receives
         * all responses.
         * The client consists of several threads:
         * One thread is reserved for for the io_service
         * One thread is used for sending packages to the server
         * And one thread for receiving data from each component
	     *
	     *  \author Thorsten Lajewski <thorsten.lajewski@rwth-aachen.de>
	     */
		class Client
		{
		public:

			Client();

			//~Client();

			int getServerPort() const;

			void connect(std::string ip, int port);

			bool isConnecting();

			void disconnect();

			bool isConnected();

			void addComponent(uint8_t id);

			//the server has to receive ping from each component
			void sendAlivePing(int identifier);

			//send a xpcc packet to the server
			void sendPacket(boost::shared_ptr<xpcc::tcpip::Message> msg);

			bool isMessageAvailable() const;

			//get a pointer to the last message in the receivedMessages list
			boost::shared_ptr<xpcc::tcpip::Message> getMessage() const;

			//place new message in the received messages list
			void receiveNewMessage(boost::shared_ptr<xpcc::tcpip::Message> message);

			//remove last Message from the receivedMessages list
			void deleteMessage();

			//timer need to be checked continously
			void update();

			//register client to receive all messages (independent of running components in the process)
			void listen();

			//get pointer to the io_service of the client
			boost::shared_ptr< boost::asio::io_service >
			getIOService();

		private:

			void connect_handler(const boost::system::error_code& error);

			void disconnect_handler(const boost::system::error_code& error);

			void connection_timeout_handler(const boost::system::error_code& error);

			void writeHandler(const boost::system::error_code& error);

			void spawnReceiveThread(uint8_t id);

	    	void readHeader();

	    	void readMessage(const xpcc::tcpip::TCPHeader& header);

	    	void readHeaderHandler(const boost::system::error_code& error);

	    	void readMessageHandler(const boost::system::error_code& error);

			void close();

			bool registered(uint8_t componentId);

			struct ComponentInfo{
				ComponentInfo(uint8_t id):
						identifier(id),
						timer(200)
				{
				}

				uint8_t identifier;
				xpcc::PeriodicTimer<> timer;

			};

			bool connecting;
			mutable boost::mutex connectingMutex;

			bool connected;
			mutable boost::mutex connectedMutex;

			bool writingMessages;
			bool closeConnection;

			boost::shared_ptr< boost::asio::io_service >  ioService;
			boost::shared_ptr< boost::asio::io_service::work > work;
			boost::thread ioThread;

			//timeouts
			boost::asio::deadline_timer connectionTimer;

			//send connection to the server
			int serverPort;
			boost::asio::ip::tcp::resolver::iterator endpointIter;
			boost::shared_ptr<boost::asio::ip::tcp::socket> sendSocket;
			boost::shared_ptr< boost::thread > serviceThread;
			boost::shared_ptr< boost::thread > sentThread;
			std::list< boost::shared_ptr<xpcc::tcpip::Message> > messagesToBeSent;
			mutable boost::mutex sendMessagesMutex;

			//one thread and one receiver for each component of the container
			std::list< boost::shared_ptr< boost::thread > > receiveThreadPool;
			std::list< boost::shared_ptr< xpcc::tcpip::Receiver> > componentReceiver;
			std::map< uint8_t, boost::shared_ptr<ComponentInfo> > componentMap;

			//list for available messages
			std::list< boost::shared_ptr<xpcc::tcpip::Message> > receivedMessages;
			mutable boost::mutex receiveMessagesMutex;

			//temporary memory for receiving messages in the client-server connection
			//storage for current received message
			char header[xpcc::tcpip::TCPHeader::HSIZE];
			char message[xpcc::tcpip::Message::MSIZE];

		};
	}
}
#endif // XPCC_TCPIP__CLIENt_HPP
