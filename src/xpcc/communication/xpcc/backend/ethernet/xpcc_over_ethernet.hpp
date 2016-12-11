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

#include <xpcc/debug/logger/logger.hpp>
// set the Loglevel
#undef  XPCC_LOG_LEVEL
#define XPCC_LOG_LEVEL xpcc::log::DEBUG

namespace xpcc {

typedef uint8_t EthernetFrame[64];

class XpccOverEthernet
{
public:
	static void
	ethernetFrameFromXpccPacket(
		/* in  */ const Header &header, const SmartPointer payload,
		/* out */ EthernetFrame &ethFrame)
	{
		// Destination MAC
		ethFrame[0] = macPreamble[0];
		ethFrame[1] = macPreamble[1];
		ethFrame[2] = macPreamble[2];
		ethFrame[3] = macPreamble[3];
		ethFrame[4] = 0x20; // container
		ethFrame[5] = header.destination; // component

		// Source MAC
		ethFrame[6 + 0] = macPreamble[0];
		ethFrame[6 + 1] = macPreamble[1];
		ethFrame[6 + 2] = macPreamble[2];
		ethFrame[6 + 3] = macPreamble[3];
		ethFrame[6 + 4] = 0x10; // container
		ethFrame[6 + 5] = header.source; // component

		// Frame Type
		ethFrame[12 + 0] = 0x82;
		ethFrame[12 + 1] = 0x11;

		// Payload of Ethernet frame
		ethFrame[14 + 0] = static_cast<uint8_t>(header.type);
		ethFrame[14 + 1] = header.isAcknowledge;
		ethFrame[14 + 2] = header.packetIdentifier;
		uint8_t pSize = payload.getSize();
		ethFrame[14 + 3] = pSize;

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
		header.destination = ethFrame[5];
		header.source = ethFrame[6 + 5];
		header.type = static_cast<Header::Type>(ethFrame[14 + 0]);
		header.isAcknowledge = ethFrame[14 + 1];
		header.packetIdentifier = ethFrame[14 + 2];

		uint8_t size = ethFrame[14 + 3];
		return size;
	}

protected:
	static constexpr
	uint8_t macPreamble[4] = {0x8e, 'R', 'C', 'A'};
};

} // xpcc namespace

#endif // XPCC__XPCC_OVER_ETHERNET_HPP
