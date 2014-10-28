// ----------------------------------------------------------------------------
/*
 * WARNING: This file is generated automatically, do not edit!
 * Please modify the corresponding XML file instead.
 */
// ----------------------------------------------------------------------------

#include <xpcc/communication.hpp>

#include "component_sender/sender.hpp"

#include "packets.hpp"
#include "identifier.hpp"
#include "postman.hpp"

namespace component
{
	extern Sender	sender;
}

// ----------------------------------------------------------------------------
xpcc::Postman::DeliverInfo
Postman::deliverPacket(const xpcc::Header& header, const xpcc::SmartPointer& payload)
{
	xpcc::ResponseHandle response(header);
	
	// Avoid warnings about unused variables
	(void) payload;
	(void) response;
	
	switch (header.destination)
	{
		case robot::component::SENDER:
		{
			switch (header.packetIdentifier)
			{
				
				default:
					return NO_ACTION;
			}
			break;
		}

		
		// Events
		case 0:
			switch (header.packetIdentifier)
			{
				default:
					break;
			}
			return OK;
		
		default:
			return NO_COMPONENT;
	}
	
	return NOT_IMPLEMENTED_YET_ERROR;
}

// ----------------------------------------------------------------------------
bool
Postman::isComponentAvaliable(uint8_t component) const
{
	switch (component)
	{
		case robot::component::SENDER:
			return true;
			break;
		
		default:
			return false;
	}
}
