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

#include <xpcc/debug/logger.hpp>
#undef XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

#include "../backend_interface.hpp"
#include "xpcc_over_ethernet.hpp"

namespace xpcc
{
typedef uint8_t (*lut_function)(const Header&);

template <typename Driver>
class EthernetConnector : public BackendInterface
{
public:
	EthernetConnector(Driver &ethDriver, lut_function lut ) :
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

		uint8_t container = lut(header);
		XPCC_LOG_DEBUG.printf("  component id = %02x, container id = %02x\n", header.destination, container);

		xpcc::EthernetFrame ethFrame;

		xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(
			/* header */ header,
			/* container */ container,
			/* payload */payload, 
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
					(message[0] = 0x8e) and
					(message[1] = 'R') and
					(message[2] = 'C') and
					(message[3] = 'A') and
					(message[6] = 0x8e) and
					(message[7] = 'R') and
					(message[8] = 'C') and
					(message[9] = 'A'))
				{
					// Valid

					// Copy to xpcc Message
					this->isAvailable = true;

					::xpcc::Header header;
					uint8_t size = xpcc::XpccOverEthernet::xpccPacketHeaderFromEthernetFrame(message, header);

					receiveItem = new ReceiveItem(size, header);
					xpcc::delayMilliseconds(100);

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
	lut_function lut; // LUT for determining the container id from component id.
}; // EthernetConnector class
} // xpcc namespace

#endif // XPCC__ETHERNET_CONNECTOR_HPP
