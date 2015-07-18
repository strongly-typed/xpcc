// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#include "../tcpip.hpp"
#include <xpcc/debug/logger.hpp>
#include <xpcc/communication/xpcc/backend/tcpip/tcpip_message.hpp>
#include <xpcc/processing/timer/timeout.hpp>


#undef  XPCC_LOG_LEVEL
#define XPCC_LOG_LEVEL xpcc::log::WARNING

// ----------------------------------------------------------------------------
xpcc::TcpIpConnector::TcpIpConnector()
{
}

// ----------------------------------------------------------------------------
xpcc::TcpIpConnector::~TcpIpConnector()
{
}

// ----------------------------------------------------------------------------
bool
xpcc::TcpIpConnector::isPacketAvailable() const
{
	return this->client.isMessageAvailable();
}

// ----------------------------------------------------------------------------
bool
xpcc::TcpIpConnector::isConnected()
{
	return this->client.isConnected();
}

// ---------------------------------------------------------------------------
bool
xpcc::TcpIpConnector::connect(std::string ip, int port)
{
	this->client.connect(ip, port);
	xpcc::Timeout timeout(1500);

	while(this->client.isConnecting())
	{
		//wait blocking for connection
	}
	return this->isConnected();
}

//-----------------------------------------------------------------------------
void
xpcc::TcpIpConnector::listen()
{
	this->client.listen();
}

// ----------------------------------------------------------------------------
void
xpcc::TcpIpConnector::disconnect()
{
	this->client.disconnect();
}

// ----------------------------------------------------------------------------
const xpcc::Header&
xpcc::TcpIpConnector::getPacketHeader() const
{
	return this->client.getMessage()->getXpccHeader();
}

// ----------------------------------------------------------------------------
const xpcc::SmartPointer
xpcc::TcpIpConnector::getPacketPayload() const
{
	return this->client.getMessage()->getMessagePayload();
}

// ----------------------------------------------------------------------------
void
xpcc::TcpIpConnector::dropPacket()
{
	this->client.deleteMessage();
}

// ----------------------------------------------------------------------------
void
xpcc::TcpIpConnector::sendPacket(const xpcc::Header &header, SmartPointer payload)
{
	boost::shared_ptr<xpcc::tcpip::Message> msg(new xpcc::tcpip::Message(header, payload));
	this->client.sendPacket(msg);
}

// ----------------------------------------------------------------------------
void
xpcc::TcpIpConnector::update()
{
}
