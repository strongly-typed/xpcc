// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_TCPIP_MESSAGE_HPP
#define XPCC_TCPIP_MESSAGE_HPP

#include <boost/asio.hpp>
#include <xpcc/container/smart_pointer.hpp>
#include <xpcc/communication/xpcc/backend/header.hpp>

#include <stdint.h>


namespace xpcc
{

namespace tcpip
{

class TCPHeader
{
public:
	enum class
	Type: int
	{
		REGISTER = 1,
		UNREGISTER,
		CLOSE_CONNECTION,
		LISTEN,
		PING,
		DATA,
		ERROR
	};

	// create REGISTER_MESSAGE
	TCPHeader(const uint8_t sender);

	// create Data Message
	TCPHeader(const xpcc::Header& header, int dataSize);

	TCPHeader(const Type type, const xpcc::Header& header, int dataSize);


	Type
	getMessageType() const;

	uint8_t
	getDataSize() const;

	static constexpr int HSIZE = sizeof(Type) + sizeof(uint8_t) + sizeof(xpcc::Header);

	static constexpr int
	headerSize()
	{
		//return sizeof(Type)+sizeof(xpcc::Header);
		return HSIZE;
	}

	xpcc::Header&
	getXpccHeader();

	Type
	getType() const;

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

	// this constructor generates a data message
	Message(const xpcc::Header& header, const SmartPointer& payload);

	// copy constructor
	Message(const Message& msg);

	// this constructor generates a register mesage
	Message(const uint8_t identifier);

	// this constructor is for all other special message types
	Message(TCPHeader::Type type, const xpcc::Header& header, const SmartPointer& payload);


	// transforms the message in an char array
	void
	encodeMessage();

	const char*
	getEncodedMessage() const;

	int
	getMessageLength() const;

	xpcc::Header&
	getXpccHeader();

	xpcc::tcpip::TCPHeader&
	getTCPHeader();

	xpcc::SmartPointer
	getMessagePayload() const;

private:
	xpcc::tcpip::TCPHeader  header;
	SmartPointer data; //contains the message in an encoded form

	//store for complete message in encoded form
	char dataStorage[SSIZE];
};

}	// namespace tcpip

}	// namespace xpcc

#endif // XPCC_TCPIP_MESSAGE_HPP
