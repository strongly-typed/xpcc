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
#ifndef XPCC_TCPIP__MESSAGE_HPP
#define XPCC_TCPIP__MESSAGE_HPP

#include <boost/asio.hpp>
#include <xpcc/container/smart_pointer.hpp>
#include <xpcc/communication/xpcc/backend/header.hpp>

#include <stdint.h>


namespace xpcc
{
	namespace tcpip
	{

        class TCPHeader{

        public:

			enum class Type{
				REGISTER,
				PING,
				DATA
			};

			//create REGISTER_MESSAGE
			TCPHeader(const uint8_t sender);

			//create Data Message
			TCPHeader(const xpcc::Header& header, int dataSize);

			bool isDataMessage() const;

			uint8_t getDataSize() const;

			static constexpr int HSIZE = sizeof(Type)+sizeof(uint8_t)+sizeof(xpcc::Header);

			static constexpr int headerSize(){
				//return sizeof(Type)+sizeof(xpcc::Header);
				return HSIZE;
			}

			xpcc::Header& getXpccHeader();

			Type getType() const;


        private:

			Type type;
			xpcc::Header header;
			uint8_t dataLength;

        };

		class Message
		{
		public:

			static constexpr int MSIZE = 252;
			static constexpr int SSIZE = TCPHeader::HSIZE + Message::MSIZE;

			//this constructor generates a data message
			Message(const xpcc::Header& header, const SmartPointer payload);

			//copy constructor
			Message(const Message& msg);

			Message(const uint8_t identifier);

			void encodeMessage();

			const char* getEncodedMessage() const;

			int getMessageLength() const;

			xpcc::Header& getXpccHeader();

			xpcc::tcpip::TCPHeader& getTCPHeader();

			xpcc::SmartPointer getMessagePayload() const;

		private:

			xpcc::tcpip::TCPHeader  header;
			SmartPointer data; //contains the message in an encoded form

			//store for complete message in encoded form
			char dataStorage[SSIZE];

		};
	}
}
#endif // XPCC_TCPIP__MESSAGE_HPP
