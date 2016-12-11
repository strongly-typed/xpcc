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

// [wip]
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_eth.h"
extern ETH_HandleTypeDef  heth;

namespace xpcc
{

// class xpcc_packed EthernetFrame
// {
// public:
// 	EthernetFrame(uint64_t destination = 0, uint64_t source = 0, uint16_t type = 0) :
// 		type(type)
// 		{
// 			setDhost(destination);
// 			setShost(source);
// 		}

// protected:
// 	uint8_t dhost[6];
// 	uint8_t shost[6];
// 	uint16_t type;

// public:
// 	uint8_t payload[46];

// public:
// 	void
// 	setDhost(uint64_t destination) {
// 		dhost[0] =  destination        & 0xff;
// 		dhost[1] = (destination >>  8) & 0xff;
// 		dhost[2] = (destination >> 16) & 0xff;
// 		dhost[3] = (destination >> 24) & 0xff;
// 		dhost[4] = (destination >> 32) & 0xff;
// 		dhost[5] = (destination >> 40) & 0xff;
// 	}

// 	void
// 	setShost(uint64_t source) {
// 		shost[0] =  source        & 0xff;
// 		shost[1] = (source >>  8) & 0xff;
// 		shost[2] = (source >> 16) & 0xff;
// 		shost[3] = (source >> 24) & 0xff;
// 		shost[4] = (source >> 32) & 0xff;
// 		shost[5] = (source >> 40) & 0xff;
// 	}

// 	void
// 	setType(uint16_t t) {
// 		type = t;
// 	}

// 	void
// 	copyToBuffer(uint8_t *buf) {
// 		buf[ 0] = dhost[5];
// 		buf[ 1] = dhost[4];
// 		buf[ 2] = dhost[3];
// 		buf[ 3] = dhost[2];
// 		buf[ 4] = dhost[1];
// 		buf[ 5] = dhost[0];
// 		buf[ 6] = shost[5];
// 		buf[ 7] = shost[4];
// 		buf[ 8] = shost[3];
// 		buf[ 9] = shost[2];
// 		buf[10] = shost[1];
// 		buf[11] = shost[0];
// 		buf[12] = type >> 8;
// 		buf[13] = type & 0xff;
// 		for(uint8_t ii = 0; ii < 46; ++ii) {
// 			buf[14 + ii] = payload[ii];
// 		}

// 	}
// };

/**
 * Ethernet Frame
 * 
 * DST(6) | SRC(6) | Type(2)
 *
 * Dst = 8E-52-43-41-11-xx
 *          R  C  A '17' DST
 * Src = 8E-52-43-41-11-xx
 *          R  C  A '17' SRC 
 *
 * Type = 0x2E = 46 = minimal payload size
 *
 * payload[0] = xpccType
 * payload[1] = isAck
 * Payload[2] = xpccPacketIdentifier
 * Payload[3] = 0 (padding)
 * Payload[4 ..] = xpccPayload[0 .. ]
 *
 * 
 *
 *
 *
 * xpcc::Header
 * 		Type type; {REQUEST, RESPONSE, NEGATIVE_RESPONSE}
 *		bool isAcknowledge;
 *		uint8_t destination;
 *		uint8_t source;
 *		uint8_t packetIdentifier;
 */

} // xpcc namespace

extern "C" {
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_eth.h"
}

namespace xpcc
{
template <typename Driver>
class EthernetConnector : public BackendInterface
{
public:
	EthernetConnector(Driver &/* d */) :
		isAvailable(false)
	{
	}

	virtual
	~EthernetConnector() {
	}

	virtual void
	sendPacket(const Header &header, SmartPointer payload)
	{
		XPCC_LOG_DEBUG.printf("EthernetConnector::sendPacket\n");

		EthernetFrame ethFrame;

		ethFrame.setDhost(0x8E5243410000 | header.destination);
		ethFrame.setShost(0x8E5243410000 | header.source);
		ethFrame.setType(0x8211);

		memset(ethFrame.payload, 0, 46);
		ethFrame.payload[0] = 
		ethFrame.payload[1] = header.isAcknowledge;
		ethFrame.payload[2] = header.packetIdentifier;
		ethFrame.payload[3] = payload.getSize();

		uint8_t *buffer = (uint8_t *)(heth.TxDesc->Buffer1Addr);

		ethFrame.copyToBuffer(buffer);

		HAL_ETH_TransmitFrame(&heth, 64);
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
		if (not this->isAvailable) {
			if(HAL_ETH_GetReceivedFrame(&heth) == HAL_OK)
			{
				__IO ETH_DMADescTypeDef *dmarxdesc = heth.RxFrameInfos.FSRxDesc;
				uint8_t *buffer = (uint8_t *)heth.RxFrameInfos.buffer;
				uint32_t length = heth.RxFrameInfos.length;
				XPCC_LOG_DEBUG.printf("connector RX: %d ***********************\n", length);

				for (uint32_t ii = 0; ii < length; ++ii) {
					XPCC_LOG_DEBUG.printf("%02x ", *buffer);
					++buffer;

					if (ii % 8 == 7) {
						XPCC_LOG_DEBUG.printf(" ");
					}
					if (ii % 16 == 15) {
						XPCC_LOG_DEBUG.printf("\n");
					}
				}
				XPCC_LOG_DEBUG.printf("\n");

				// Copy to xpcc Message
				this->isAvailable = true;

				uint8_t size = buffer[0x11];

				::xpcc::Header header = ::xpcc::Header(
					::xpcc::Header::Type(	/* type = */ buffer[14]),
											/* ack  = */ buffer[15],
											/* dest = */ buffer[5],
											/* src  = */ buffer[11],
											/* id   = */ buffer[16]);

				receiveItem = new ReceiveItem(size, header);

				// The XPCC Header is five bytes long.
				// The XPCC payload begins after the XPCC Header
				// The XPCC payload is five bytes shorter than the zeromq message.
				std::memcpy(receiveItem->payload.getPointer(), buffer + 0x12, 0x11);



				/* Release descriptors to DMA */
				/* Point to first descriptor */
				dmarxdesc = heth.RxFrameInfos.FSRxDesc;
				/* Set Own bit in Rx descriptors: gives the buffers back to DMA */
				for (uint32_t i=0; i< heth.RxFrameInfos.SegCount; i++)
				{  
					dmarxdesc->Status |= ETH_DMARXDESC_OWN;
					dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
				}

				/* Clear Segment_Count */
				heth.RxFrameInfos.SegCount =0;

				/* When Rx Buffer unavailable flag is set: clear it and resume reception */
				if ((heth.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)  
				{
					/* Clear RBUS ETHERNET DMA flag */
					heth.Instance->DMASR = ETH_DMASR_RBUS;
					/* Resume DMA reception */
					heth.Instance->DMARPDR = 0;
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
}; // EthernetConnector class
} // xpcc namespace

#endif // XPCC__ETHERNET_CONNECTOR_HPP
