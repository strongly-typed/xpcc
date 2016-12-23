/* Copyright (c) 2016, Roboterclub Aachen e. V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 * ------------------------------------------------------------------------- */

#include <xpcc/debug/logger.hpp>

// set new log level
#undef XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

#include "communication/identifiers.hpp"
#include "communication/packets.hpp"
#include "communication/postman.hpp"
#include "communication/communication.hpp"

#include "sender.hpp"

// ----------------------------------------------------------------------------
component::Sender::Sender(uint8_t id, xpcc::Dispatcher &communication) :
	xpcc::AbstractComponent(id, communication),
	positionCallback(this, &Sender::getPositionCallback),
	timer(10000),
	location(10, 20, 0.0f)
{
}

// ----------------------------------------------------------------------------
void
component::Sender::getPositionCallback(const xpcc::Header&,
		const robot::packet::Position *parameter)
{
	XPCC_LOG_INFO << XPCC_FILE_INFO
			<< "get position callback: x=" << parameter->x
			<< ", y=" << parameter->y << xpcc::endl;
}

// ----------------------------------------------------------------------------
void
component::Sender::eventRobotLocation(const xpcc::Header& header,
	const robot::packet::Location *payload)
{
	XPCC_LOG_INFO << XPCC_FILE_INFO
		<< "event Robot Location: loc = " << *payload << xpcc::endl;
}

// ----------------------------------------------------------------------------
void
component::Sender::update()
{
	if (timer.execute())
	{
		XPCC_LOG_INFO << XPCC_FILE_INFO << "sender update: " << location << xpcc::endl;		
		
		// robot::EventPublisher::robotLocation(getCommunicator(), location);

		robot::packet::Position position;
		position.x = location.x;
		position.y = location.y;
		
		this->callAction(
				robot::component::RECEIVER,
				robot::action::SET_POSITION,
				position);

		// this->callAction(
		// 		robot::component::RECEIVER,
		// 		robot::action::GET_POSITION,
		// 		positionCallback);

		location.x += 1;
		location.y += 2;
		location.phi += 0.03;
	}
}
