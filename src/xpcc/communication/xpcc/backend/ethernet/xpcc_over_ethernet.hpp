// coding: utf-8
/* Copyright (c) 2016, Roboterclub Aachen e. V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC__XPCC_OVER_ETHERNET_HPP
#define XPCC__XPCC_OVER_ETHERNET_HPP

#include <xpcc/communication/xpcc/backend/header.hpp>

namespace xpcc {

/* Ethernet Frame of minimal length */
class xpcc_packed
EthernetFrame
{
public:
	uint8_t buffer[64];
};

class XpccOverEthernet
{
public:
	static void
	ethernetFrameFromXpccPacket(
		/* in  */ const Header &header, const SmartPointer payload,
		/* out */ EthernetFrame &ethFrame)
	{
		// Destination MAC
		ethFrame.buffer[0] = macPreamble[0];
		ethFrame.buffer[1] = macPreamble[1];
		ethFrame.buffer[2] = macPreamble[2];
		ethFrame.buffer[3] = macPreamble[3];
		ethFrame.buffer[4] = 0; // padding or container
		ethFrame.buffer[5] = header.destination;

		// Source MAC
		ethFrame.buffer[6 + 0] = macPreamble[0];
		ethFrame.buffer[6 + 1] = macPreamble[1];
		ethFrame.buffer[6 + 2] = macPreamble[2];
		ethFrame.buffer[6 + 3] = macPreamble[3];
		ethFrame.buffer[6 + 4] = 0; // padding or container
		ethFrame.buffer[6 + 5] = header.source;

		// Frame Type
		ethFrame.buffer[12 + 0] = 0x82;
		ethFrame.buffer[12 + 1] = 0x11;

		// Payload of Ethernet frame
		ethFrame.buffer[14 + 0] = static_cast<uint8_t>(header.type);
		ethFrame.buffer[14 + 1] = header.isAcknowledge;
		ethFrame.buffer[14 + 2] = header.packetIdentifier;
		uint8_t pSize = payload.getSize();
		ethFrame.buffer[14 + 3] = pSize;

		// Payload of XPCC Message
		memcpy(ethFrame.buffer + 18, payload.getPointer(), pSize);

		// Padding
		memset(ethFrame.buffer + 18 + pSize, 0x00, 64 - 18 - pSize);
	}

	static void
	XpccPacketFromEthernetFram(
		/* in  */ EthernetFrame &ethFrame,
		/* out */ Header &header, SmartPointer payload)
	{
		header.destination = ethFrame.buffer[5];
		header.source = ethFrame.buffer[6 + 5];
		header.type = static_cast<Header::Type>(ethFrame.buffer[14 + 0]);
		header.isAcknowledge = ethFrame.buffer[14 + 1];
		header.packetIdentifier = ethFrame.buffer[14 + 2];

		// payload.setSize(ethFrame.buffer[14 + 3]);
	}

protected:
	static constexpr
	uint8_t macPreamble[4] = {0x8e, 'R', 'C', 'A'};
};

} // xpcc namespace

#endif // XPCC__XPCC_OVER_ETHERNET_HPP
