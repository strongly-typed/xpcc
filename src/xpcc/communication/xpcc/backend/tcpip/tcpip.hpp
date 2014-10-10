// coding: utf-8
// ----------------------------------------------------------------------------
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
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

#ifndef XPCC__TCPIP_HPP
#define XPCC__TCPIP_HPP

#include <xpcc/architecture/platform/hosted/tcpip/tcpip_client.hpp>
#include <xpcc/container/smart_pointer.hpp>

#include "../backend_interface.hpp"

#include <list>
#include <string>

namespace xpcc
{
	/**
     * \brief	Class that connects the communication to the TCPIP network
     * 			A server has to be available for the communication.
     * 			Each componentes receives messages on it's own port.
	 * 
     * \see 	tcpip
	 * 
	 * \ingroup	backend
     * \author Thorsten Lajewski <thorsten.lajewski@rwth-aachen.de>
	 */
    class TcpIpConnector : public BackendInterface
	{
	public :
        TcpIpConnector(/*std::string ip, int port*/);
		
        ~TcpIpConnector();

        /// check if a connection to the server has been established
        bool
        isConnected();

        /// connect client to server
        void
        connect(std::string ip, int port);

        /// disconnect
        void
        disconnect();
		
		/// Check if a new packet was received by the backend
		virtual bool
		isPacketAvailable() const;
		
		/**
		 * \brief	Access the packet header
		 * 
		 * Only valid if isPacketAvailable() returns \c true.
		 */
		virtual const Header&
		getPacketHeader() const;
		
		/**
		 * \brief	Access the packet payload
		 * 
		 * Only valid if isPacketAvailable() returns \c true.
		 */
		virtual const SmartPointer
		getPacketPayload() const;
		
		/**
		 * \brief	Delete the current packet
		 * 
		 * Only valid if isPacketAvailable() returns \c true.
		 */
		virtual void
		dropPacket();
		
		/**
		 * \brief	Update method
		 */
		virtual void
		update();
		
		inline void
		addReceiverId(uint8_t id)
		{
			this->client.addComponent(id);
		}

		/**
		 * Send a Message.
		 */
		virtual void
		sendPacket(const Header &header,
				   SmartPointer payload = SmartPointer());
		
	private:

		xpcc::tcpip::Client client;

	};
};
 
#endif	// XPCC__TIPC_HPP
