// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_TCPIP_CONNECTOR_HPP
#define XPCC_TCPIP_CONNECTOR_HPP

#include <xpcc/communication/xpcc/backend/tcpip/tcpip_client.hpp>
#include <xpcc/container/smart_pointer.hpp>

#include <xpcc/communication/xpcc/backend/backend_interface.hpp>

#include <list>
#include <string>

namespace xpcc
{

/**
 * Class that connects the communication to the TCPIP network.
 *
 * A server has to be available for the communication.
 * Each componentes receives messages on it's own port.
 *
 * @see 	tcpip
 *
 * @ingroup	backend
 * @author Thorsten Lajewski <thorsten.lajewski@rwth-aachen.de>
 */
class TcpIpConnector : public BackendInterface
{
public :
	TcpIpConnector(/*std::string ip, int port*/);

	~TcpIpConnector();

	/// check if a connection to the server has been established
	bool
	isConnected();

	///let process listen to all messages processed by the server
	void
	listen();

	/// connect client to server
	bool
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
	sendPacket(const Header &header, SmartPointer payload = SmartPointer());

private:
	xpcc::tcpip::Client client;
};

}	// namespace xpcc

#endif	// XPCC_TIPC_CONNECTOR_HPP
