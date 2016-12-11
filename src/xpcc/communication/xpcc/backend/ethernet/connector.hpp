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

// [wip]
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_eth.h"
extern ETH_HandleTypeDef  heth;

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

		xpcc::EthernetFrame ethFrame;

		xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(
			/* header */ header, 
			/* payload */payload, 
			/* EthernetFrame */ ethFrame);

		uint8_t *buffer = (uint8_t *)(heth.TxDesc->Buffer1Addr);

		memcpy(buffer, ethFrame, 64);

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
				uint8_t const *buffer = (uint8_t *)heth.RxFrameInfos.buffer;
				uint32_t length = heth.RxFrameInfos.length;

				{
					uint8_t const *buf = buffer; // Copy for increment
					XPCC_LOG_DEBUG.printf("EthernetConnector RX: %d ***********************\n", length);

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

				// Copy to xpcc Message
				this->isAvailable = true;

				::xpcc::Header header;
				uint8_t size = xpcc::XpccOverEthernet::xpccPacketHeaderFromEthernetFrame(buffer, header);


				receiveItem = new ReceiveItem(size, header);

				std::memcpy(receiveItem->payload.getPointer(), buffer + 18, size);



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
				heth.RxFrameInfos.SegCount = 0;

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
