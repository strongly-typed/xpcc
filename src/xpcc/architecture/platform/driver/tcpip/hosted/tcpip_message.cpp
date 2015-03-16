// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#include "tcpip_message.hpp"

xpcc::tcpip::TCPHeader::TCPHeader(const uint8_t sender):
		type(Type::REGISTER), header(), dataLength(0)
{
	this->header.source = sender;
}

xpcc::tcpip::TCPHeader::TCPHeader(const xpcc::Header& header, const int dataSize):
		type(Type::DATA), header(header), dataLength(dataSize)
{
}

xpcc::tcpip::TCPHeader::TCPHeader(const Type type, const xpcc::Header& header, int dataSize):
		type(type), header(header), dataLength(dataSize)
{
}

xpcc::Header&
xpcc::tcpip::TCPHeader::getXpccHeader()
{
	return this->header;
}

xpcc::tcpip::TCPHeader::Type
xpcc::tcpip::TCPHeader::getMessageType() const
{
	return this->type;
}

uint8_t
xpcc::tcpip::TCPHeader::getDataSize() const
{
	return this->dataLength;
}

xpcc::tcpip::TCPHeader&
xpcc::tcpip::Message::getTCPHeader()
{
	return this->header;
}

// ----------------------------------------------------------------------------
xpcc::tcpip::Message::Message(const xpcc::Header& header, const SmartPointer& payload):
		header(header, static_cast<int>(payload.getSize())), data(payload)
{
}

xpcc::tcpip::Message::Message(TCPHeader::Type type, const xpcc::Header& header, const SmartPointer& payload):
		header(type, header, payload.getSize()), data(payload)
{
}

xpcc::tcpip::Message::Message(const uint8_t identifier):
		header(identifier), data()
{
}

xpcc::tcpip::Message::Message(const Message& msg):
		header(msg.header), data(msg.data)
{
}

void
xpcc::tcpip::Message::encodeMessage()
{
	//deep copy message to char array
	char* header = reinterpret_cast<char*>(&this->getTCPHeader());

	for(int i = 0; i< this->getMessageLength(); ++i)
	{
		this->dataStorage[i] = 0;
	}

	for(int i = 0; i<xpcc::tcpip::TCPHeader::headerSize(); ++i)
	{
		this->dataStorage[i] = header[i];
	}

	if(this->getTCPHeader().getDataSize() > 0)
	{
		uint8_t* dataPtr = this->data.getPointer();

		for(int i = xpcc::tcpip::TCPHeader::headerSize(); i< this->getMessageLength(); ++i)
		{
			this->dataStorage[i] = static_cast<char>(dataPtr[i-xpcc::tcpip::TCPHeader::headerSize()]);
		}
	}
}

const char*
xpcc::tcpip::Message::getEncodedMessage() const
{
	return this->dataStorage;
}

int
xpcc::tcpip::Message::getMessageLength() const
{
	return xpcc::tcpip::TCPHeader::headerSize() + this-> header.getDataSize();
}

xpcc::Header&
xpcc::tcpip::Message::getXpccHeader()
{
	return this->header.getXpccHeader();
}

xpcc::SmartPointer
xpcc::tcpip::Message::getMessagePayload() const
{
	return this->data;
}
