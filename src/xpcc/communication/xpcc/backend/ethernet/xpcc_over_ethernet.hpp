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

typedef uint8_t EthernetFrame[64];

// Lookup container from component
// typedef robot::container::Identifier (*container_lut_function)(const robot::component::Identfier);
typedef uint8_t (*container_lut_function)(const uint8_t);

class XpccOverEthernet
{
public:
	static void
	ethernetFrameFromXpccPacket(
		/* in  */ const Header &header, const SmartPointer payload, const container_lut_function lut,
		/* out */ EthernetFrame &ethFrame)
	{
		// Destination MAC
		ethFrame[0] = macPreamble[0] | 0x01; // Multicast
		ethFrame[1] = macPreamble[1];
		ethFrame[2] = macPreamble[2];
		ethFrame[3] = lut(header.destination); // Destination container
		ethFrame[4] = header.destination;      // Destination component
		ethFrame[5] = header.packetIdentifier;

		// Source MAC
		ethFrame[6 + 0] = macPreamble[0];
		ethFrame[6 + 1] = macPreamble[1];
		ethFrame[6 + 2] = macPreamble[2];
		ethFrame[6 + 3] = lut(header.source); // Source container
		ethFrame[6 + 4] = header.source;      // Source component
		ethFrame[6 + 5] = header.packetIdentifier;

		// Frame Type
		ethFrame[12 + 0] = 0x82;
		ethFrame[12 + 1] = 0x11;

		// Payload of Ethernet frame
		ethFrame[14 + 0] = static_cast<uint8_t>(header.type);
		ethFrame[14 + 1] = header.isAcknowledge;
		uint8_t pSize = payload.getSize();
		ethFrame[14 + 2] = pSize;

		ethFrame[17] = 0; // zero-padding

		// Payload of XPCC Message
		memcpy(ethFrame + 18, payload.getPointer(), pSize);

		// Padding
		memset(ethFrame + 18 + pSize, 0x00, 64 - 18 - pSize);
	}

	static uint8_t
	xpccPacketHeaderFromEthernetFrame(
		/* in  */ const uint8_t *ethFrame,
		/* out */ Header &header)
	{
		header.destination = ethFrame[4];
		header.source = ethFrame[6 + 4];
		header.type = static_cast<Header::Type>(ethFrame[14 + 0]);
		header.isAcknowledge = ethFrame[14 + 1];
		header.packetIdentifier = ethFrame[5];

		uint8_t size = ethFrame[14 + 2];
		return size;
	}

	/**
	 * Preamble of the MAC address.
	 * 
	 * Luckily, 'R' = 0x82 = 0b10000010, so the 'second-least-significant 
	 * bit of the first octet of the address' [1] is set. This is a
	 * locally administered MAC address.
	 *
	 * The least significant bit of the first octet of an address is set to 0
	 * so this is a unicast. That is true for actions, but not for events.
	 *
	 * [1] https://en.wikipedia.org/wiki/MAC_address#Universal_vs._local
	 */
	static constexpr
	uint8_t macPreamble[3] = {'R', 'C', 'A'};
};

} // xpcc namespace

#endif // XPCC__XPCC_OVER_ETHERNET_HPP
