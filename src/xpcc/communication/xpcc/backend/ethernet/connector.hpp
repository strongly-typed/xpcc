// coding: utf-8
/* Copyright (c) 2016, Roboterclub Aachen e. V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef	XPCC__ETHERNET_CONNECTOR_HPP
#define	XPCC__ETHERNET_CONNECTOR_HPP

#include <cstring>		// for std::memcpy

#include "../backend_interface.hpp"
#include "xpcc_over_ethernet.hpp"

#include <xpcc/debug/logger.hpp>
#undef XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::ERROR

namespace xpcc
{
template <typename Driver>
class EthernetConnector : public BackendInterface
{
public:
	EthernetConnector(Driver &ethDriver, container_lut_function lut) :
		isAvailable(false),
		driver(ethDriver),
		lut(lut)
	{
	}

	virtual
	~EthernetConnector() {
	}

	virtual void
	sendPacket(const Header &header, SmartPointer payload)
	{
		XPCC_LOG_DEBUG.printf("EC::sendPacket\n");

		XPCC_LOG_DEBUG.printf("  component id = %02x, packet id = %02x\n", header.destination, header.packetIdentifier);

		xpcc::EthernetFrame ethFrame;

		xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(
			/* header  */ header,
			/* payload */ payload,
			/* lut     */ lut,
			/* EthernetFrame */ ethFrame);

		driver.sendMessage(ethFrame);
	}

	virtual bool
	isPacketAvailable() const {
		return isAvailable;
	}

	virtual const Header&
	getPacketHeader() const { return receiveItem->header; };

	virtual const xpcc::SmartPointer
	getPacketPayload() const { return receiveItem->payload; };

	virtual void
	dropPacket()
	{
		isAvailable = false;
		delete receiveItem;
	}


	virtual void
	update()
	{
		if (not this->isAvailable)
		{
			uint8_t length = 0;
			EthernetFrame message;
			if (driver.getMessage(length, message))
			{
				{
					uint8_t const *buf = message; // Copy for increment
					XPCC_LOG_DEBUG.printf("EC RX: %d ***********************\n", length);

					for (uint32_t ii = 0; ii < length; ++ii) {
						XPCC_LOG_DEBUG.printf("%02x ", *buf);
						++buf;

						if (ii % 8 == 7) {
							XPCC_LOG_DEBUG.printf(" ");
						}
						if (ii % 16 == 15) {
							XPCC_LOG_DEBUG.printf("\n");
						}
					}
					XPCC_LOG_DEBUG.printf("\n");
				}

				// Check if valid xpcc message
				if ((length == 64) and 
					((message[0] & 0xfe) == XpccOverEthernet::macPreamble[0]) and /* ignore multicast bit */
					(message[1] == XpccOverEthernet::macPreamble[1]) and
					(message[2] == XpccOverEthernet::macPreamble[2]) and
					(message[6] == XpccOverEthernet::macPreamble[0]) and
					(message[7] == XpccOverEthernet::macPreamble[1]) and
					(message[8] == XpccOverEthernet::macPreamble[2]))
				{
					// Valid

					// Copy to xpcc Message
					this->isAvailable = true;

					::xpcc::Header header;
					uint8_t size = xpcc::XpccOverEthernet::xpccPacketHeaderFromEthernetFrame(message, header);

					receiveItem = new ReceiveItem(size, header);

					std::memcpy(receiveItem->payload.getPointer(), message + 18, size);
				} else {
					// Invalid
					XPCC_LOG_DEBUG.printf("drop message \n");
					return;
				}
			}
		}
	}

protected:
	bool isAvailable;

	class ReceiveItem
	{
	public:
		ReceiveItem(uint8_t size, const Header& inHeader) :
			header(inHeader), payload(size) {}

	public:
		xpcc::Header header;
		xpcc::SmartPointer payload;
	};

	ReceiveItem *receiveItem;

	Driver &driver;
	container_lut_function lut; // Function to determine container id from component id.
}; // EthernetConnector class
} // xpcc namespace

#endif // XPCC__ETHERNET_CONNECTOR_HPP
